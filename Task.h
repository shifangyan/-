#ifndef  __TASK
#define __TASK
//Task对应一个任务   主要保管该任务需要的视频流和对应的消息，目前只支持同时存在一个任务
#include<map>
#include<string>
#include"VideoStream.h"
#include<functional>
#include<memory>
#include "CJsonObject.hpp"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"
#include "ImageData.h"

class Message;
class AlgorithmServerCommunication;
class Configuration;
class FrontendServer;
enum  TaskType
{
	Detection,
	Track
};
enum ClassifyType
{
	Type1,  //json文件只包含一个float型数据
	Type2  //json文件包含一个string数据和一个float数据
};

enum PackageType
{
	kDetection,
	kClassify,
	kComparison
};

class Task
{
private:
	int task_id_;
	TaskType task_type_;
	//typedef  std::map<std::string, std::unique_ptr<VideoStream> > StreamMapType;
	//StreamMapType video_stream_managed_;
	std::unique_ptr<VideoStream> videoStream_;
	std::set < std::unique_ptr < VideoStream> > aroundVideoStream_; //只有在跟踪任务时才有意义
	typedef std::shared_ptr<Message> MessagePtr;
	std::map<int, MessagePtr> message_managed_;
	int message_ms_interval_;
	muduo::net::EventLoop* loop_;
	AlgorithmServerCommunication* algo_;
	std::map<std::string, ClassifyType> classify_map_;
	//muduo::net::TcpConnectionPtr connection_;
	uint32_t key_frame_count_;
	//WebSocketClientPtr webSocketClientPtr_;
	Configuration* config_;
	FrontendServer* frontendServer_;

	void encodeImage(neb::CJsonObject& json, const cv::Mat& image);
	//void encodeBox(neb::CJsonObject& json, const ImageData& imageData);
	void structureJson(neb::CJsonObject& json, ImageData& imageData, PackageType type);
public:
	Task(int task_id, TaskType task_type, muduo::net::EventLoop* loop, AlgorithmServerCommunication* algo,Configuration *config);
	//void addVideoStream(const std::string &str);
	//void delVideoStream(const std::string &str);
	void setVideoStream(int64_t id,const std::string& str)
	{
		videoStream_.reset(new VideoStream(id,str));
		//videoStream_->Start();
	}
	void start();
	void createMessage();
	//virtual void TaskProcess();
	Task(const Task& rhs) = delete;
	Task& operator=(const Task& rhs) = delete;
	int GetTaskID() {
		return task_id_;
	}
	bool messageExist(int message_id);
	void sendMessage(int message_id);
	void receiveMessage(neb::CJsonObject& json);
	const TaskType& getTaskType()
	{
		return task_type_;
	}
	void setFrontServer(FrontendServer* frontend)
	{
		frontendServer_ = frontend;
	}
	void sendFrontend(int messageID);
};
#endif // ! __TASK



