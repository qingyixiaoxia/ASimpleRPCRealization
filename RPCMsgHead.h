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
#ifndef REMOTE_ACTION_RPC_MSG_HEAD_H
#define REMOTE_ACTION_RPC_MSG_HEAD_H
#include "MPCommon.h"
#include <string>

namespace RemoteActionRPC
{
#define START_FLAG         '$'
#define START_FLAG_NUM      4

#define MAX_NODE_NAME_LEN   128
#define MAX_HEAD_LENGHT     (16 + MAX_NODE_NAME_LEN)
#define MIN_HEAD_LENGHT     16

/*
    struct of message
    1. head
        START_FLAG(4) (4 byte)
        actionType    (4 byte)
        bodyLength    (4 byte)
        nodeNamelen   (4 byte)
        nodeName      (nodeNamelen byte)
        
    2. body
       in xml formate
*/

class RPCMsgHead
{
public:
	const char* findHeadPos(IN const char* pMsgBuf, IN unsigned int msgBufMaxLen);
	bool form(IN unsigned int actionType, IN const std::string nodeName, IN unsigned int bodyLen, 
                 INOUT char* pHeadBuf, IN unsigned int headBufLen, OUT unsigned int &headLen);
	bool parse(IN const char *pHeadBuf,  IN unsigned int headBufLen, OUT unsigned int &headLen,
                  OUT unsigned int &actionType, OUT std::string &nodeName, OUT unsigned int &bodyLen);

private: 
	short transToNetByteOrder_s(short data);
	int transToNetByteOrder_i(int data);
	long transToNetByteOrder_l(long data);

private:
	short transToHostByteOrder_s(short data);
	int transToHostByteOrder_i(int data);
	long transToHostByteOrder_l(long data);

private:
	unsigned int m_actionType;
	unsigned int m_bodyLen;
};
}

#endif

