#include "VideoStream.h"
#include <thread>
#include <muduo/base/Logging.h>
#include "Base64.h"
#include <chrono>

void VideoStream::ThreadFunc()
{
	
	//	std::unique_lock<std::mutex> lck(mutex_);
	//	while (!start_)
	//		condition_.wait(lck);

		//打开视频流
	cv::VideoCapture video(videoAddr_);
	if (!video.isOpened())
	{
		std::cout << "error:" << videoAddr_ << "can not open" << std::endl;
		exit(-1);
	}

		//获取视频流相关信息
	videoWidth_ = static_cast<int>(video.get(cv::CAP_PROP_FRAME_WIDTH));
	videoHeight_ = static_cast<int>(video.get(cv::CAP_PROP_FRAME_HEIGHT));
	videoFps_ = static_cast<int>(video.get(cv::CAP_PROP_FPS));
	LOG_INFO << "success open the stream: " << videoAddr_
		<< "width: " << videoWidth_
		<< "height: " << videoHeight_
		<< "fps: " << videoFps_;
	if (videoFps_ <= 0 || videoFps_ >= 30)
		videoFps_ = 25;
	
	cv::Mat frame;
	while (1)
	{
		{
			std::lock_guard<std::mutex> lck(mutex_);
			auto startTime = std::chrono::high_resolution_clock::now();
			if (leave_)
			{
				mutex_.unlock();
				break;
			}
			//frame_.reset(new cv::Mat());
			do
			{
				video >> frame;
			} while (frame.empty());
			opened_ = true;
			std::vector<unsigned char> data_encode;
			int res = cv::imencode(".jpg", frame, data_encode);
			if (!res)
			{
				std::cout << "imencode failed" << std::endl;
			}
			frameBase64_.clear();
			base64_encode((const char*)&(*data_encode.begin()), data_encode.size(), frameBase64_);
			auto endTime = std::chrono::high_resolution_clock::now();
			LOG_DEBUG << "send  time:" << (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime)).count();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

VideoStream::VideoStream(int64_t id, std::string addr):
	videoAddr_(addr),
	videoWidth_(0),
	videoHeight_(0),
	videoFps_(0),
	condition_(),
	mutex_(),
	//frame_(new cv::Mat()),
	opened_(false),
	readVideoThread_(&VideoStream::ThreadFunc, this),
	leave_(false),
	start_(false),
	cameraID_(id)
{
}

VideoStream::~VideoStream()
{
	mutex_.lock();
	leave_ = true;
	mutex_.unlock();
	readVideoThread_.join(); //等待子线程结束 ，这应该在主线程执行
}

//void VideoStream::Start()
//{
//	if (opened_)
//	{
//		std::cout << "error: the video stream opened, can not open twice" << std::endl;
//		exit(-1);
//	}
//	//read_video_thread_ = std::move(std::thread(&VideoStream::ThreadFunc, this));
//	std::unique_lock<std::mutex> lck(mutex_);
//	start_ = true;
//	condition_.notify_all();
//}

std::string VideoStream::getFrameBase64() const
{
	std::lock_guard<std::mutex> lck(mutex_);
	if (!opened_)
	{
		LOG_ERROR << "error: the video stream not open";
		return std::string();
	}
	return frameBase64_;
}

