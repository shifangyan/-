#ifndef  __MESSAGE
#define __MESSAGE
#include"ImageData.h"
#include<vector>
#include<map>
#include<string>
#include"VideoStream.h"
#include"muduo/net/EventLoop.h"
#include"muduo/net/TcpConnection.h"
#include"Task.h"
enum MessageState
{
	kWaitSendDetection,
	kWaitReceiveDetection,
	kWaitSendClassification,
	kWaitReceiveClassification,
	kWaitSendComparison,
	kWaitReceiveComparison,
//	kWaitSendJavaBackend,
//	kWaitReceiveJavaBackend,
	kWaitClose
};

//struct ClassifyReceive
//{
//	bool have_receive_;
//	ClassifyType type_;
//};

class Message
{
private:
	//std::vector<ImageData> image_datas_;
	ImageData imageData_;
	MessageState state_;
	//std::string task_type_;
	std::map<std::string, bool> classify_receive_map_;
	bool keyFrame_;
public:
	Message(const std::map<std::string, ClassifyType>& map,const ImageData& imageData);
	//void AddImageData(const ImageData& image_data);
	//void MessageProcess(neb::CJsonObject &json); 
	MessageState State();
	void SetState(const MessageState state) {
		state_ = state;
	}
	std::string StateToString(const MessageState& state);
	ImageData& getImageData()
	{
		return imageData_;
	}
	void ReceiveClassify(const std::string& classify_name);
	bool IsReceiveAllClassify();
	void setKeyFrame(bool keyFrame)
	{
		keyFrame_ = keyFrame_;
	}
	const bool& isKeyFrame() const
	{
		return keyFrame_;
	}
};
#endif // ! __MESSAGE



