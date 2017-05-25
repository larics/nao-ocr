#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "picSceneDetect.hpp"

using namespace cv;

/*Draw rectangels*/
cv::Mat picSceneDetection::drawRectangles(){
    Mat image = picture.clone();
    std::vector<std::vector<Point> > segTemp;
    segTemp.push_back(segFrame);

    drawContours(image, segRectangles, -1, Scalar(255,0,0), 3);
    drawContours(image, segTemp, -1, Scalar(0,255,0), 3);

    return image;
}

/*Angle between 3 points on a plane*/
double picSceneDetection::getAngle( Point pt1, Point pt2, Point pt0 ){
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/*Rectangle detection*/
/*Original code from stackoverflow.com by user: karlphillip*/
void picSceneDetection::findRectangles(Mat& image){
    Mat blurred(image);
    medianBlur(image, blurred, 3);

    Mat gray0(blurred.size(), CV_8U), gray;
    vector<vector<Point> > contours;

    // try several threshold levels
    const int threshold_level = 5;
    for (int k = 1; k < threshold_level; k++)
    {
        gray = blurred >= (k+1) * 255 / threshold_level;

        findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

        vector<Point> approx;
        for (size_t i = 0; i < contours.size(); i++)
        {
                // approximate contour with accuracy proportional to the contour perimeter
                approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);

                if (approx.size() == 4 && fabs(contourArea(Mat(approx))) > 1000 && isContourConvex(Mat(approx)))
                {
                        double maxCosine = 0;

                        for (int j = 2; j < 5; j++)
                        {
                                double cosine = fabs(getAngle(approx[j%4], approx[j-2], approx[j-1]));
                                maxCosine = MAX(maxCosine, cosine);
                        }

                        if (maxCosine < 0.3)
                                segRectangles.push_back(approx);
                }
        }
    }

}

void picSceneDetection::runSceneDetection(){
        Mat picGray;


        /*Picture to grayscale*/
        cvtColor(picture,picGray,CV_RGB2GRAY);


        /*Rectangle detection*/

        findRectangles(picGray);

        int maxIndex=-1, maxArea=0;
        for (int k=0; k < segRectangles.size(); k++){
            vector<Point> rectangle;
            approxPolyDP(Mat(segRectangles[k]), rectangle, arcLength(Mat(segRectangles[k]), true)*0.02, true);
            int area = fabs(contourArea(Mat(rectangle)));
            if (area > maxArea){
                maxArea = area;
                    maxIndex=k;
            }
        }

        segFrame = segRectangles[maxIndex];

        /*debug*/ if (debug) imwrite(debugLocation + "picSceneDetect/rectangles.png", drawRectangles());

    }

void picSceneDetection::setPoolVariable(std::string variableName, cv::Mat variableValue){
        if (variableName == "picture") picture =  variableValue;
    }
void picSceneDetection::setPoolVariable(std::string variableName, std::vector<cv::Point> variableValue){
        if (variableName == "segFrame") segFrame = variableValue;
    }
void picSceneDetection::setPoolVariable(std::string variableName, std::vector<std::vector<cv::Point> > variableValue){
        if (variableName == "segRectangles") segRectangles = variableValue;
    }
void picSceneDetection::setPoolVariable(std::string variableName, bool variableValue){
        if (variableName == "debug") debug = variableValue;
    }
void picSceneDetection::setPoolVariable(std::string variableName, std::string variableValue){
        if (variableName == "debugLocation") debugLocation = variableValue;
    }

void picSceneDetection::getPoolVariable(std::string variableName, cv::Mat &variableValue){
        if (variableName == "picture") variableValue = picture;
    }
void picSceneDetection::getPoolVariable(std::string variableName, std::vector<cv::Point> &variableValue){
        if (variableName == "segFrame") variableValue = segFrame;
    }
void picSceneDetection::getPoolVariable(std::string variableName, std::vector<std::vector<cv::Point> > &variableValue){
        if (variableName == "segRectangles") variableValue = segRectangles;
    }
void picSceneDetection::getPoolVariable(std::string variableName, bool &variableValue){
        if (variableName == "debug") variableValue = debug;
    }
void picSceneDetection::getPoolVariable(std::string variableName, std::string &variableValue){
        if (variableName == "debugLocation") variableValue = debugLocation;
    }

