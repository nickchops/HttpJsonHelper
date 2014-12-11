
//-----------------------------------------------------------------------------

#include "CHttpJsonHelper.h"
#include <string>
#include "s3eMemory.h"
#include "s3eMemory.h"

CHttpJsonHelper::CHttpJsonHelper()
{
	this->result = NULL;
	this->secureResult = NULL;
	this->status = kNone;
	this->theHttpObject = new CIwHTTP;
}

CHttpJsonHelper::~CHttpJsonHelper()
{
	if (this->theHttpObject)
	{
		this->Cancel();
		delete this->theHttpObject;
	}
	s3eFree(result);
	s3eFree(secureResult);
}

//NB: using a bit of a hack to cal member var from static callback.
// This means the users loses the userData variable.
// OK as we only have on eoperation going at a time per CHttpJsonHelper -
// not sure if one helper should be able to do many calls at once...
// Bette solution is to use boost!
int32 CHttpJsonHelper::GotDataCallback(void* data, void* userData)
{
	CHttpJsonHelper* self = static_cast<CHttpJsonHelper*>(userData);
    return self->GotData(data, NULL);
}

int32 CHttpJsonHelper::GotData(void* data, void* userData)
{
    // This is the callback indicating that a ReadContent call has
    // completed.  Either we've finished, or a bigger buffer is
    // needed.  If the correct ammount of data was supplied initially,
    // then this will only be called once. However, it may well be
    // called several times when using chunked encoding.

    // Firstly see if there's an error condition.
    if (this->theHttpObject->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
        this->status = kError;

		// inform app we failed to get all the data (data = null)
		this->resultCallback(NULL, NULL);
    }
    else if (theHttpObject->ContentReceived() != theHttpObject->ContentLength())
    {
        // We have some data but not all of it. We need more space.
        uint32 oldLen = len;
        // If iwhttp has a guess how big the next bit of data is (this
        // basically means chunked encoding is being used), allocate
        // that much space. Otherwise guess.
        if (len < theHttpObject->ContentExpected())
            len = theHttpObject->ContentExpected();
        else
            len += 1024;

        // Allocate some more space and fetch the data.
		s3eFree(this->result);
        this->result = (char*)s3eRealloc(this->result, len);

        this->theHttpObject->ReadContent(this->result+oldLen, len - oldLen, &CHttpJsonHelper::GotDataCallback, (void*)this);
    }
    else
    {
        // We've got all the data. Display it.
        this->status = kOK;

		// send complete data to app
		// TODO: convert result to json object format
		this->resultCallback((void*)result, NULL);
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Called when the response headers have been received
//-----------------------------------------------------------------------------
int32 CHttpJsonHelper::GotHeadersCallback(void* data, void* userData)
{
	CHttpJsonHelper* self = static_cast<CHttpJsonHelper*>(userData);
    return self->GotHeaders(data, NULL);
}

int32 CHttpJsonHelper::GotHeaders(void* data, void* userData)
{
    if (this->theHttpObject->GetStatus() == S3E_RESULT_ERROR)
    {
        // Something has gone wrong
        this->status = kError;

		// inform app that Get/Put succeeded
		this->resultCallback((void*)false, NULL);
    }
    else if (theHttpObject->GetType() == CIwHTTP::HEAD)
    {
        // this is a HEAD request, we have got the header so return
        //this->status = kOK; //TODO should only set this if we were waiting for header only

		// inform app that Get/Put succeeded
		this->resultCallback((void*)true, NULL);
    }
    else // got body from Get request
    {
        // Depending on how the server is communicating the content
        // length, we may actually know the length of the content, or
        // we may know the length of the first part of it, or we may
        // know nothing. ContentExpected always returns the smallest
        // possible size of the content, so allocate that much space
        // for now if it's non-zero. If it is of zero size, the server
        // has given no indication, so we need to guess. We'll guess at 1k.
        this->len = this->theHttpObject->ContentExpected();
        if (!this->len)
        {
            this->len = 1024;
        }

        s3eFree(this->result);
        this->result = (char*)s3eMalloc(len + 1);
        this->result[len] = 0;
        this->theHttpObject->ReadContent(this->result, this->len, &CHttpJsonHelper::GotDataCallback, this);
    }

    return 0;
}

void CHttpJsonHelper::Cancel()
{
	if (this->theHttpObject)
		this->theHttpObject->Cancel();
}

s3eResult CHttpJsonHelper::Get(const char* uri, s3eCallback gotResult, s3eCallback gotData, const char* inputData, int requestID) //TODO: inputData should not be string!
{
	if (this->status != kNone)
    {
		return S3E_RESULT_ERROR;
	}

	this->resultCallback = gotResult;
	this->dataCallback = gotData;

	// TODO: convert json object into string
	// ALso, inputData will be a single list/array. needs converting to strings and then appending to URI
	// until we know better, assume format is "uri?param0=foo&param1=bar"
	char *fullUri= (char*)s3eMalloc(strlen(uri) + strlen(inputData) + 1);
	strcpy(fullUri, uri);
	strcpy(fullUri+strlen(uri), inputData);

    if (this->theHttpObject->Head(fullUri, &CHttpJsonHelper::GotHeadersCallback, this) == S3E_RESULT_ERROR)
		return S3E_RESULT_ERROR;

	this->status = kInProgress;
	return S3E_RESULT_SUCCESS;
}

s3eResult CHttpJsonHelper::Post(const char* uri, s3eCallback gotResult, s3eCallback gotData, const char* inputData, int requestID) //TODO: jsonData should not be string!
{
	if (this->status != kNone)
    {
		return S3E_RESULT_ERROR;
	}

	this->resultCallback = gotResult;
	this->dataCallback = gotData;

	//Something like this for HTTPS (from IwHttpExample)...
//    char buf[256];
//    const char *passString = "tobe:ryloth";
//    char *base64;
//    base64 = base64_encode(passString, strlen(passString), NULL);
//    snprintf(buf, 256, "Basic %s", base64);
//    free(base64);
//
//    theHttpObject->SetRequestHeader("Authorization", buf);
//    theHttpObject->SetRequestHeader("Cache-Control", "max-age=0");
//    theHttpObject->SetRequestHeader("Accept", "application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5");
//    theHttpObject->SetRequestHeader("Accept-Encoding", "gzip,deflate,sdch");
//    theHttpObject->SetRequestHeader("Accept-Language", "en-GB");
//    theHttpObject->SetRequestHeader("Accept-Charset", "ISO-8859-1,utf-8;q=0.7,*;q=0.3");


    if (this->theHttpObject->Post(uri, inputData, strlen(inputData), &CHttpJsonHelper::GotHeadersCallback, this) == S3E_RESULT_ERROR)
		return S3E_RESULT_ERROR;

	this->status = kInProgress;
	return S3E_RESULT_SUCCESS;
}

