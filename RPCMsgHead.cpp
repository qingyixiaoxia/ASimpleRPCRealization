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
#include "RPCMsgHead.h"

namespace RemoteActionRPC
{
const char* RPCMsgHead::findHeadPos(IN const char* pMsgBuf, IN unsigned int msgBufMaxLen)
{
    if (pMsgBuf == NULL)
    {
        return NULL;
    }

    if (msgBufMaxLen < MIN_HEAD_LENGHT)
    {
        return NULL;
    }
	
	const char *pCurPos = pMsgBuf;
    for (int i = 0; i < msgBufMaxLen - START_FLAG_NUM; i++)
    {
        bool isFind = true;
        for (int j = 0; j < START_FLAG_NUM; j++)
        {
            if (pCurPos[j] != START_FLAG)
            {
               isFind = false;
               break;
            }
        }

		if (isFind)
		{
		    return pCurPos;
		}
		pCurPos++;
    }

	return NULL;
}

bool RPCMsgHead::form(IN unsigned int actionType, IN const std::string nodeName, IN unsigned int bodyLen, 
	                        INOUT char* pHeadBuf, IN unsigned int headBufLen, OUT unsigned int &headLen)
{
    if (pHeadBuf == NULL)
    {
        return false;
    }


	char *pCurPos = pHeadBuf;
	
	// Append start flag
	for (int i = 0; i < START_FLAG_NUM; i++)
	{
	    *(pCurPos) = START_FLAG;
		pCurPos++;
	}

	// Append action type
    *((unsigned int*)(pCurPos)) = transToNetByteOrder_i(actionType);
	pCurPos += sizeof(unsigned int);
	
	// Append body length 
	*((unsigned int*)(pCurPos)) = transToNetByteOrder_i(bodyLen);
	pCurPos += sizeof(unsigned int);

    // Append node name len
	*((unsigned int*)(pCurPos)) = transToNetByteOrder_i(nodeName.length());
	pCurPos += sizeof(unsigned int);
	
	// Append node name
	memcpy(pCurPos, nodeName.c_str(), nodeName.length());
	pCurPos += nodeName.length();

	headLen = pCurPos - pHeadBuf;
    return true;
}

bool RPCMsgHead::parse(IN const char *pHeadBuf,  IN unsigned int headBufLen, OUT unsigned int &headLen, 
	                OUT unsigned int &actionType, OUT std::string &nodeName, OUT unsigned int &bodyLen)
{
    if (pHeadBuf == NULL)
    {
        return false;
    }
	if (headBufLen < MIN_HEAD_LENGHT)
	{
	    return false;
	}
	
    const char *pCurPos = pHeadBuf;
	
    // Parse start flag
    for (int i = 0; i < START_FLAG_NUM; i++)
    {
        if (*pCurPos != START_FLAG)
        {
            return false;
        }
		pCurPos++;
    }
	
	// Parse action type
	actionType = transToHostByteOrder_i(*((unsigned int *)pCurPos));
	pCurPos += sizeof(unsigned int);
	
	// Parse bodyLen
	bodyLen = transToHostByteOrder_i(*((unsigned int *)pCurPos));
	pCurPos += sizeof(unsigned int);

	// Parse node name len 
	unsigned int nodeNameLen = transToHostByteOrder_i(*((unsigned int *)pCurPos));
	pCurPos += sizeof(unsigned int);


	char *pNodeName = new char[nodeNameLen + 1];
	memset(pNodeName, 0x00, sizeof(nodeNameLen + 1));
	memcpy(pNodeName, pCurPos, nodeNameLen);
	nodeName = pNodeName;

	headLen = MIN_HEAD_LENGHT + nodeNameLen;
    return true;
}

short RPCMsgHead::transToNetByteOrder_s(short data)
{
    return data;
}

int RPCMsgHead::transToNetByteOrder_i(int data)
{
    return data;
}

long RPCMsgHead::transToNetByteOrder_l(long data)
{
    return data;
}

short RPCMsgHead::transToHostByteOrder_s(short data)
{
    return data;
}

int RPCMsgHead::transToHostByteOrder_i(int data)
{
    return data;
}

long RPCMsgHead::transToHostByteOrder_l(long data)
{
    return data;
}
}