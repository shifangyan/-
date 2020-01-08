//该类为视频流类，每一个类对应一个视频流
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
	std::string videoAddr_; //视频流地址
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
	//void Start();  //开始采集视频
	std::string getFrameBase64() const; //获取视频流
	VideoStream(const VideoStream& rhs) = delete;  //不允许拷贝
	VideoStream& operator=(const VideoStream& rhs) = delete; //不允许拷贝
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



