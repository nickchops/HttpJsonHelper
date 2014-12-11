
#ifndef HTTP_JSON_HELPER_H
#define HTTP_JSON_HELPER_H

#include "s3eTypes.h"
#include "s3eDebug.h"
#include "IwHTTP.h"

class CHttpJsonHelper
{
protected:
	enum HTTPStatus
	{
		kNone,
		kInProgress,
		kOK,
		kError,
	};

	CIwHTTP *theHttpObject;
	char* result;
	char* secureResult;
	uint32 len;
	HTTPStatus status;
	bool headResult;

	int32 GotData(void* data, void* userData);
	int32 GotHeaders(void* data, void* userData);
	s3eCallback resultCallback;
	s3eCallback dataCallback;

public:
	CHttpJsonHelper();
	~CHttpJsonHelper();

	static int32 GotDataCallback(void* data, void* userData);
	static int32 GotHeadersCallback(void* data, void* userData);

	//todo: these should take userData and maybe work internally via actual callback register/fire mechanism
	//Alternatively might not actually want to use "s3eCallback" and have something bespoke
	s3eResult Get(const char* URI, s3eCallback gotResult, s3eCallback gotData, const char* inputData, int requestID=-1);
	s3eResult Post(const char* uri, s3eCallback gotResult, s3eCallback gotData, const char* inputData, int requestID=-1); //TODO add param(s) for using HTTPS
	HTTPStatus GetStatus(){ return status; };
	void Cancel();
};

#endif //HTTP_JSON_HELPER_H