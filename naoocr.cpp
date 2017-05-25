#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <math.h>
#include <ctime>
#include <locale>

#include <pthread.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <alerror/alerror.h>
#include <alcommon/alproxy.h>
#include <alproxies/almotionproxy.h>
#include <alproxies/alrobotpostureproxy.h>
#include <alproxies/alvideodeviceproxy.h>
#include <alvision/alimage.h>
#include <alproxies/altexttospeechproxy.h>


#include "boost/filesystem.hpp"
#include "APIwrappers.h"


/*
Preparing for two threads for program execution:
    - first thread has function that sends recognized text to speach module
    - second thread has function that gets recognized text from OCR module

First thread waits for approval from second send text to speach

*/


void * threadTextAcquisition(void * arg);
void * threadTextSpeak(void * arg);
bool threadingDone = false;

pthread_mutex_t mutexSpeak = PTHREAD_MUTEX_INITIALIZER;

/*Prototype function for NAO relocation*/

void relocateRobot();


/*Program declaration*/
boost::shared_ptr<AL::ALProxy> TextRecognition;
std::vector<std::string> result;

std::string picture, database, resultLoc, input, output, mode, naoIP, lang, debug, debugLocation, ocrMode, naoMove;
int naoPort;
bool verbose = false;

void help(){
    std::cout<<"Supported commands:"<<std::endl;
    std::cout<<"\t--help, -h\t\t   Print help."<<std::endl;
    std::cout<<"\t--verbose -v\t\t   Print detailed information."<<std::endl;
    std::cout<<"\t--address, -a [argument]   Set NAO IP address."<<std::endl;
    std::cout<<"\t--port, -p [argument]\t   Set port."<<std::endl;
    std::cout<<"\t--movement, -m [argument]\t   Enable NAO movement. TRUR/FALSE"<<std::endl;
    std::cout<<"\t--picture, -pct [argument] Set picture location."<<std::endl;
    std::cout<<"\t--input, -i [argument]\t   Set input mode. FILE/CAMERA"<<std::endl;
    std::cout<<"\t--output, -o [argument]\t   Set output mode. SPEAK/SILENT"<<std::endl;
    std::cout<<"\t--ocr_mode, -om [argument]\t   Set ocr mode. WHOLE/SEGMENTED"<<std::endl;
    std::cout<<"\t--lang, -l [argument]\t   Set language. eng/hrv"<<std::endl;
    std::cout<<"\t--debug, -d [argument]\t   Enable debug. TRUE/FALSE"<<std::endl;
    std::cout<<"\t--debug_location, -dl [argument]\t   Set debug output folder location."<<std::endl;
}

void checkNaoConfig(){
    std::ifstream config("nao.config");
    if (config.good())
    {
        std::string line, command;
        int position;
        while(getline(config,line)){
            position = line.find('=');
            command = line.substr(0,position);
            line = line.substr(position + 1);

            if( command == "PICTURE_LOCATION") picture = line;
            if( command == "RESULT_LOCATION") resultLoc = line;
            if( command == "INPUT_MODE") input = line;
            if( command == "OUTPUT_MODE") output = line;
            if( command == "OCR_MODE") ocrMode = line;
            if( command == "LANGUAGE") lang = line;
            if( command == "DEBUG") debug = line;
            if( command == "DEBUG_LOCATION") debugLocation = line;
            if( command == "NAO_MOVEMENT") naoMove = line;
            if( command == "NAO_IP") naoIP = line;
            if( command == "NAO_PORT"){
                const char* c = line.c_str();
                naoPort = strtol(c,NULL,10);
            }
        }
    }
    else{
        std::cerr << "[ERROR]: The nao.config file doesn't exist! For more information read the README file." << std::endl;
        exit(2);
    }
    config.close();
}

