/* Copyright Wenjing Li. and other Node contributors. All rights reserved.
* Author: Wenjing Li, GitHub Address: http://www.github.com/qingyixiaoxia
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
#include "RPCMsgBody.h"
#include "RPCMsgHead.h"
#include "RPCMessage.h"
#include<cstring>
namespace RemoteActionRPC
{
bool RPCMessage::form(IN unsigned int actionType, IN const std::string nodeName, IN const char *pBodyBuf, IN unsigned int bodyBufLen, 
	               INOUT char *pFullMsgBuf, IN unsigned int fullMsgBufLen, OUT unsigned int &fullMsgLen)
{
    if (pFullMsgBuf == NULL || pBodyBuf == NULL)
    {
        return false;
    }

	// Form head buffer 
    RPCMsgHead head;
	char headBuf[MAX_HEAD_LENGHT] = {0};
	unsigned int headLen = 0;
	head.form(actionType, nodeName, bodyBufLen, headBuf, MAX_HEAD_LENGHT, headLen);

    // Check if there is enough memory 
	if (fullMsgBufLen < (headLen + bodyBufLen))
	{
	    return false;
	}
	
    // Append head and body 
	memcpy(pFullMsgBuf, headBuf, headLen);
	if (bodyBufLen > 0)
	{
	    memcpy(pFullMsgBuf + headLen, pBodyBuf, bodyBufLen);
	}
	fullMsgLen = headLen + bodyBufLen;

	return true;
}

const char* RPCMessage::parseHead(IN const char *pFullMsgBuf, IN unsigned int fullMsgBufMaxLen, 
	            OUT unsigned int &actionType, OUT std::string &nodeName, OUT unsigned int &bodyLen, OUT unsigned int &fullMsgLen)
{
    if (pFullMsgBuf == NULL)
    {
        return NULL;
    }
	
    RPCMsgHead head;
	const char* pHeadStartPos = head.findHeadPos(pFullMsgBuf, fullMsgBufMaxLen);
	if (pHeadStartPos == NULL)
	{
	    return NULL;
	}

    unsigned int uselessLen = pHeadStartPos - pFullMsgBuf;
	unsigned int headBufMaxLen = fullMsgBufMaxLen - uselessLen;
	unsigned int headLen = 0;
	if (!head.parse(pHeadStartPos, headBufMaxLen, headLen, actionType, nodeName, bodyLen))
	{
	    return NULL;
	}

	fullMsgLen = headLen + bodyLen;
	return pHeadStartPos;
}

bool RPCMessage::parseBody(IN const char *pValidMsgBuf, IN unsigned int validMsgBufMaxLen, 
	                      INOUT char *pBodyBuf, IN unsigned int bodyBufLen)
{
    if (pValidMsgBuf == NULL || pBodyBuf == NULL)
    {
        return false;
    }

    unsigned int actionType = 0;
	std::string nodeName;
	unsigned int bodyLen = 0;
	unsigned int headLen = 0;
	RPCMsgHead head;
	if (!head.parse(pValidMsgBuf, validMsgBufMaxLen, headLen, actionType, nodeName, bodyLen))
	{
	    return false;
	}

	if (bodyLen > bodyBufLen)
	{
	    return false;
	}
	
	if (bodyLen > 0)
	{
	    memcpy(pBodyBuf, pValidMsgBuf + headLen, bodyLen); 
	}

	return true;
}
}