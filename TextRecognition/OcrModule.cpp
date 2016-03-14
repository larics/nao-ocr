#include <iostream>
#include <fstream>
#include <ctime>
#include <locale>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include <alcommon/almodule.h>
#include <alvalue/alvalue.h>

#include "OcrModule.hpp"
#include "picProcessing.hpp"
#include "picSceneDetect.hpp"
#include "picSegmentation.hpp"
#include "APIwrappers.h"
#include <locale>

/*NAO*/
OcrModule::OcrModule(boost::shared_ptr<AL::ALBroker> broker, const std::string &name) : AL::ALModule(broker, name){
    // Describe the module here. This will appear on the webpage
      setModuleDescription("Module for text recognition on pictures.");

      functionName("setPoolVariable", getName(), "Set a new value for pool variable.");
      addParam("variableName","Name of variable to which is new value appended");
      addParam("variableValue","New value for variable");
      BIND_METHOD(OcrModule::setPoolVariable);

      functionName("getPoolVariable", getName(), "Get pool variable  value.");
      addParam("variableName","Name of variable to read value");
      setReturn("AL::ALValue","Variable value");
      BIND_METHOD(OcrModule::getPoolVariable);

      functionName("runSceneDetection", getName(), "Detection of text bounding frame");
      BIND_METHOD(OcrModule::runSceneDetection);

      functionName("runPictureProcessing", getName(), "Picture processing for text recognition.");
      BIND_METHOD(OcrModule::runPictureProcessing);

      functionName("runPictureSegmentation", getName(), "Picture segmentation for line by line recognition.");
      setReturn ("int", "Returns number of segments");
      BIND_METHOD(OcrModule::runPictureSegmentation);

      functionName("runTextRecognition", getName(), "Text recognition of whole text body.");
      setReturn("string", "Recognized text.");
      BIND_METHOD(OcrModule::runTextRecognition);

      functionName("runSegmentRecognition", getName(), "Get recognized text segment.");
      addParam ("SegmentIndex", "Segment index to be recognized");
      setReturn("string", "Recognized segment of text.");
      BIND_METHOD(OcrModule::runSegmentRecognition);
}


/*Module functions*/
OcrModule::~OcrModule(){
    reset();
}

void OcrModule::init(){
    reset();
}

void OcrModule::reset(){
    ocrLang.clear();
    debugLocation.clear();
    picOriginal.release();
    picProcesed.release();
    picProcesedBin.release();
    debug = false;
}

/*SET ang GET functions*/
void OcrModule::setPoolVariable(const std::string & variableName, const AL::ALValue & variableValue){
        if (variableName == "picOriginal") picOriginal = ALtoMAT(variableValue);
        if (variableName == "picProcesed") picProcesed = ALtoMAT(variableValue);
        if (variableName == "picProcesedBin") picProcesedBin = ALtoMAT(variableValue);
        if (variableName == "segLines") segLines = ALtoSEGn(variableValue);
        if (variableName == "segFrame") segFrame = ALtoSEG(variableValue);
        if (variableName == "ocrLang") ocrLang = ALtoSTRING(variableValue);
        if (variableName == "debugLocation") debugLocation = ALtoSTRING(variableValue);
        if (variableName == "debug") debug = ALtoBOOL(variableValue);
        if (variableName == "numAvgSegHeight") numAvgSegHeight = ALtoINT(variableValue);
    }

AL::ALValue OcrModule::getPoolVariable(const std::string & variableName){

        if (variableName == "picOriginal")  return MATtoAL(picOriginal);
        if (variableName == "picProcesed")  return MATtoAL(picProcesed);
        if (variableName == "picProcesedBin")  return MATtoAL(picProcesedBin);
        if (variableName == "segLines")  return SEGntoAL(segLines);
        if (variableName == "segFrame")  return SEGtoAL(segFrame);
        if (variableName == "ocrLang")  return STRINGtoAL(ocrLang);
        if (variableName == "debugLocation") return STRINGtoAL(debugLocation);
        if (variableName == "debug") return BOOLtoAL(debug);
        if (variableName == "numAvgSegHeight") return INTtoAL(numAvgSegHeight);

        return 0;
    }


/*RUN functions*/
void OcrModule::runSceneDetection(){

        picSceneDetection frameDetect;

        frameDetect.setPoolVariable("debug", debug);
        frameDetect.setPoolVariable("debugLocation", debugLocation);

        frameDetect.setPoolVariable("picture", picOriginal);
        frameDetect.runSceneDetection();
        frameDetect.getPoolVariable("segFrame", segFrame);

    }

void OcrModule::runPictureProcessing(){

    picProcessing picProc;

    picProc.setPoolVariable("debug", debug);
    picProc.setPoolVariable("debugLocation", debugLocation);

    picProc.setPoolVariable("picOriginal", picOriginal);
    picProc.setPoolVariable("segFrame", segFrame);

    picProc.runPicProcessing();

    picProc.getPoolVariable("picCutOriginal", picProcesed);
    picProc.getPoolVariable("picCutBinary", picProcesedBin);

}

int OcrModule::runPictureSegmentation(){

    picSegmentation picSeg;

    picSeg.setPoolVariable("debug", debug);
    picSeg.setPoolVariable("debugLocation", debugLocation);

    picSeg.setPoolVariable("picOriginal", picProcesedBin);
    picSeg.runPicSegmentation();
    picSeg.getPoolVariable("segTextLines", segLines);
    picSeg.getPoolVariable("numAvgSegHeight", numAvgSegHeight);

    return segLines.size();
}

std::string OcrModule::runTextRecognition(){

    std::string text;

    tess = new tesseract::TessBaseAPI();
    tess->Init(NULL, ocrLang.c_str());
    std::cout<<"[Tesseract] Ready!"<<std::endl;
    tess->SetPageSegMode (tesseract::PSM_AUTO);
    tess->SetImage((uchar*)picProcesed.data, picProcesed.size().width, picProcesed.size().height, picProcesed.channels(), picProcesed.step1());
    std::cout<<"[Tesseract] Image loaded!"<<std::endl;

    tess->Recognize(0);
    text = tess->GetUTF8Text();
    std::cout<<"[Tesseract] OCR has finished!"<<std::endl;

    tess->End();

    return text;
}

std::string OcrModule::runSegmentRecognition(const int &segmentIndex){

    if (segmentIndex >= segLines.size()) return "ERROR Index out of range.\n";

    tess = new tesseract::TessBaseAPI();
    tess->Init(NULL, ocrLang.c_str());
    tess->SetImage((uchar*)picProcesed.data, picProcesed.size().width, picProcesed.size().height, picProcesed.channels(), picProcesed.step1());

    std::string text;

    int ULx=segLines[segmentIndex][0].x;
    int ULy=segLines[segmentIndex][0].y;
    int W=segLines[segmentIndex][1].x-segLines[segmentIndex][0].x;
    int H=segLines[segmentIndex][3].y-segLines[segmentIndex][0].y;

    tess->SetRectangle(ULx, ULy, W, H);
    std::cout<<"[Tesseract] Segment "<< segmentIndex + 1 <<" Loaded!"<<std::endl;

    if (H < numAvgSegHeight * 1.3)
        tess->SetPageSegMode (tesseract::PSM_SINGLE_LINE);
    else
        tess->SetPageSegMode (tesseract::PSM_AUTO);

    tess->Recognize(0);
    text = tess->GetUTF8Text();
    text = text.substr(0,text.size()-2);
    std::cout<<"[Tesseract] Segment "<< segmentIndex + 1 <<" OCR finished!"<<std::endl;

    tess->End();

    return text;
}