void checkParameters(std::string name){
    if(naoMove.length() == 0 || picture.length() == 0 || resultLoc.length() == 0 || input.length() == 0 || output.length() == 0 || ocrMode.length() == 0 || lang.length() == 0 || naoIP.length() == 0 || debug.length() == 0 ){
        std::cerr << "[ERROR]: One or more of the default parameters are not defined. Please check " << name << std::endl;
        exit(2);
    }

    if(debug == "TRUE" & debugLocation.size() == 0){
        std::cerr << "[ERROR]: DEBUG is enabled but there is no DEBUG_LOCATION specified. Please check " << name << std::endl;
        exit(2);
    }


    bool flag_mode = false;
    if(input != "FILE" && input != "CAMERA"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << input << " INPUT_MODE doesn't exist. The supported modes are 'FILE' and 'CAMERA'. Please check " << name << std::endl;
    }
    if(output != "SPEAK" && output != "SILENT"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << output << " OUTPUT_MODE doesn't exist. The supported modes are 'SPEAK' and 'SILENT'. Please check " << name << std::endl;
    }

    if(ocrMode != "WHOLE" && ocrMode != "SEGMENTED"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << ocrMode << " OCR_MODE doesn't exist. The supported modes are 'WHOLE' and 'SEGMENTED'. Please check " << name << std::endl;
    }

    if(naoPort == 0L){
        flag_mode = true;
        std::cerr << "[ERROR]: Please enter a valid number for the NAO_PORT parameter in " << name << std::endl;
    }
    if(lang != "eng" && lang != "hrv"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << lang << " LANGUAGE doesn't exist. The supported languages are 'eng' and 'hrv'. Please check " << name << std::endl;
    }


    if(debug != "TRUE" && debug != "FALSE"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << debug << " is not a valid DEBUG option. The supported options are 'TRUE' and 'FALSE'. Please check " << name << std::endl;
    }

    if(naoMove != "TRUE" && naoMove != "FALSE"){
        flag_mode = true;
        std::cerr << "[ERROR]: The " << naoMove << " is not a valid NAO_MOVEMENT option. The supported options are 'TRUE' and 'FALSE'. Please check " << name << std::endl;
    }

    std::ifstream pic(picture);
    if (!pic.good()){
        std::cerr << "[ERROR]: The picture file doesn't exist! Please check " << name << std::endl;
        flag_mode = true;
    }
    pic.close();

    std::ifstream res;
    res.open(resultLoc,std::ios::app);
    res.close();
    res.open(resultLoc);
    if (!res.good()){
        std::cerr << "[ERROR]: The result location doesn't exist! Please check " << name << std::endl;
        flag_mode = true;
    }
    res.close();

    if (debugLocation[debugLocation.size()-1] != '/') debugLocation.push_back('/');
    if ( !(boost::filesystem::exists(debugLocation))){
         std::cerr << "[ERROR]: The debug folder location doesn't exist! Please check " << name << std::endl;
         flag_mode = true;
    }

    if (flag_mode == true) exit(2);
}

void checkArguments(int argc, char* argv[]){
    std::string argument;
    bool ver = false;
    for(int i = 1; i < argc; i+=2){
        if (ver == true){
            ver = false;
            i--;
        }
        argument = argv[i];
        if(argument == "--help" || argument == "-h"){
            help();
            exit(3);
        }
        if (argument == "--verbose" || argument == "-v"){
                verbose = true;
                ver = true;
        }
        else if ((argument == "--addres" || argument == "-a") && i+1 < argc)
            naoIP = argv[i+1];
        else if ((argument == "--port" || argument == "-p")&& i+1 < argc)
            naoPort = strtol(argv[i+1],NULL,10);
        else if ((argument == "--movement" || argument == "-m")&& i+1 < argc)
            naoMove = strtol(argv[i+1],NULL,10);
        else if ((argument == "--picture" || argument == "-pct")&& i+1 < argc)
            picture = argv[i+1];
        else if ((argument == "--result" || argument == "-r")&& i+1 < argc)
            resultLoc = argv[i+1];
        else if ((argument == "--input" || argument == "-i")&& i+1 < argc)
            input = argv[i+1];
        else if ((argument == "--output" || argument == "-o")&& i+1 < argc)
            output = argv[i+1];
        else if ((argument == "--ocr_mode" || argument == "-om")&& i+1 < argc)
            ocrMode = argv[i+1];
        else if ((argument == "--lang" || argument == "-l")&& i+1 < argc)
            lang = argv[i+1];
        else if ((argument == "--debug" || argument == "-d")&& i+1 < argc)
            debug = argv[i+1];
        else if ((argument == "--debug_location" || argument == "-dl")&& i+1 < argc)
            debugLocation = argv[i+1];
        else{
            std::cerr << "[ERROR]: Invalide argument "<< argument<< " !" << std::endl;
            exit(3);
        }
    }
}


void printParameters(){
    if (input == "CAMERA")  picture = "none";
    if (debug == "FALSE")  debugLocation = "none";
    if (input == "CAMERA")  picture = "none";

    if (verbose == true){
    std::cout<<"---------------------------------Properties-------------------------------------"<<std::endl;
    std::cout<<"NAO_IP = "<<naoIP<<std::endl;
    std::cout<<"NAO_PORT = "<<naoPort<<std::endl;
    std::cout<<"NAO_MOVEMENT = "<<naoMove<<std::endl;
    std::cout<<"INPUT_MODE = "<<input<<std::endl;
    std::cout<<"OUTPUT_MODE = "<<output<<std::endl;
    std::cout<<"OCR_MODE = "<<ocrMode<<std::endl;
    std::cout<<"LANGUAGE = "<<lang<<std::endl;
    std::cout<<"RESULT_LOCATON = "<<result<<std::endl;
    std::cout<<"PICTURE_LOCATION = "<<picture<<std::endl;
    std::cout<<"DEBUG = "<< debug << std::endl;
    std::cout<<"DEBUG_LOCATION = " << debugLocation << std::endl;
    std::cout<<"--------------------------------------------------------------------------------"<<std::endl;
    }
}


