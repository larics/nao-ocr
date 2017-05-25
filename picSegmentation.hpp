#ifndef PIC_SEGMENTATION_H
#define PIC_SEGMENTATION_H

#include <list>

class picSegmentation{
	private:
		/*Pool START*/
		int numMinSegHeight;
		int numAvgSegHeight;
		int numSmudgeSize;

		cv::Mat picOriginal;
		cv::Mat picEroded;
		cv::Mat matHistogram;
		std::vector<int> vecHistogram;
		std::vector<std::vector<cv::Point> > segTextLines;

		bool debug;
		std::string debugLocation;
		/*Pool END*/

		cv::Mat drawRectangles();
	public:

		picSegmentation(){
			reset();
			numMinSegHeight = 10;
			numAvgSegHeight = 0;
			numSmudgeSize = 4;
			}

		void reset(){
			picEroded.release();
			segTextLines.clear();
			}

		void runPicSegmentation();

        void setPoolVariable(std::string, int);
        void setPoolVariable(std::string, cv::Mat);
        void setPoolVariable(std::string, std::vector<std::vector<cv::Point> >);
        void setPoolVariable(std::string, std::vector<int >);
        void setPoolVariable(std::string, bool);
        void setPoolVariable(std::string, std::string);

        void getPoolVariable(std::string, int &);
        void getPoolVariable(std::string, cv::Mat &);
        void getPoolVariable(std::string, std::vector<std::vector<cv::Point> > &);
        void getPoolVariable(std::string, std::vector<int > &);
        void getPoolVariable(std::string, bool &);
        void getPoolVariable(std::string, std::string &);
};
#endif // !PIC_SEGMENTATION_H
