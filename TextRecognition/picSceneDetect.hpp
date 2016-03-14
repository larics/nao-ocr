#ifndef PICTURE_SCENE_DETECT_H
#define PICTURE_SCENE_DETECT_H

class picSceneDetection{
private:
    /*Pool START*/
    cv::Mat picture;
    std::vector<cv::Point> segFrame;
    std::vector<std::vector<cv::Point> > segRectangles;

    bool debug;
    std::string debugLocation;
    /*Pool END*/

    cv::Mat drawRectangles();
    double getAngle(cv::Point, cv::Point , cv::Point);
    void findRectangles(cv::Mat& );
public:
    picSceneDetection(){
            reset();
        }

    void reset(){
        picture.release();
        debug = false;
    }

    void runSceneDetection();

    void setPoolVariable(std::string, cv::Mat);
    void setPoolVariable(std::string, std::vector<cv::Point>);
    void setPoolVariable(std::string, std::vector<std::vector<cv::Point> >);
    void setPoolVariable(std::string, bool);
    void setPoolVariable(std::string, std::string);

    void getPoolVariable(std::string, cv::Mat &);
    void getPoolVariable(std::string, std::vector<cv::Point> &);
    void getPoolVariable(std::string, std::vector<std::vector<cv::Point> > &);
    void getPoolVariable(std::string, bool &);
    void getPoolVariable(std::string, std::string &);
};

#endif // !PICTURE_SCENE_DETECT_H
