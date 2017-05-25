#include <iostream>
#include <list>
#include <algorithm>
#include <numeric>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "picSegmentation.hpp"

using namespace cv;

/*Draw rectangles*/
cv::Mat picSegmentation::drawRectangles(){
    Mat image = picOriginal.clone();

    drawContours(image, segTextLines, -1, 0, 3);

    return image;
}

/*Segmentise picture and rows by sentences*/

void picSegmentation::runPicSegmentation(){

    /*Dilate objects*/
    erode(picOriginal, picEroded, Mat(), Point(-1, -1), 3, 1, 1);

    /*Get histogram (add columns)*/
    reduce(picEroded, matHistogram, 1, CV_REDUCE_SUM, CV_32S);
    matHistogram /= 255;
    matHistogram.col(0).copyTo(vecHistogram);
    int globMin = *min_element(vecHistogram.begin(),vecHistogram.end()) * numSmudgeSize/(numSmudgeSize+1);
    int globMax = *max_element(vecHistogram.begin(),vecHistogram.end());

    std::list <int> minima;
    std::list <int> maxima;

    int startPoint=0, endPoint=vecHistogram.size()-1;
    while(vecHistogram[startPoint++] == globMax);
    while(vecHistogram[endPoint--] == globMax);
    minima.push_back(startPoint - 1);

    /*All local minimums on interval of minTextHeight*/
    int tempMin=startPoint;
    int tempMaxL=startPoint-1;
    int tempMaxR=startPoint;

    for (int k=startPoint; k < endPoint; k++){
            tempMaxR = (vecHistogram[tempMaxR] < vecHistogram[k]) ? k : tempMaxR ;
            if(vecHistogram[tempMin] > vecHistogram[k]){
                    tempMaxL = (vecHistogram[tempMaxR] > vecHistogram[tempMaxL]) ? tempMaxR : tempMaxL ;
                    tempMaxR = k;
                    tempMin = k;
                }

            if (!(k % numMinSegHeight)){
                    minima.push_back(tempMin);
                    maxima.push_back((vecHistogram[tempMin] > vecHistogram[tempMaxL]) ? tempMin : tempMaxL);
                    tempMin = ++k;
                    tempMaxL = tempMaxR;
                    tempMaxR =  k;
                }
        }
    minima.push_back(tempMin);
    maxima.push_back((vecHistogram[tempMin] > vecHistogram[tempMaxL]) ? tempMin : tempMaxL);
    minima.push_back(endPoint + 1);
    maxima.push_back(endPoint + 1);

    std::list<int>::iterator itlMin1, itlMin2, itlMax, itlMin;
    itlMax = maxima.begin();
    itlMin2 = minima.begin();
    itlMin1 = itlMin2++;

    for (; itlMin2 != minima.end() ;){
            std::vector<int>::iterator itvMin1, itvMin2, itvMax;
            itvMax = vecHistogram.begin() + *itlMax;
            itvMin1 = vecHistogram.begin() + *itlMin1;
            itvMin2 = vecHistogram.begin() + *itlMin2;

            double numH = *itvMin1 > *itvMin2 ? *itvMin1 : *itvMin2;
            double numL = *itvMin1 < *itvMin2 ? *itvMin1 : *itvMin2;
            double numU = (*itvMin1 + *itvMin2)/2;
            double numD = *itvMax;
            double numQ = (abs(*itvMin1 -*itvMin2)/(*itvMax-numL))*(*itvMax-numH);

            if (!(*itvMax-numH)) numQ = -numD + numU;

            double peakness = 1 - numU/(numD +numQ);
            double limit = 0.35 + 0.10*picOriginal.cols/2000;

            /*debug*/   if(debug){
            /*debug*/       int q1=*itvMin1;
            /*debug*/       int q2=*itvMin2;
            /*debug*/       int w1=*itlMin1;
            /*debug*/       int w2=*itlMin2;
            /*debug*/       std::cout << std::endl << "[minL " << w1 << "] "<< q1 << std::endl;
            /*debug*/       std::cout << "[max_ "<< *itlMax << "] "<< *itvMax<< std::endl;
            /*debug*/       std::cout << "[minR " << w2 << "] "<< q2 << std::endl;
            /*debug*/       std::cout << "    p: " << peakness << std::endl;
            /*debug*/       std::cout << "limit: " << limit << std::endl;
            /*debug*/       }

            if (peakness < limit){
                    if(*itvMin1 > *itvMin2){
                            minima.erase(itlMin1);
                            itlMin1 = itlMin2++;

                            std::list<int>::iterator itlTemp = itlMax;
                            if(itlMax == maxima.begin()){
                                    ++itlMax;
                                    maxima.erase(itlTemp);
                                }
                            else if(vecHistogram[*itlMax] > vecHistogram[*(--itlTemp)] ) {
                                    ++itlMax;
                                    maxima.erase(itlTemp);
                                }
                                else {
                                    ++itlMax;
                                    maxima.erase(++itlTemp);
                                }
                        }
                    else {
                            minima.erase(itlMin2);
                            itlMin2 = itlMin1;
                            ++itlMin2;

                            std::list<int>::iterator itlTemp = itlMax++;

                            if(itlMin2 == minima.end()) maxima.erase(itlTemp);
                            else if(vecHistogram[*itlTemp] < vecHistogram[*itlMax]) maxima.erase(itlTemp);
                                else {
                                    itlMax=itlTemp++;
                                    maxima.erase(itlTemp);
                                }
                        }

                }
            else {
                    /*debug*/   if(debug) std::cout << "+++ PEAK +++ PEAK +++ PEAK +++ PEAK +++" << std::endl;
                    ++itlMin1;
                    ++itlMin2;
                    ++itlMax;
                }



        }

    numAvgSegHeight = 0; //reseting for new calculation;
    itlMin = minima.begin();
    itlMax = maxima.begin();
    int maxUp, maxDown;
    maxUp = startPoint - 1;

    for (; itlMin != minima.end() ;){

            /*debug*/if(debug) matHistogram.at<int>(*(itlMin))=picOriginal.cols;
            maxDown = *(itlMax++);

            if(++itlMin != minima.end()){
                    std::vector<Point> segment;
                    segment.push_back(Point(0, maxUp - 1));
                    segment.push_back(Point(picOriginal.cols, maxUp  - 1));
                    segment.push_back(Point(picOriginal.cols, maxDown + 1));
                    segment.push_back(Point(0, maxDown + 1));


                    numAvgSegHeight += maxDown - maxUp;
                    segTextLines.push_back(segment);
                    maxUp = maxDown;
                }

            while(1){
                int temp = maxUp+1;
                if (vecHistogram[temp] == vecHistogram[maxUp]) ++maxUp;
                else break;
            }

            if (itlMin == minima.end()){
                  std::vector<Point> segment;
                  segment.push_back(Point(0, maxUp - 1));
                  segment.push_back(Point(picOriginal.cols, maxUp - 1));
                  segment.push_back(Point(picOriginal.cols, endPoint + 2));
                  segment.push_back(Point(0, endPoint + 2));

                  numAvgSegHeight += maxUp - endPoint+1;
                  segTextLines.push_back(segment);
                }
        }
    numAvgSegHeight /= segTextLines.size();

    /*debug*/ if(debug){
    /*debug*/   double minVal;
    /*debug*/   double maxVal;
    /*debug*/   minMaxLoc(matHistogram, &minVal, &maxVal);
    /*debug*/   Mat temp=matHistogram >= minVal;
    /*debug*/   for(int k=0;k<picOriginal.cols;k+=5)
    /*debug*/       hconcat(temp, matHistogram >= k, temp);
    /*debug*/
    /*debug*/   imwrite(debugLocation + "picSegmentation/histogram.png", temp);
    /*debug*/   imwrite(debugLocation + "picSegmentation/smudged.png", picEroded);
    /*debug*/   imwrite(debugLocation + "picSegmentation/segmented.png", drawRectangles());
    /*debug*/ }
}


