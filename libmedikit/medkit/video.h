#ifndef _VIDEO_H_
#define _VIDEO_H_
#include "config.h"
#include "media.h"
#include "codecs.h"

class VideoFrame : public MediaFrame
{
public:
	VideoFrame(VideoCodec::Type codec,DWORD size, bool own = true) : MediaFrame(MediaFrame::Video,size, own)
	{
		//Store codec
		this->codec = codec;
		//Init values
		isIntra = 0;
		width = 0;
		height = 0;
		SetH264NalSizeLength(0);
	}

	virtual MediaFrame* Clone()
	{
		//Create new one
		VideoFrame *frame = new VideoFrame(codec,length);
		//Copy
		memcpy(frame->GetData(),buffer,length);
		//Set length
		frame->SetLength(length);
		//Size
		frame->SetWidth(width);
		frame->SetHeight(height);
		//Set intra
		frame->SetIntra(isIntra);
		//Set timestamp
		frame->SetTimestamp(GetTimeStamp());
		//Set duration
		frame->SetDuration(GetDuration());
		//Check if it has rtp info
		for (MediaFrame::RtpPacketizationInfo::iterator it = rtpInfo.begin();it!=rtpInfo.end();++it)
		{
			//Gete info
			MediaFrame::RtpPacketization *rtp = (*it);
			//Add it
			frame->AddRtpPacket(rtp->GetPos(),rtp->GetSize(),rtp->GetPrefixData(),rtp->GetPrefixLen(), rtp->IsMark());
		}
		//Return it
		return (MediaFrame*)frame;
	}
	
	VideoCodec::Type GetCodec()	{ return codec;			}
	bool  IsIntra()			{ return isIntra;		}
	DWORD GetWidth()		{ return width;			}
	DWORD GetHeight()		{ return height;		}

	void SetCodec(VideoCodec::Type codec)	{ this->codec = codec;		}
	void SetWidth(DWORD width)		{ this->width = width;		}
	void SetHeight(DWORD height)		{ this->height = height;	}
	void SetIntra(bool isIntra)		{ this->isIntra = isIntra;	}
	
	virtual bool Packetize(unsigned int mtu);

	/**
	 * Select how the NALU are to be parsed 
	 * @param sz: how many bytes used to store NALU sizein the bitstream. 0 = use start code
	 **/
	void SetH264NalSizeLength(DWORD sz)
	{
		if (sz == 0)
		{
			useStartCode = true;
			naluSizeLen = 0;
		}
		else if (sz <= 4)
		{
			useStartCode = false;
			naluSizeLen = sz;
		}
		else
		{
			useStartCode = false;
			naluSizeLen = 4;			
		}
	}

private:
	VideoCodec::Type codec;
	bool	isIntra;
	DWORD	width;
	DWORD	height;

	bool PacketizeH264(unsigned int mtu);
	bool PacketizeH263(unsigned int mtu);
	

	// H.264 specific
	bool useStartCode;
	bool naluSizeLen;
	
	// If NALU size is stored in data (MP4 file)
	DWORD ReadNaluSize(BYTE * data);
	
	//If butstream contains H.264 sync codes
	DWORD DetectNaluBoundary(BYTE * data);
	
	// Handle fragmentation
	void PacketizeH264Nalu(unsigned int mtu, DWORD offset, DWORD naluSz);
};



class VideoEncoder
{
public:
	virtual ~VideoEncoder(){};

	virtual int SetSize(int width,int height)=0;
	virtual VideoFrame* EncodeFrame(BYTE *in,DWORD len)=0;
	virtual int FastPictureUpdate()=0;
	virtual int SetFrameRate(int fps,int kbits,int intraPeriod)=0;
public:
	VideoCodec::Type type;
};

class VideoDecoder
{
public:
	virtual ~VideoDecoder(){};

	virtual int GetWidth()=0;
	virtual int GetHeight()=0;
	virtual int Decode(BYTE *in,DWORD len) = 0;
	virtual int DecodePacket(BYTE *in,DWORD len,int lost,int last)=0;
	virtual BYTE* GetFrame()=0;
	virtual bool  IsKeyFrame()=0;
public:
	VideoCodec::Type type;

};

class VideoCodecFactory
{
public:
	static VideoDecoder* CreateDecoder(VideoCodec::Type codec);
	static VideoEncoder* CreateEncoder(VideoCodec::Type codec);
	static VideoEncoder* CreateEncoder(VideoCodec::Type codec, const Properties &properties);
};
#endif
