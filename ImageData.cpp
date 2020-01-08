#include "ImageData.h"


ImageData::ImageData(const std::string& imageBase64):imageBase64_(imageBase64)
{
}

void ImageData::addData(const Pedestrian& data)
{
	data_.push_back(data);
}

const std::string& ImageData::getImageBase64() const
{
	// TODO: 在此处插入 return 语句
	return imageBase64_;
}

std::vector<Pedestrian>& ImageData::getDatas()
{
	// TODO: 在此处插入 return 语句
	return data_;
}
