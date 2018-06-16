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
#ifndef REMOTE_ACTION_RPC_MESSAGE_H
#define REMOTE_ACTION_RPC_MESSAGE_H

namespace RemoteActionRPC
{
class RPCMessage
{
public:
	// Return forming result 
	bool form(IN unsigned int actionType, IN const std::string nodeName, IN const char *pBodyBuf, IN unsigned int bodyBufLen, 
	             INOUT char *pFullMsgBuf, IN unsigned int fullMsgBufLen, OUT unsigned int &fullMsgLen);  

	// Return actual start position of head 
	const char* parseHead(IN const char *pFullMsgBuf, IN unsigned int fullMsgBufMaxLen, 
	            OUT unsigned int &actionType, OUT std::string &nodeName, OUT unsigned int &bodyLen, OUT unsigned int &fullMsgLen);

	bool parseBody(IN const char *pValidMsgBuf, IN unsigned int validMsgBufMaxLen, INOUT char *pBodyBuf, IN unsigned int bodyBufLen);
};
}

#endif

