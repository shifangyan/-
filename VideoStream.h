//����Ϊ��Ƶ���࣬ÿһ�����Ӧһ����Ƶ��
#ifndef  __VIDEOSTREAM
#define __VIDEOSTREAM
#include<string>
#include<opencv2/opencv.hpp>
#include<memory>
#include<condition_variable>
#include<mutex>
#include<thread>

class VideoStream
{
private:
	std::string videoAddr_; //��Ƶ����ַ
	int videoWidth_, videoHeight_;
	int videoFps_;
	void ThreadFunc();
	std::condition_variable condition_;
	mutable std::mutex mutex_;
	//std::shared_ptr<cv::Mat> frame_;
	std::string frameBase64_;
	bool opened_;
	std::thread readVideoThread_;
	bool leave_;
	bool start_;
	int64_t cameraID_;
public:
	VideoStream(int64_t id,std::string addr);
	~VideoStream();
	//void Start();  //��ʼ�ɼ���Ƶ
	std::string getFrameBase64() const; //��ȡ��Ƶ��
	VideoStream(const VideoStream& rhs) = delete;  //��������
	VideoStream& operator=(const VideoStream& rhs) = delete; //��������
	bool isOpened() 
	{
		mutex_.lock();
		bool res = opened_;
		mutex_.unlock();
		return res;
	}
	int64_t getCameraID() const
	{
		return cameraID_;
	}
	//VideoStream(VideoStream&& rhs);
	//VideoStream& operator=(VideoStream&& rhs);
};
#endif // ! __VIDEOSTREAM



