#ifndef REMOTE_PLAYER_H
#define REMOTE_PLAYER_H

#include <iostream>

#include <opencv2/opencv.hpp>
#include "include/serializer.h"

#ifdef __cplusplus
	extern "C" {
#endif
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavdevice/avdevice.h>
	#include <libswscale/swscale.h>
#ifdef __cplusplus
	}
#endif

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
	#define av_frame_alloc avcodec_alloc_frame
	#define av_frame_free avcodec_free_frame
#endif

class FFMPEGPlayer;
class FFMPEGPlayerContext
{
	friend class FFMPEGPlayer;
public:
	enum class InputSource : uint8_t {
		IS_UNDEFINED, 
		IS_FILE, 
		IS_REMOTE, 
		IS_CAM0, 
		IS_CAM1, 
		IS_CAM2, 
		IS_CAM3, 
		IS_CAM4, 
		IS_CAM5, 
		IS_CAM6, 
		IS_CAM7, 
		IS_CAM8, 
		IS_CAM9};
	enum class InputType : uint8_t {
		IT_UNDEFINED, 
		IT_FILE, 
		IT_REMOTE, 
		IT_CAM};
private:
	AVFormatContext* pFormatCtx = nullptr;
	AVInputFormat* pInputFormat = nullptr;
	AVDictionary* pDictionary = nullptr;
	int videoStream;
	AVCodecContext* pCodecCtxOrig = nullptr;
	AVCodecContext* pCodecCtx = nullptr;
	AVCodec* pCodec = nullptr;
	AVFrame* pFrame = nullptr;
	AVPacket packet;
	int frameFinished;
	struct SwsContext* swsCtx = nullptr;
	InputSource inputSource = InputSource::IS_UNDEFINED;
	InputType inputType = InputType::IT_UNDEFINED;
	std::string fileName = "";
	bool isSuccess = false;
private:
	FFMPEGPlayerContext(InputSource inputSource = InputSource::IS_UNDEFINED, const std::string* pFileName = nullptr, AVCodecContext* pRemoteCodecCtx = nullptr);
	~FFMPEGPlayerContext();
private:
	void freePacket();
	bool init(AVCodecContext* pRemoteCodecCtx = nullptr);
	bool isInitSuccess();
};

class FFMPEGPlayer /////////////////////////////////////////////////////////////////// FFMPEGPlayer ///////////////////////////////////////////////////////////////////
{
private:
	FFMPEGPlayerContext* context = nullptr;
public:
	FFMPEGPlayer();
	FFMPEGPlayer(FFMPEGPlayerContext::InputSource inputSource, const std::string* pFileName = nullptr, AVCodecContext* pRemoteCodecCtx = nullptr);
	~FFMPEGPlayer();
public:
	bool setupInputSource(
			FFMPEGPlayerContext::InputSource inputSource = FFMPEGPlayerContext::InputSource::IS_CAM0, 
			const std::string* pFileName = nullptr, 
			AVCodecContext* pRemoteCodecCtx = nullptr);
	void freeContext();
	void freePacket();
	bool read();
	bool decode();
	void transform(); // TODO by transform context
	bool reflect(uint8_t* data);
	uint32_t widthFrame() const noexcept;
	uint32_t heightFrame() const noexcept;
	void dump();
	AVCodecContext* getCodecCtxOrig();
	void serializeCodecContext(void** dst, size_t* size); // TODO
	void deserializeCodecContext(const void* dst, size_t size); // TODO
	void serializePacket(void** dst, size_t* size); // TODO 
	void deserializePacket(const void* dst, size_t size); // TODO
	AVPacket* getPacket();
};

#endif // REMOTE_PLAYER_H

