#include "Task.h"
#include"Base64.h"
#include"Message.h"
#include "Configuration.h"
#include "AlgorithmServerCommunication.h"
#include "muduo/base/Logging.h"
#include "muduo/net/http/HttpResponse.h"
#include "HttpClient.h"
#include "muduo/base/Logging.h"
#include "FrontendServer.h"
#include <chrono>


void Task::encodeImage(neb::CJsonObject& json,const cv::Mat& image)
{
	//auto& imageData = message_managed_[message_id]->getImageData();
	std::vector<unsigned char> data_encode;
//	cv::Mat image = *(imageData.GetImage().get());
	int res = cv::imencode(".jpg", image, data_encode);
	if (!res)
	{
		std::cout << "imencode failed" << std::endl;
	}
	std::string str_encode(data_encode.begin(), data_encode.end());
	const char* c = str_encode.c_str();
	std::string str_base64;
	base64_encode(str_encode.c_str(), str_encode.size(), str_base64);
	json["image"].Add(str_base64);
}

void Task::structureJson(neb::CJsonObject& json, ImageData& imageData, PackageType type)
{
	//json.Add("source_camera_id", videoStream_->getCameraID());
	////编码视频帧
	//std::vector<unsigned char> data_encode;
	////	cv::Mat image = *(imageData.GetImage().get());
	//auto image = *imageData.GetImage();
	//int res = cv::imencode(".jpg", image, data_encode);
	//if (!res)
	//{
	//	std::cout << "imencode failed" << std::endl;
	//}
	//std::string str_encode(data_encode.begin(), data_encode.end());
	//const char* c = str_encode.c_str();
	//std::string str_base64;
	//base64_encode(str_encode.c_str(), str_encode.size(), str_base64);
	//json.Add("src_img_base64", str_base64);
	////坐标
	//json.AddEmptySubArray("image_info");
	//auto& datas = imageData.GetDatas();
	//for (auto data : datas)
	//{
	//	neb::CJsonObject son_json;
	//	//坐标信息
	//	std::string pos;
	//	auto pos_json = data.GetPosition();
	//	int xmin;
	//	pos_json.Get("xmin", xmin);
	//	pos += xmin + ",";
	//	int ymin;
	//	pos_json.Get("ymin", ymin);
	//	pos += ymin + ',';
	//	int xmax;
	//	pos_json.Get("xmax", xmax);
	//	pos += xmax + ',';
	//	int ymax;
	//	pos_json.Get("ymax", ymax);
	//	pos += ymax;
	//	son_json.Add("position", pos);

	//	//属性信息
	//	if (type == PackageType::kClassify)
	//	{
	//		auto& attributesMap = data.GetAllAtribute();
	//		for (auto item : attributesMap)
	//		{
	//			son_json.Add(item.first, item.second);
	//		}
	//	}
	//	else if (type == PackageType::kComparison) //比对信息
	//	{

	//	}

	//	json["image_info"].Add(son_json);
	//}
}

//void Task::encodeBox(neb::CJsonObject& json, const ImageData& imageData)
//{
//	std::string array_data;
//	if (i == 0)
//		array_data += "[";
//	else
//		array_data += ",[";
//
//	auto datas = imageData.GetDatas();
//	for (int j = 0; j < datas.size(); j++)
//	{
//		neb::CJsonObject position_json = datas[j].GetPosition();
//		if (j == 0)
//			array_data += position_json.ToString();
//		else
//			array_data += "," + position_json.ToString();
//	}
//	array_data += "]";
//	//	std::cout << "array_data:" << array_data << std::endl;
//	neb::CJsonObject json(array_data);
//	//std::cout << "json:" << json.ToString() << std::endl;
//	content_json["box"].Add(json);
//	//std::cout << "content_json" << content_json.ToString() << std::endl;
//}

