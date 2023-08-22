#include "remote_player.h"

FFMPEGPlayerContext::FFMPEGPlayerContext(InputSource inputSource, const std::string* pFileName, AVCodecContext* pRemoteCodecCtx)
	: inputSource(inputSource)
{
	switch (inputSource) {
	case InputSource::IS_UNDEFINED:
		inputType = InputType::IT_UNDEFINED;
		break;
	case InputSource::IS_FILE:
		inputType = InputType::IT_FILE;
		if (pFileName == nullptr) break;
		fileName = *pFileName;
		break;
	case InputSource::IS_REMOTE:
		inputType = InputType::IT_REMOTE;
		break;
	case InputSource::IS_CAM0:
		fileName = "/dev/video0";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM1:
		fileName = "/dev/video1";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM2:
		fileName = "/dev/video2";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM3:
		fileName = "/dev/video3";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM4:
		fileName = "/dev/video4";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM5:
		fileName = "/dev/video5";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM6:
		fileName = "/dev/video6";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM7:
		fileName = "/dev/video7";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM8:
		fileName = "/dev/video8";
		inputType = InputType::IT_CAM;
		break;
	case InputSource::IS_CAM9:
		fileName = "/dev/video9";
		inputType = InputType::IT_CAM;
		break;
	};
	init(pRemoteCodecCtx);
}

FFMPEGPlayerContext::~FFMPEGPlayerContext()
{
	av_free_packet(&packet);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avcodec_close(pCodecCtxOrig);
	avformat_close_input(&pFormatCtx);
}
	
void FFMPEGPlayerContext::freePacket()
{
	av_free_packet(&packet);
}

bool FFMPEGPlayerContext::init(AVCodecContext* pRemoteCodecCtx)
{
	isSuccess = false;
	if (inputType == InputType::IT_UNDEFINED)
	{
		return false;
	}
	if (inputType == InputType::IT_CAM)
	{
		avdevice_register_all();
		avcodec_register_all();
	}
	av_register_all();
	
	if (inputType == InputType::IT_FILE)
	{
		if (avformat_open_input(&pFormatCtx, fileName.c_str(), nullptr, nullptr) != 0)
		{
			std::cout << "[ERROR] Couldn't open video source: " << fileName.c_str() << std::endl;
			return false;
		}
	} else
	if (inputType == InputType::IT_CAM)
	{
		pInputFormat = av_find_input_format("v4l2");
		av_dict_set(&pDictionary, "framerate", "20", 0);
		if(avformat_open_input(&pFormatCtx, fileName.c_str(), pInputFormat, nullptr) != 0)
		{
			std::cout << "[ERROR] Couldn't open video source: " << fileName.c_str() << std::endl;
			return false;
		}
	} else
	if (inputType == InputType::IT_REMOTE)
	{
		if (pRemoteCodecCtx == nullptr)
		{
			std::cout << "[ERROR] Couldn't initialize remote player: " << std::endl;
			return false;
		}
		// Get a pointer to the codec context for the video stream
		pCodecCtxOrig = pRemoteCodecCtx;
	}
	
	if (inputType != InputType::IT_REMOTE)
	{
		// Retrieve stream information
		if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		{
			std::cout << "[ERROR] Couldn't find stream information" << std::endl;
			return false;
		}

		// Find the first video stream
		videoStream = -1;
		for (int i = 0; i < pFormatCtx->nb_streams; ++i)
		{
			if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
		  		videoStream = i;
		  		break;
			}
		}
		if (videoStream == -1)
		{
			std::cout << "[ERROR] Didn't find a video stream" << std::endl;
			return false;
		}
		// Get a pointer to the codec context for the video stream
		pCodecCtxOrig = pFormatCtx->streams[videoStream]->codec;
	}

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtxOrig->codec_id);
	if (pCodec == NULL)
	{
		std::cout << "[ERROR] Codec not found" << std::endl;
		return false;
	}
	// Copy context
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0)
	{
		std::cout << "[ERROR] Couldn't copy codec context" << std::endl;
		return false;
	}
	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		std::cout << "[ERROR] Couldn't open codec" << std::endl;
		return false;
	}
	
	// Allocate video frame
	pFrame = av_frame_alloc();
	
	isSuccess = true;
	return true;
}

