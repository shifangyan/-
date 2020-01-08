#ifndef WEBSOCKETCONTENT
#define WEBSOCKETCONTENT
#include <stdint.h>
#include <string>

class WebSocketContent
{
public:
	enum OpcodeType
	{
		kContinue = 0x00,
		kText = 0x01,
		kBinary = 0x02,
		kDisconnect = 0x08,
		kPing = 0x09,
		kPong = 0x0A
	};
	WebSocketContent():
		fin_(true)
	{}
	void setFin(bool fin)
	{
		fin_ = fin;
	}
	bool getFin() const
	{
		return fin_;
	}
	void setOpcode(OpcodeType code)
	{
		opcode_ = code;
	}
	OpcodeType getOpcode() const
	{
		return opcode_;
	}
	void setMask(bool mask)
	{
		mask_ = mask;
	}
	bool getMask() const
	{
		return mask_;
	}
	void setPayloadLength(uint64_t length)
	{
		payloadLength_ = length;
	}
	uint64_t getPayloadLength() const
	{
		return payloadLength_;
	}
	void setMaskKey(uint8_t key[4])
	{
		for (int i = 0; i < 4; i++)
		{
			maskKey_[i] = key[i];
		}
	}
	const uint8_t* getMaskKey() const
	{
		return maskKey_;
	}
	void setPayloadData(const std::string& data)
	{
		payloadData_ = data;
	}
	std::string getPayloadData() const
	{
		return payloadData_;
	}
private:
	bool fin_;
	OpcodeType opcode_;
	bool mask_;
	uint64_t payloadLength_;
	uint8_t maskKey_[4];
	std::string payloadData_;
};
#endif

