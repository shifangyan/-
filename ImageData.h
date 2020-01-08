#ifndef  __IMAGEDATA
#define __IMAGEDATA
//此类为图片资源类，包含一张图片以及检测的行人坐标和属性
#include<opencv2/opencv.hpp>
#include<vector>
#include"Pedestrian.h"
#include<string>
#include<memory>

class ImageData
{
private:
	std::string stream_name_;
	//cv::Mat image_;
	//typedef std::shared_ptr<cv::Mat> ImagePtr;
	//ImagePtr image_;
	std::string imageBase64_;
	std::vector<Pedestrian> data_;
public:
	ImageData(const std::string& imageBase64);
	void addData(const Pedestrian& data);
	const std::string& getImageBase64() const;
	std::vector<Pedestrian>& getDatas();
	const std::vector<Pedestrian>& getDatas() const
	{
		return data_;
	}
};
#endif // ! __IMAGEDATA