void picSegmentation::setPoolVariable(std::string variableName, int variableValue){
        if (variableName == "numMinSegHeight") numMinSegHeight =  variableValue;
        if (variableName == "numAvgSegHeight") numAvgSegHeight =  variableValue;
        if (variableName == "numSmudgeSize") numSmudgeSize =  variableValue;
    }
void picSegmentation::setPoolVariable(std::string variableName, cv::Mat variableValue){
        if (variableName == "picOriginal") picOriginal =  variableValue;
        if (variableName == "picEroded") picEroded =  variableValue;
        if (variableName == "matHistogram") matHistogram =  variableValue;
    }
void picSegmentation::setPoolVariable(std::string variableName, std::vector<std::vector<cv::Point> > variableValue){
        if (variableName == "segTextLines") segTextLines = variableValue;
    }
void picSegmentation::setPoolVariable(std::string variableName, std::vector<int > variableValue){
        if (variableName == "vecHistogram") vecHistogram = variableValue;
    }
void picSegmentation::setPoolVariable(std::string variableName, bool variableValue){
        if (variableName == "debug") debug = variableValue;
    }
void picSegmentation::setPoolVariable(std::string variableName, std::string variableValue){
        if (variableName == "debugLocation") debugLocation = variableValue;
    }

void picSegmentation::getPoolVariable(std::string variableName, int &variableValue){
        if (variableName == "numMinSegHeight") variableValue = numMinSegHeight;
        if (variableName == "numAvgSegHeight") variableValue = numAvgSegHeight;
        if (variableName == "numSmudgeSize") variableValue = numSmudgeSize;
    }
void picSegmentation::getPoolVariable(std::string variableName, cv::Mat &variableValue){
        if (variableName == "picOriginal") variableValue = picOriginal;
        if (variableName == "picEroded") variableValue = picEroded;
        if (variableName == "matHistogram") variableValue = matHistogram;
    }
void picSegmentation::getPoolVariable(std::string variableName, std::vector<std::vector<cv::Point> > &variableValue){
        if (variableName == "segTextLines") variableValue = segTextLines;
    }
void picSegmentation::getPoolVariable(std::string variableName, std::vector<int> &variableValue){
        if (variableName == "vecHistogram") variableValue = vecHistogram;
    }
void picSegmentation::getPoolVariable(std::string variableName, bool &variableValue){
        if (variableName == "debug") variableValue = debug;
    }
void picSegmentation::getPoolVariable(std::string variableName, std::string &variableValue){
        if (variableName == "debugLocation") variableValue = debugLocation;
    }

