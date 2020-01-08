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
	// TODO: �ڴ˴����� return ���
	return imageBase64_;
}

std::vector<Pedestrian>& ImageData::getDatas()
{
	// TODO: �ڴ˴����� return ���
	return data_;
}
