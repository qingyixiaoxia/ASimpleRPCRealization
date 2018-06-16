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
#ifndef REMOTE_ACTION_RPC_BASE_H
#define REMOTE_ACTION_RPC_BASE_H
#include <vector>
#include <string>
#include "stdio.h"
#include "MP_Mutex.h"
using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
#define MAX_RECEIVE_FAIL_COUNT  5

class RPCDataListener
{
public:
	virtual bool onCheckLeftBufSize(IN unsigned int neededBufferLen) = 0;
	virtual void onReceiveRPCData(IN char* pBuffer, IN unsigned int bufferLen) = 0;
};

class RPCStatusListener
{
public:
	virtual void onPeerConnectStatus(IN bool isConnected) = 0;
};

class RPCBase
{
public:
	RPCBase(IN MPBaseLibraryType platform);
	virtual ~RPCBase() {}
	void init(IN std::string name);
	void terminate();
	
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool send(IN char* pBuffer, IN unsigned int bufferLen) = 0;
	
	void setRecvLength(IN unsigned int recvLength);
	bool registerListener(IN RPCStatusListener *pStatusListener, IN RPCDataListener *pDataListener);

	void lockSend();
	void unlockSend();
	void lockRecv();
	void unlockRecv();
	void lock();
	void unlock();

protected:
	bool checkCanHandle(IN unsigned int newBufSize);
    bool notifyNewData(IN char* pBuffer, IN unsigned int bufferLen);
	bool notifyNewStatus(IN bool isConnected);
	
protected:
	MP_Mutex* m_sendMutex;
	MP_Mutex* m_recvMutex;
	RPCStatusListener* m_rpcStatusListener;
	RPCDataListener* m_rpcDataListener;
	unsigned int m_recvLength;
	MPBaseLibraryType m_platform;
};
}

#endif

