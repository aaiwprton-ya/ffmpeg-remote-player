#include <opencv2/opencv.hpp>
#include "include/server.h"
#include "include/argparser.h"

void terminate (int signum)
{}

// server side protocol algorithm
// 0. send hello
// 1. accept client request, choose sending data, check it planned size and send to client
// 2. accept client confirm, send data
// 4. accept next request...

cv::VideoCapture cap("../video/bandicam__2022_12_03__02_11_41_829.mp4");
cv::Mat frame;

int main(int argc, char** argv)
{
	signal(SIGINT, terminate);
	
	Argparser parser(argc, argv);
	parser.findArg(ArgType::ARGTYPE_STRING, "addr", "a");
	parser.findArg(ArgType::ARGTYPE_INT, "port", "p");
	
	std::string* p_addr = parser.getArg("addr", &p_addr);
	int* p_port = parser.getArg("port", &p_port);

	Server server(*p_addr, *p_port, AF_INET);

	// test data
	struct SupplySources
	{
		cv::VideoCapture* cap = nullptr;
		cv::Mat* frame = nullptr;
	};
	SupplySources gSources;
	uint8_t p_gSource[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	gSources.cap = &cap;
	gSources.frame = &frame;
	std::vector<unsigned char> encodeBuff;
	DataGeneratorType frameReader = [](RecvPoolMutableEntityType targetBuffer, const void* anySources, size_t sourcesSize) -> RecvPoolMutableEntityType
	{
		((std::vector<uint32_t>*)targetBuffer)->clear();
		SupplySources* gSources = (SupplySources*)anySources;
		if (gSources->cap->isOpened())
		{
			gSources->cap->read(frame);
			cv::imencode(".jpg", frame, *((std::vector<unsigned char>*)targetBuffer));
		}
		return targetBuffer;
	};
	
	Processor::UnitTemplateType startUnitTemplate;
	startUnitTemplate.unitTemplate = Processor::UT_GET_TO_AGREEMENT_UNIVERSAL;
	startUnitTemplate.nextUnit = "accepted";
	startUnitTemplate.errorUnit = "error";
	startUnitTemplate.recvPoolGenerative["frame"] = RecvPoolGenerativeEntity(&encodeBuff, frameReader);
	startUnitTemplate.generativeSources = (void*)&gSources;
	startUnitTemplate.generativeSourcesSize = sizeof(gSources);
	server.processor.makeUnit("start", startUnitTemplate);
	
	Processor::UnitTemplateType acceptedUnitTemplate;
	acceptedUnitTemplate.unitTemplate = Processor::UT_READY_TO_SEND;
	acceptedUnitTemplate.nextUnit = "start";
	acceptedUnitTemplate.backUnit = "start";
	acceptedUnitTemplate.errorUnit = "error";
	server.processor.makeUnit("accepted", acceptedUnitTemplate);
		
	Processor::UnitTemplateType errorUnitTemplate;
	errorUnitTemplate.unitTemplate = Processor::UT_ERROR;
	errorUnitTemplate.backUnit = "start";
	server.processor.makeUnit("error", errorUnitTemplate);
	
	server.processor.setStartState("start");
	server.processor.setExitState("exit");
	server.processor.setErrorState("error");
	server.processor.setupStartState();
	
	// if the server is the initiator 
	server.setStartCmd({InetUtils::RT_CMD_GIVE_OK, ""});
	
	return server.start();
}