int main(int argc, char* argv[]) {

    checkNaoConfig();
    checkParameters("your nao.config file!");
    checkArguments(argc, argv);
    checkParameters("the argument list!");
    printParameters();
;

    TextRecognition = boost::shared_ptr<AL::ALProxy>(new AL::ALProxy("OcrModule", naoIP, naoPort));



    try {
        if(debug == "TRUE"){
            TextRecognition->callVoid("setPoolVariable", "debug", BOOLtoAL (true));
            TextRecognition->callVoid("setPoolVariable", "debugLocation", STRINGtoAL (debugLocation));
        }
        if (naoMove == "TRUE")relocateRobot();
        cv::Mat imgTemp;
        if (input == "FILE") imgTemp = cv::imread(picture);
        if (input == "CAMERA"){
                AL::ALVideoDeviceProxy camProxy(naoIP, naoPort);
                const std::string clientName = camProxy.subscribe("OcrModule", AL::k4VGA, AL::kBGRColorSpace, 5);
                AL::ALValue pic = camProxy.getImageRemote(clientName);
                imgTemp = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC3);
                imgTemp.data = (uchar*) pic[6].GetBinary();
                camProxy.releaseImage(clientName);
                camProxy.unsubscribe(clientName);
            }

        TextRecognition->callVoid("setPoolVariable","picOriginal", MATtoAL(imgTemp));

    }
    catch (const AL::ALError& e) {
      std::cerr << e.what() << std::endl;
    }

    TextRecognition->callVoid ("runSceneDetection");
    TextRecognition->callVoid ("runPictureProcessing");

    pthread_t thread1, thread2;

    pthread_create( &thread1, NULL, &threadTextSpeak, NULL);
    pthread_create( &thread2, NULL, &threadTextAcquisition, NULL);

    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);

    return 0;
}

//
//MULTITHREDING FUNCTIONS
//

void *threadTextSpeak(void * arg){
    std::ofstream resultFile;
    resultFile.open(resultLoc);

    int nextToSay= 0;
    bool textExist = false;
    std::string textToSay;
    std::cout<<"------------------------------------Result--------------------------------------"<<std::endl;
    while(!threadingDone || nextToSay < result.size() || textExist){
        if (textExist){
            std::cout << textToSay << std::endl;
            resultFile << textToSay << std::endl;
            if (output == "SPEAK"){
                AL::ALTextToSpeechProxy textToSpeach(naoIP, naoPort);
                textToSpeach.say(textToSay);
                }
            textExist = false;
            }
        if (nextToSay < result.size()){
            pthread_mutex_lock( &mutexSpeak );
            textToSay = result[nextToSay++];
            textExist = true;
            pthread_mutex_unlock( &mutexSpeak );
            }

    }
    std::cout<<"--------------------------------------------------------------------------------"<<std::endl;
    resultFile.close();
}

void *threadTextAcquisition(void * arg){
    try{
        TextRecognition->callVoid("setPoolVariable","ocrLang", STRINGtoAL(lang));
        if (ocrMode == "WHOLE"){
            result.push_back(TextRecognition->call<std::string>("runTextRecognition"));
        }
        else{
            int SegmentNr = TextRecognition->call<int>("runPictureSegmentation");
            std::string sentence = "";

            for (int k=0; k < SegmentNr ; k++){

                std::string tempText = TextRecognition->call<std::string>("runSegmentRecognition", k );
                int lastBreak=0;
                //std::cout<<"[DEBUG]"<<tempText <<std::endl;
                for (int i=0; i < tempText.size(); i++){
                    if (tempText[i]== '\n') tempText[i]= ' ';
                    if (tempText[i]== '.' || tempText[i]== '!' || tempText[i]== '?'){
                        sentence += tempText.substr(lastBreak, i+1 - lastBreak);
                        //std::cout<<"[DEBUG][SUB_SEG]"<<sentence<<std::endl;
                        lastBreak = i+1;
                        pthread_mutex_lock( &mutexSpeak );
                        result.push_back(sentence.substr((sentence[0] == ' ' ? 1 : 0 ),(sentence[1] == ' ' ? sentence.size()-1 : sentence.size())));
                        pthread_mutex_unlock( &mutexSpeak );
                        sentence = "";
                    }
                }
                sentence += tempText.substr(lastBreak, tempText.size() - lastBreak) + " ";
            }
        if (sentence.size() > 1) result.push_back(sentence.substr((sentence[0] == ' ' ? 1 : 0 ),(sentence[1] == ' ' ? sentence.size()-2 : sentence.size()-1)));
        }
    }
    catch (const AL::ALError& e) {
        std::cerr << e.what() << std::endl;
    }
    threadingDone=true;
}