Task::Task(int task_id, TaskType task_type, muduo::net::EventLoop* loop, AlgorithmServerCommunication* algo, Configuration* config):
	task_id_(task_id),
	task_type_(task_type),
	message_ms_interval_(40), //默认40ms一次
	loop_(loop),
	algo_(algo),
	key_frame_count_(0),
	config_(config)
{
	auto& algo_server_info = config->algorithmServerInfo();
	for (auto iter = algo_server_info.begin(); iter != algo_server_info.end(); ++iter)
	{
		if (iter->first.substr(0, 8) == std::string("classify"))
		{
			if (iter->second.type_ == 1)
				classify_map_.insert({ iter->first.substr(9),ClassifyType::Type1} ); //要求分类服务在配置文件config的name格式为：classify-label
			else if (iter->second.type_ == 2)
				classify_map_.insert({ iter->first.substr(9),ClassifyType::Type2} );
			else
				std::cout << "the config.txt type filed illegal" << std::endl;
		}
	}
}

//void Task::addVideoStream(const std::string& str)
//{
//	//video_stream_managed_.insert({ std::move(str),std::move(ptr)});
//	video_stream_managed_.emplace(std::move(str), std::unique_ptr<VideoStream>(new VideoStream(str)));
//	video_stream_managed_[str]->Start();
//}

//void Task::delVideoStream(const std::string& str)
//{
//	video_stream_managed_.erase(str);
//}



void Task::start()
{
	loop_->runEvery((double)message_ms_interval_ / 1000.0, std::bind(&Task::createMessage, this));
}


//每隔message_ms_interval_ (ms)会调用一次该函数来创建message，并发送
void Task::createMessage()
{
	//从视频流获取图片，如果一张图片都没有，则message无效 不会发送到网络上
	static int message_id = 0;
	if (videoStream_->isOpened())
	{
		ImageData image_data(videoStream_->getFrameBase64());
		MessagePtr message(new Message(classify_map_,image_data));
		key_frame_count_++;
		if ((message_ms_interval_ * key_frame_count_) % 1000 == 0) //超过1s
		{
			message->setKeyFrame(true);
		}
		
			//开始任务处理
		message_managed_[message_id] = message;
		sendMessage(message_id++);
	}
}


bool Task::messageExist(int message_id)
{
	return message_managed_.find(message_id) == message_managed_.end()? false:true;
}

void Task::sendMessage(int message_id)
{
	sendFrontend(message_id);
	/*int messageID;
	bool ret = json.Get("messageID", messageID);
	if (!ret)
	{
		std::cout << "the message not find messageID field" << std::endl;
		return;
	}*/
	//if (!messageExist(message_id))
	//{
	//	LOG_ERROR << "the message not exist";
	//	return;
	//}
	//

	//neb::CJsonObject send_json;
	//send_json.Add("connectType", "long");//long
	//send_json.Add("stopSign", "keep");
	//send_json.Add("taskID", task_id_);
	//send_json.Add("messageID", message_id);
	//
	//if (message_managed_[message_id]->State() == MessageState::kWaitSendDetection) //发送检测包
	//{
	//	send_json.Add("taskType", "detec");
	//	neb::CJsonObject content_json;
	//	content_json.AddEmptySubArray("image");
	//	auto& imageData = message_managed_[message_id]->getImageData();
	//	cv::Mat image = *(imageData.GetImage().get());
	//	encodeImage(content_json, image);
	//	
	//	//跟踪任务再说
	//	if (task_type_ == TaskType::Track)
	//	{

	//	}
	//	send_json.Add("content", content_json);
	//	
	//	//发送检测包并修改状态
	//	algo_->Send("detec", send_json);
	//	message_managed_[message_id]->SetState(MessageState::kWaitReceiveDetection);
	//}
	//else if (message_managed_[message_id]->State() == MessageState::kWaitSendClassification) //发送分类包
	//{
	//	send_json.Add("taskType", "classify");
	//	neb::CJsonObject content_json;
	//	content_json.AddEmptySubArray("image");
	//	auto& imageData = message_managed_[message_id]->getImageData();
	//	cv::Mat image = *(imageData.GetImage().get());
	//	encodeImage(content_json, image);

	//	//box
	//	content_json.AddEmptySubArray("box");
	//	std::vector<ImageData> imageDatas;
	//	imageDatas.push_back(imageData);
	//	for (int i = 0; i < imageDatas.size(); i++)
	//	{
	//		std::string array_data;
	//		if (i == 0)
	//			array_data += "[";
	//		else
	//			array_data += ",[";
	//		
	//		auto datas = imageDatas[i].GetDatas();
	//		for (int j = 0; j < datas.size(); j++)
	//		{
	//			neb::CJsonObject position_json = datas[j].GetPosition();
	//			if(j == 0)
	//				array_data += position_json.ToString();
	//			else
	//				array_data += "," + position_json.ToString();
	//		}
	//		array_data += "]";
	//	//	std::cout << "array_data:" << array_data << std::endl;
	//		neb::CJsonObject json(array_data);
	//		//std::cout << "json:" << json.ToString() << std::endl;
	//		content_json["box"].Add(json);
	//		//std::cout << "content_json" << content_json.ToString() << std::endl;
	//	}
	//	send_json.Add("content", content_json);
	//	//发送分类包并修改状态
	//	for (auto iter = classify_map_.begin(); iter != classify_map_.end(); iter++)
	//	{
	//		if (!send_json.Add("classifyType", iter->first))
	//			send_json.Replace("classifyType", iter->first);
	//		//std::cout << send_json.ToFormattedString() << std::endl;
	//		algo_->Send(iter->first, send_json);
	//	}
	//	message_managed_[message_id]->SetState(MessageState::kWaitReceiveClassification);
	//}
	//else if (message_managed_[message_id]->State() == MessageState::kWaitSendComparison) //发送比对包
	//{
	//	//暂时不写
	//}
	//else
	//{
	//	LOG_ERROR << "the message state is error";
	//}
}

