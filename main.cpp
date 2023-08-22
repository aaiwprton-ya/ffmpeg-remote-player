#include <stdio.h>

#include <opencv2/opencv.hpp>
#include "remote_player.h"
	
int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "[ERROR] Please provide a movie file" << std::endl;
		return -1;
	}
	
	FFMPEGPlayer player1;
	std::string fileName(argv[1]);
	if (!player1.setupInputSource(FFMPEGPlayerContext::InputSource::IS_FILE, &fileName))
	{
		return -2;
	}
	player1.dump();
	
	FFMPEGPlayer player2;
	if (!player2.setupInputSource(FFMPEGPlayerContext::InputSource::IS_REMOTE, nullptr, player1.getCodecCtxOrig()))
	{
		return -3;
	}
	
	cv::Mat* cvFrame = nullptr;
	while(player1.read())
	{
		// how it is:
		av_packet_ref(player2.getPacket(), player1.getPacket()); // TODO how it work inside?
		
		// how it must to be:
		// 1) player1 on the server read AVPacket, serialize it and sending to the client 
		// 2) player2 on the client received the serialized data, deserialize it to AVPacket and decode it, and show
		
		player1.freePacket();
		if (!player2.decode())
		{
			continue;
		}
		player2.transform();
		if (cvFrame == nullptr)
		{
			cvFrame = new cv::Mat(player2.heightFrame(), player2.widthFrame(), CV_8UC3);
		}
		if (!player2.reflect(cvFrame->data))
		{
			break;
		}
		player2.freePacket();
		cv::imshow("asdf", *cvFrame);
        if (cv::waitKey(2) == 27)
        {
            break;
        }
	}
	if (cvFrame != nullptr)
	{
		delete cvFrame;
	}

	return 0;
}

