#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs/legacy/constants_c.h>
#include "include/client.h"
#include "include/argparser.h"

// client side protocol algorithm
// 0. accept server hello
// 1. send request
// 2. get expected data size, confirm readiness to receive
// 4. accept data
// 5. process data, next iteration or closing

cv::Mat decodeFrame;

int main(int argc, char** argv)
{
	Argparser parser(argc, argv);
	parser.findArg(ArgType::ARGTYPE_STRING, "addr", "a");
	parser.findArg(ArgType::ARGTYPE_INT, "port", "p");
	
	std::string* p_addr = parser.getArg("addr", &p_addr);
	int* p_port = parser.getArg("port", &p_port);

	Client client;
	
	// type of requested data 
	typedef unsigned char RequestType;
	std::string dataName = "frame";
	
	Processor::UnitTemplateType startUnitTemplate;
	startUnitTemplate.unitTemplate = Processor::UT_STATE_TO_GET;
	startUnitTemplate.nextUnit = "accept";
	startUnitTemplate.backUnit = "start";
	startUnitTemplate.errorUnit = "error";
	startUnitTemplate.getValue = dataName;
	client.processor.makeUnit("start", startUnitTemplate);
	
	Processor::UnitTemplateType acceptUnitTemplate;
	acceptUnitTemplate.unitTemplate = Processor::UT_ACCEPT_TO_READY;
	acceptUnitTemplate.nextUnit = "receive";
	acceptUnitTemplate.errorUnit = "error";
	client.processor.makeUnit("accept", acceptUnitTemplate);
	
	Processor::UnitTemplateType receiveUnitTemplate;
	receiveUnitTemplate.unitTemplate = Processor::UT_RECEIVE_TO_GET;
	receiveUnitTemplate.nextUnit = "accept";
	receiveUnitTemplate.errorUnit = "error";
	receiveUnitTemplate.getValue = dataName;
	Processor::RecvBufferType<RequestType> recvBuffer;
	receiveUnitTemplate.recvBuffer = &recvBuffer;
	client.processor.makeUnit("receive", receiveUnitTemplate);
	
	Processor::UnitTemplateType errorUnitTemplate;
	errorUnitTemplate.unitTemplate = Processor::UT_ERROR;
	errorUnitTemplate.backUnit = "start";
	client.processor.makeUnit("error", errorUnitTemplate);
	
	client.processor.setStartState("start");
	client.processor.setExitState("exit");
	client.processor.setErrorState("error");
	client.processor.setupStartState();
	
	// if the client is the initiator 
	//client.setStartCmd("get --value arr1");
	
	int connection;
	if ((connection = client.connect(*p_addr, *p_port, AF_INET)) == -1)
	{
		return -1;
	}
	
	std::vector<RequestType> encodeBuff;
	short events = POLLIN;
	while ((events = client.step(events)) != 0)
	{
		if (recvBuffer.getSize() > 0)
		{
			encodeBuff.resize(recvBuffer.getSize());
			memcpy(encodeBuff.data(), recvBuffer.getBuffer(), recvBuffer.getSize());
			decodeFrame = cv::imdecode(encodeBuff, CV_LOAD_IMAGE_COLOR);
			cv::imshow("asdf", decodeFrame);
			if (cv::waitKey(2) == 27)
			{
				break;
			}
		}
	}
	
	return 0;
}