void Task::receiveMessage(neb::CJsonObject& json)
{
	int messageID;
	bool ret = json.Get("messageID", messageID);
	if (!ret)
	{
		LOG_ERROR << "the message not find messageID field";
		return;
	}
	if (!messageExist(messageID))
	{
		LOG_ERROR << "the message not exist";
		return;
	}
	auto& message = message_managed_[messageID];

	std::string taskType;
	ret = json.Get("taskType",taskType);
	if (!ret)
	{
		LOG_ERROR << "the json can not find taskType field";
		return;
	}

	//int count = 0;
	if (taskType == std::string("detec") && message->State() == MessageState::kWaitReceiveDetection)
	{
		auto& imageData = message->getImageData();
		//for (int i = 0; i < json["content"]["box"].GetArraySize(); i++)
		//{
			for (int j = 0; j < json["content"]["box"][0].GetArraySize(); j++)
			{
				neb::CJsonObject box;
				box = json["content"]["box"][0][j];
				int xmin, ymin, xmax, ymax;
				box.Get("xmin", xmin);
				box.Get("xmax", xmax);
				box.Get("ymin", ymin);
				box.Get("ymax", ymax);
				Pedestrian p(xmin, xmax, ymin, ymax);
				imageData.addData(p);
			}
	//	}
		
		LOG_INFO << "receive detection success";
		//通过websocket发送视频流到前端
	//	if (webSocketClientPtr_->getConnected())
	//	{
	//		neb::CJsonObject frontend_json;
	//		structureJson(frontend_json, imageData, PackageType::kDetection);
	//		webSocketClientPtr_->send(frontend_json.ToString());
	//	}
		
		if (!message->isKeyFrame()) //非关键帧 直接销毁
		{
			message->SetState(MessageState::kWaitClose);
		}
		else //发送分类包
		{
			message->SetState(MessageState::kWaitSendClassification);
			sendMessage(messageID);
		}
	}
	else if (taskType == std::string("classify") && message->State() == MessageState::kWaitReceiveClassification)
	{
		std::string classify_type;
		bool ret = json.Get("classifyType", classify_type);
		if (!ret)
		{
			LOG_ERROR << "the json can not find classifyType field";
			return;
		}
		for (auto iter = classify_map_.begin(); iter != classify_map_.end(); ++iter)
		{
			if (iter->first == classify_type)
			{
				if (iter->second == ClassifyType::Type1)
				{
					auto& imageData = message->getImageData();
				//	for (int i = 0; i < json["content"][classify_type].GetArraySize(); i++)
				//	{
						auto pedestrians = imageData.getDatas();
						for (int j = 0; j < json["content"][classify_type][0].GetArraySize(); j++)
						{
							float prob;
							json["content"][classify_type][0].Get(j, prob);
							pedestrians[j].UpdateAttribute(classify_type, std::to_string(prob));
						}
				//	}
				}
				else
				{
					auto& image_datas = message->getImageData();
				//	for (int i = 0; i < json["content"][classify_type].GetArraySize(); i++)
				//	{
						auto pedestrians = image_datas.getDatas();
						for (int j = 0; j < json["content"][classify_type][0].GetArraySize(); j++)
						{
							neb::CJsonObject color_json;
							json["content"][classify_type][0].Get(j, color_json);
							std::string color;
							color_json.Get("color", color);
							int prob;
							color_json.Get("prob", prob);
							pedestrians[j].UpdateAttribute(classify_type, color+":" + std::to_string(prob));
						}
				//	}
				}
				message->ReceiveClassify(classify_type);
				LOG_ERROR << classify_type << " receive success";
			}
		}
		if (message->IsReceiveAllClassify())
		{
			LOG_INFO << "ClassificationReceive success";
			if (getTaskType() == TaskType::Detection)
			{
				message->SetState(MessageState::kWaitClose);
				//发往java后台
				auto& backendHttpInfo = config_->getBackendAddr();
				neb::CJsonObject send_json;
				auto& imageData = message->getImageData();
				structureJson(send_json, imageData, PackageType::kClassify);
				HttpRequest httpRequest;
				httpRequest.setHttpMethod(HttpRequest::HttpMethod::kPost);
				httpRequest.setPath(backendHttpInfo.path_);
				httpRequest.setHttpVersion(HttpRequest::HttpVersion::kHttp1_1);
				std::string body = send_json.ToString();
				httpRequest.addHeader("Host", backendHttpInfo.addr_.toIpPort());
				httpRequest.addHeader("Content-Length", std::to_string(body.size()));
				httpRequest.setBody(body);
				muduo::net::HttpClient httpClient(loop_, backendHttpInfo.addr_);
				httpClient.sendRequest(httpRequest); 
			}
			else
			{
				message->SetState(MessageState::kWaitSendComparison);
			}
			//发送比对包或者java后台包
			sendMessage(messageID);
		}
	}
	else if (taskType == std::string("comparison") && message->State() == MessageState::kWaitReceiveComparison)
	{
		//暂时不写
	}
	
	if (message->State() == MessageState::kWaitClose)
	{
		message_managed_.erase(messageID);
	}
}

