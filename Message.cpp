#include "Message.h"




Message::Message(const std::map<std::string, ClassifyType>& map, const ImageData& imageData):
	state_(MessageState::kWaitSendDetection),
	keyFrame_(false),
	imageData_(imageData)
{
	for (auto iter = map.begin(); iter != map.end(); ++iter)
	{
		classify_receive_map_.insert({ iter->first,false });
	}
}

//void Message::AddImageData(const ImageData& image_data)
//{
//	image_datas_.push_back(image_data);
//}

//void Message::MessageProcess(neb::CJsonObject& json)
//{
//	std::string taskType;
//	bool ret = json["content"].Get("taskType", taskType);
//	if (!ret)
//	{
//		std::cout << "can not find taskType field" << std::endl;
//		return;
//	}
//
//	if (state_ == MessageState::DetectionSent && taskType == std::string("Detec"))
//	{
//		//处理检测结果 并放入Imagedata中
//		std::cout << "process Detec result" << std::endl;
//		state_ = MessageState::DetectionReceive;
//	}
//	else if (state_ == MessageState::ClassificationSent && taskType == std::string("classify"))
//	{
//		//处理分类结果 并放入Imagedata中
//		std::cout << "process classify result" << std::endl;
//		//classify_count_++;
//		if (classify_count_ == classify_num_)
//			state_ = MessageState::ClassificationReceive;
//	}
//	else if (state_ == MessageState::ComparisonSent && taskType == std::string("comparison") && task_type_ == "Track")
//	{
//		//处理比对结果 并放入Imagedata中
//		std::cout << "process comparison result" << std::endl;
//		state_ = MessageState::ComparisonReceive;
//	}
//	else
//	{
//		std::cout << "taskType:" << task_type_ << "  "
//			<< "state_:" << StateToString(state_) << "  "
//			<< "receive the message type:" << taskType << "  "
//			<< std::endl;
//	}
//}

MessageState Message::State()
{
	return state_;
}

std::string Message::StateToString(const MessageState& state)
{
	static char* str[9] = { "kWaitSendDetection","kWaitReceiveDetection","kWaitSendClassification","kWaitReceiveClassification",
	"kWaitSendComparison","kWaitReceiveComparison","kWaitSendJavaBackend","kWaitReceiveJavaBackend","kWaitClose"};
	return std::string(str[state]);
}

void Message::ReceiveClassify(const std::string& classify_name)
{
	auto iter = classify_receive_map_.find(classify_name);
	if (iter == classify_receive_map_.end())
	{
		std::cout << "error:cnt not find the classify name" << std::endl;
		return;
	}
	classify_receive_map_[classify_name] = true;
}

bool Message::IsReceiveAllClassify()
{
	for (auto iter = classify_receive_map_.begin(); iter != classify_receive_map_.end(); ++iter)
	{
		if (iter->second == false)
			return false;
	}
	return true;
}


