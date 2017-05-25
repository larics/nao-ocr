#ifndef OCR_MODULE_H
#define OCR_MODULE_H

#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

# include <alcommon/almodule.h>


using namespace cv;

namespace AL
{
  // This is a forward declaration of AL:ALBroker which
  // avoids including <alcommon/albroker.h> in this header
  class ALBroker;
}


class OcrModule : public AL::ALModule
{
private:
    /*Pool START*/
    Mat picOriginal;
    Mat picProcesed;
    Mat picProcesedBin;
    int numAvgSegHeight;
    std::vector<std::vector<Point> > segLines;
    std::vector<Point> segFrame;
    std::string ocrLang;
    std::string debugLocation;
    bool debug;
    /*Pool END*/

    tesseract::TessBaseAPI *tess;

public:

    OcrModule(boost::shared_ptr<AL::ALBroker>, const std::string&);

    virtual ~OcrModule();

    virtual void init();

    void reset();

    void setPoolVariable(const std::string&, const AL::ALValue & );

    AL::ALValue getPoolVariable(const std::string&);

    void runSceneDetection();

    void runPictureProcessing();

    int runPictureSegmentation();

    std::string runTextRecognition();

    std::string runSegmentRecognition(const int&);

};

#endif //OCR_MODULE_H
