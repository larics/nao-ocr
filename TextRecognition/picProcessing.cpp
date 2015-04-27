#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "picProcessing.hpp"

using namespace cv;

/*Distance between two points*/
double picProcessing::getDistance(cv::Point2f pnt1, cv::Point2f pnt2)
{
    return hypot(pnt2.x - pnt1.x, pnt2.y - pnt1.y);
}

void picProcessing::runPicProcessing(){

    /*Cuting and dewarping selected frame out of the picture*/

    Point2f dstPoints[4], srcPoints[4];
    double minDist=getDistance(segFrame[0],Point2f(0,0));
    int minIndex=1, orientationArea=segFrame[3].x*segFrame[0].y;
    for(int k=1 ; k<4 ; k++){
        orientationArea+= segFrame[k-1].x*segFrame[k].y;
        int temp=getDistance(segFrame[k],Point2f(0,0));
        if (minDist > temp){
            minDist=temp;
            minIndex=k;
        }
    }

    srcPoints[0]=segFrame[minIndex];
    srcPoints[1]=(orientationArea < 0)?segFrame[(minIndex+1)%4]:segFrame[(minIndex+3)%4];
    srcPoints[2]=(orientationArea > 0)?segFrame[(minIndex+1)%4]:segFrame[(minIndex+3)%4];
    srcPoints[3]=segFrame[(minIndex+2)%4];

    double dimX1,dimX2, dimY1, dimY2, dimX, dimY;
    dimX1=getDistance(srcPoints[0],srcPoints[1]);
    dimX2=getDistance(srcPoints[2],srcPoints[3]);
    dimY1=getDistance(srcPoints[0],srcPoints[2]);
    dimY2=getDistance(srcPoints[1],srcPoints[3]);
    dimX = (dimX1 > dimX2) ? dimX1 : dimX2;
    dimY = (dimY1 > dimY2) ? dimY1 : dimY2;

    if (dimY > dimX){
        double tmp=dimY;
        dimY = dimX;
        dimX=tmp;
        Point2f temp=srcPoints[1];
        srcPoints[1]=srcPoints[3];
        srcPoints[3]=srcPoints[2];
        srcPoints[2]=srcPoints[0];
        srcPoints[0]=temp;
    }

    int marginTreshold= 5;

    dstPoints[0] = Point2f(-marginTreshold,-marginTreshold);
    dstPoints[1] = Point2f(dimX+marginTreshold,-marginTreshold);
    dstPoints[2] = Point2f(-marginTreshold,dimY+marginTreshold);
    dstPoints[3] = Point2f(dimX+marginTreshold, dimY+marginTreshold);


    Mat warpMatrix=getPerspectiveTransform(srcPoints,dstPoints);

    warpPerspective(picOriginal, picCutOriginal, warpMatrix, Size(dimX, dimY), INTER_CUBIC, BORDER_CONSTANT);

    /*Cutted picture to Grayscale to binary image*/

    Mat picCutGray;

    if (picCutOriginal.channels() == 3)
        cvtColor(picCutOriginal,picCutGray,CV_RGB2GRAY);
    if (picCutOriginal.channels() == 1)
        picCutGray = picCutOriginal;


    int blockSize = picCutGray.rows*picCutGray.cols*0.0003;
    if (blockSize % 2 == 0)
        blockSize--;
    if (blockSize < 3)
        blockSize = 3;
    adaptiveThreshold( picCutGray, picCutBinary, 255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY, blockSize+2, 10);

    /*weighted blending of processed cut of original picture and binary mask*/
    double maskWeight = 0.3;
    Mat blendMask;
    Mat inverseBin;
    std::vector<Mat> tempMask;
    bitwise_not ( picCutBinary, inverseBin );
    for(int k=0; k<3 ; k++)tempMask.push_back(inverseBin);
    merge(tempMask, blendMask);

    picCutOriginal -= blendMask * maskWeight ;

    /*debug*/ if (debug){
    /*debug*/   imwrite(debugLocation + "picProcessing/ProcessedBIN.png", picCutBinary);
    /*debug*/   imwrite(debugLocation + "picProcessing/ProcessedORG.png", picCutOriginal);
    /*debug*/ }
    }

void picProcessing::setPoolVariable(std::string variableName, cv::Mat variableValue){
        if (variableName == "picOriginal") picOriginal =  variableValue;
        if (variableName == "picCutOriginal") picCutOriginal =  variableValue;
        if (variableName == "picCutBinary") picCutBinary =  variableValue;
    }
void picProcessing::setPoolVariable(std::string variableName, std::vector<cv::Point> variableValue){
        if (variableName == "segFrame") segFrame = variableValue;
    }
void picProcessing::setPoolVariable(std::string variableName, bool variableValue){
        if (variableName == "debug") debug = variableValue;
    }
void picProcessing::setPoolVariable(std::string variableName, std::string variableValue){
        if (variableName == "debugLocation") debugLocation = variableValue;
    }

void picProcessing::getPoolVariable(std::string variableName, cv::Mat &variableValue){
        if (variableName == "picOriginal") variableValue = picOriginal;
        if (variableName == "picCutOriginal") variableValue = picCutOriginal;
        if (variableName == "picCutBinary") variableValue = picCutBinary;
    }
void picProcessing::getPoolVariable(std::string variableName, std::vector<cv::Point> &variableValue){
        if (variableName == "segFrame") variableValue = segFrame;
    }
void picProcessing::getPoolVariable(std::string variableName, bool &variableValue){
        if (variableName == "debug") variableValue = debug;
    }
void picProcessing::getPoolVariable(std::string variableName, std::string &variableValue){
        if (variableName == "debugLocation") variableValue = debugLocation;
    }