/* Moves robot to a position where text frame is almost rectangular*/
void relocateRobot(){
        cv::Mat imgPos1, imgPos2;
        std::vector<cv::Point> points1, points2;

        double moveDist= 0.5; //meters
        double degViewAngle = 60.97;
        double degSideAngle = 59.515;
        double radViewAngle = 3.1415926 * degViewAngle / 180;


        try {
            AL::ALRobotPostureProxy pozer(naoIP, naoPort);
            pozer.goToPosture("StandInit", 0.3);


            AL::ALVideoDeviceProxy camProxy(naoIP, naoPort);
            const std::string clientName = camProxy.subscribe("OcrModule", AL::k4VGA, AL::kBGRColorSpace, 5);
            AL::ALValue pic = camProxy.getImageRemote(clientName);
            imgPos1 = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC3);
            imgPos1.data = (uchar*) pic[6].GetBinary();

            camProxy.releaseImage(clientName);

            TextRecognition->callVoid("setPoolVariable", "picOriginal",MATtoAL(imgPos1));
            TextRecognition->callVoid ("runSceneDetection");
            points1 = ALtoSEG(TextRecognition->call<AL::ALValue>("getPoolVariable", "segFrame"));

            AL::ALMotionProxy motion(naoIP, naoPort);
            motion.moveTo(0, -moveDist, 0);

            pic = camProxy.getImageRemote(clientName);
            imgPos2 = cv::Mat(cv::Size(pic[0], pic[1]), CV_8UC3);
            imgPos2.data = (uchar*) pic[6].GetBinary();

            camProxy.releaseImage(clientName);
            camProxy.unsubscribe(clientName);

            TextRecognition->callVoid("setPoolVariable", "picOriginal", MATtoAL(imgPos2));
            TextRecognition->callVoid ("runSceneDetection");
            points2 = ALtoSEG(TextRecognition->call<AL::ALValue>("getPoolVariable", "segFrame"));
        }
        catch (const AL::ALError& e) {
            std::cerr << e.what() << std::endl;
        }

        cv::Point2d frame[4]; //coordinates of frame points in space (topdown xy projection)
        for (int k=0; k<4 ;k++){
                double degLeftAngle = degSideAngle + degViewAngle*(1280-points1[k].x)/1280;
                double degRightAngle = degSideAngle + degViewAngle*points2[k].x/1280;
                double degFarAngle = 180 - degLeftAngle - degRightAngle;

                double radFarAngle = 3.141926 * degFarAngle / 180;
                double radLeftAngle = 3.141926 * degLeftAngle / 180;
                double radRightAngle = 3.141926 * degRightAngle / 180;

                double dist = moveDist*sin(radLeftAngle)/sin(radFarAngle);
                frame[k].x = sin(radRightAngle)*dist;
                frame[k].y = sqrt(dist*dist - frame[k].x*frame[k].x)* (degRightAngle < 90 ? 1 : -1);
            }
        double x1,x2,y1,y2,xc,yc,xr,yr,m, theta, dist, l;
        x1 = (frame[0].x + frame[3].x)/2; //left border x
        y1 = (frame[0].y + frame[3].y)/2; //left border y
        x2 = (frame[1].x + frame[1].x)/2; //right border x
        y2 = (frame[2].y + frame[2].y)/2; //right border y
        xc = (x1 + x2)/2; //center x
        yc = (y1 + y2)/2; //center y
        m = (y2 -y1)/(x2-x1); //frame slope
        l = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)); //frame width
        dist = l/(2*tan(0.66*radViewAngle/2)); //wanted distance to frame (so frame takes 0.66 of picture width)

        theta = atan(1/m); // angle to turn
        xr = xc - sqrt(m*m*dist*dist/(m*m+1)); //new robot x coordinate
        yr = yc + (theta > 0 ? 1 : -1) * sqrt(dist*dist/(m*m+1)); //new robot y coordinate


        imwrite(debugLocation + "pozicija1.png",imgPos1);
        imwrite(debugLocation + "pozicija2.png",imgPos2);

        std::cout<< "Better robot position:"<<std::endl;
        std::cout<< "Forward: "<< xr <<" [m]"<<std::endl;
        std::cout<< (yr >= 0 ? "Left: " : "Right: ") << (yr >= 0 ? 1 : -1)*yr <<" [m]"<<std::endl;
        std::cout<< (theta <= 0 ? "CCW: " : "CW: ") << (theta <= 0 ? -1 : 1 )*theta*180/3.1415926  << " [deg]"<<std::endl;


    }