void Task::sendFrontend(int messageID)
{
	static decltype(std::chrono::high_resolution_clock::now()) lastTime = std::chrono::high_resolution_clock::now();
	auto nowTime = std::chrono::high_resolution_clock::now();
	LOG_DEBUG << (std::chrono::duration_cast<std::chrono::milliseconds>(nowTime - lastTime)).count();
	lastTime = nowTime;
	if (!messageExist(messageID))
	{
		LOG_ERROR << "the message not exist";
		return;
	}

	neb::CJsonObject json;
	auto& message = message_managed_[messageID];
	auto& imageData = message->getImageData();
	const std::string& frameBase64 = imageData.getImageBase64();
	
	//std::string str_encode(data_encode.begin(), data_encode.end());
	//const char* c = str_encode.c_str();
	auto startTime = std::chrono::high_resolution_clock::now();
	json.Add("srcImgBase64",frameBase64);
	auto endTime = std::chrono::high_resolution_clock::now();
	LOG_DEBUG << "json add time:" << (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime)).count();
	
	startTime = std::chrono::high_resolution_clock::now();
	frontendServer_->send(json.ToString());
	endTime = std::chrono::high_resolution_clock::now();
	LOG_DEBUG << "send  time:" << (std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime)).count();
}


//void Task::TaskProcess()
//{
//}