bool FFMPEGPlayerContext::isInitSuccess()
{
	return isSuccess;
}

FFMPEGPlayer::FFMPEGPlayer()
{}

FFMPEGPlayer::FFMPEGPlayer(FFMPEGPlayerContext::InputSource inputSource, const std::string* pFileName, AVCodecContext* pRemoteCodecCtx)
{
	setupInputSource(inputSource, pFileName, pRemoteCodecCtx);
}

FFMPEGPlayer::~FFMPEGPlayer()
{
	freeContext();
}

bool FFMPEGPlayer::setupInputSource(
		FFMPEGPlayerContext::InputSource inputSource, 
		const std::string* pFileName, 
		AVCodecContext* pRemoteCodecCtx)
{
	freeContext();
	context = new FFMPEGPlayerContext(inputSource, pFileName, pRemoteCodecCtx);
	return context->isInitSuccess();
}

void FFMPEGPlayer::freeContext()
{
	if (context != nullptr)
	{
		delete context;
		context = nullptr;
	}
}

void FFMPEGPlayer::freePacket()
{
	if (context != nullptr)
	{
		context->freePacket();
	}
}

bool FFMPEGPlayer::read()
{
	return av_read_frame(context->pFormatCtx, &context->packet) >= 0;
}

bool FFMPEGPlayer::decode()
{
	if (context->packet.stream_index == context->videoStream)
    {
		avcodec_decode_video2(context->pCodecCtx, context->pFrame, &context->frameFinished, &context->packet);
		if (context->frameFinished)
		{
			return true;
		}
	}
	return false;
}

void FFMPEGPlayer::transform() // TODO by transform context
{
	return;
}

bool FFMPEGPlayer::reflect(uint8_t* data)
{
	AVFrame dst;
	int w = context->pFrame->width;
	int h = context->pFrame->height;
	dst.data[0] = data;
	avpicture_fill((AVPicture*)&dst, dst.data[0], AV_PIX_FMT_BGR24, w, h);
	AVPixelFormat srcPixFmt = (AVPixelFormat)context->pFrame->format;
	AVPixelFormat dstPixFmt = AV_PIX_FMT_BGR24;
	context->swsCtx = sws_getContext(w, h, srcPixFmt, w, h, dstPixFmt,
			SWS_FAST_BILINEAR, NULL, NULL, NULL);
	if(context->swsCtx == NULL)
	{
		fprintf(stderr, "[ERROR] Cannot initialize the conversion context!\n");
		return false;
	}
	sws_scale(context->swsCtx, context->pFrame->data, context->pFrame->linesize, 0, h,
			dst.data, dst.linesize);
	return true;
}

uint32_t FFMPEGPlayer::widthFrame() const noexcept
{
	return context->pFrame->width;
}

uint32_t FFMPEGPlayer::heightFrame() const noexcept
{
	return context->pFrame->height;
}

void FFMPEGPlayer::dump()
{
	if (context != nullptr && context->isSuccess)
	{
		if (context->pFormatCtx != nullptr)
		{
			av_dump_format(context->pFormatCtx, 0, context->fileName.c_str(), 0);
		} else
		{
			std::cerr << "[WARNING] No information about remote player" << std::endl;
		}
	} else
	{
		std::cerr << "[ERROR] Player was't initialized" << std::endl;
	}
}

AVCodecContext* FFMPEGPlayer::getCodecCtxOrig()
{
	return context->pCodecCtxOrig;
}

void FFMPEGPlayer::serializeCodecContext(void** dst, size_t* size)
{
	// TODO
}

void FFMPEGPlayer::deserializeCodecContext(const void* dst, size_t size)
{
	// TODO
}

void FFMPEGPlayer::serializePacket(void** dst, size_t* size)
{
	// TODO
}

void FFMPEGPlayer::deserializePacket(const void* dst, size_t size)
{
	// TODO
}

AVPacket* FFMPEGPlayer::getPacket()
{
	return &(context->packet);
}

