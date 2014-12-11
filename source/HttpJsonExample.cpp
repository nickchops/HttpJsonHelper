
#include <string>
#include "s3eMemory.h"
#include "ExamplesMain.h"
#include "IwGx.h"
#include "IwGxPrint.h"
#include "CHttpJsonHelper.h"

static const char* HTTP_URI = "http://test.ideaworkslabs.com/example.txt";
//static const char* HTTPS_URI = "https://test.ideaworkslabs.com/example.txt";

CHttpJsonHelper* networkRequest; //todo: have vector for multiple requests at once

#define CasinoBetLimitsAllRequest 40943
#define CasinoGameLimitsResponse  40025


int32 GotResult(void* data, void* userData)
{
	//if error then reactivate buttons to try again
	//print status
	return 0;
}

int32 GotData(void* data, void* userData)
{
	//print error if happened
	//else print data and convert to JSON
	return 0;
}

//-----------------------------------------------------------------------------
void ExampleInit()
{
    IwGxInit();
	
	networkRequest = new CHttpJsonHelper();

	const char* inputData = "";

	networkRequest->Post(HTTP_URI, (s3eCallback)GotResult, (s3eCallback)GotData, inputData);
}

//-----------------------------------------------------------------------------
void ExampleShutDown()
{
	if(networkRequest)
		delete networkRequest;

    IwGxTerminate();
}

//-----------------------------------------------------------------------------
bool ExampleUpdate()
{
	if (!networkRequest)
		return false;

	//get json from file on button presses
	//....

    return true;
}

//-----------------------------------------------------------------------------
void ExampleRender()
{
    // Clear screen
    IwGxClear( IW_GX_COLOUR_BUFFER_F | IW_GX_DEPTH_BUFFER_F );

    // Swap buffers
    IwGxFlush();
    IwGxSwapBuffers();
}
