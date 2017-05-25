#ifndef PICTURE_PRE_PROCESS_H
#define PICTURE_PRE_PROCESS_H

class picProcessing{
	private:
		/*Pool START*/
		cv::Mat picOriginal;
		cv::Mat picCutOriginal;
		cv::Mat picCutBinary;

		std::vector<cv::Point> segFrame;

		bool debug;
		std::string debugLocation;
		/*Pool END*/

		double getDistance(cv::Point2f, cv::Point2f);
	public:

		picProcessing(){
			reset();

		}

		void reset(){
			picOriginal.release();
			picCutOriginal.release();
			picCutBinary.release();
		}

		void runPicProcessing();

        void setPoolVariable(std::string, cv::Mat);
        void setPoolVariable(std::string, std::vector<cv::Point>);
        void setPoolVariable(std::string, bool);
        void setPoolVariable(std::string, std::string);

        void getPoolVariable(std::string, cv::Mat &);
        void getPoolVariable(std::string, std::vector<cv::Point> &);
        void getPoolVariable(std::string, bool &);
        void getPoolVariable(std::string, std::string &);
};
#endif // !PICTURE_PRE_PROCESS_H
