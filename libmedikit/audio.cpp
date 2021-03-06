#include "medkit/log.h"
#include "medkit/audio.h"
#include "g711/g711codec.h"

#ifdef G722_SUPPORT
#include "g722/g722codec.h"
#endif


AudioEncoder* AudioCodecFactory::CreateEncoder(AudioCodec::Type codec)
{
	//Empty properties
	Properties properties;

	//Create codec
	return CreateEncoder(codec,properties);
}

AudioEncoder* AudioCodecFactory::CreateEncoder(AudioCodec::Type codec, const Properties &properties)
{
	Log("-CreateAudioEncoder [%d,%s]\n",codec,AudioCodec::GetNameFor(codec));

	//Creamos uno dependiendo del tipo
	switch(codec)
	{
		case AudioCodec::PCMA:
			return new PCMAEncoder(properties);
		case AudioCodec::PCMU:
			return new PCMUEncoder(properties);
#ifdef G722_SUPPORT

		case AudioCodec::G722:
			return new G722Encoder(properties);
#endif
	
#if 0
		case AudioCodec::GSM:
			return new GSMEncoder(properties);
		case AudioCodec::SPEEX16:
			return new SpeexEncoder(properties);
		case AudioCodec::NELLY8:
			return new NellyEncoder(properties);
		case AudioCodec::NELLY11:
			return new NellyEncoder11Khz(properties);
#ifdef OPUS_SUPPORT
		case AudioCodec::OPUS:
			return new OpusEncoder(properties);
#endif
#ifdef AAC_SUPPORT
		case AudioCodec::AAC:
			return new AACEncoder(properties);
#endif
#endif
		default:
			Error("Codec not found [%d]\n",codec);
	}

	return NULL;
}

AudioDecoder* AudioCodecFactory::CreateDecoder(AudioCodec::Type codec)
{
	Log("-CreateAudioDecoder [%d,%s]\n",codec,AudioCodec::GetNameFor(codec));

	//Creamos uno dependiendo del tipo
	switch(codec)
	{
#if 0
		case AudioCodec::GSM:
			return new GSMDecoder();
		case AudioCodec::PCMA:
			return new PCMADecoder();
		case AudioCodec::PCMU:
			return new PCMUDecoder();
		case AudioCodec::SPEEX16:
			return new SpeexDecoder();
		case AudioCodec::NELLY8:
			return NULL;
		case AudioCodec::NELLY11:
			return new NellyDecoder11Khz();
#ifdef OPUS_SUPPORT
		case AudioCodec::OPUS:
			return new OpusDecoder();
#endif
		case AudioCodec::G722:
			return new G722Decoder();
#endif
		default:
			Error("Codec not found [%d]\n",codec);
	}

	return NULL;
}

bool AudioFrame::Packetize(unsigned int mtu)
{
	unsigned int paksize = packetization;
	if (paksize > mtu && mtu > 0) paksize = mtu;
	
	ClearRTPPacketizationInfo();
	for (unsigned int i=0; i<GetLength(); i+= paksize )
	{
		unsigned int rtplen = GetLength() - i;

		if (rtplen > paksize ) rtplen = paksize;
		AddRtpPacket(i, rtplen, 0, NULL, false);
	}
}
