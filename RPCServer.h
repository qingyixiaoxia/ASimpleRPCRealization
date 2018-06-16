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
#ifndef REMOTE_ACTION_RPC_SERVER_H
#define REMOTE_ACTION_RPC_SERVER_H
#include "RPCBase.h"
#include "MP_Mutex.h"
#include "MP_Socket.h"
#include "MP_Thread.h"
using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
class RPCServer : public RPCBase
{
public:
	RPCServer(IN std::string nodeName, IN std::string clientIP, IN unsigned long serverPort, IN MPBaseLibraryType platform);
	~RPCServer();
	bool start();
	bool stop();
	bool send(IN char* pBuffer, IN unsigned int bufferLen);

private:
	bool restartListenClientThread();
	void stopListenClientThread();
	bool restartReceiveDataThread();
	void stopReceiveDataThread();

	static int run_listenClient(void *pUser1, void *pUser2);
	static int run_receiveMessage(void *pUser1, void *pUser2);

private:
	std::string m_nodeName;
	std::string m_serverIP;
	unsigned int m_serverPort;
	MP_TCPSocket* m_listenSocket;
	MP_TCPSocket* m_receiveSocket;
	
	MP_Thread* m_pListenThread;
	MP_Thread* m_pReceiveThread;
	bool m_isListening;
	bool m_isReceiving;
};
}

#endif

