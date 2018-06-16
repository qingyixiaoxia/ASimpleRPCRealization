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
#include "RPCBase.h"
#include "RPCLog.h"
#include "MP_Socket.h"
#include "MP_MutexFactory.h"

using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
RPCBase::RPCBase(IN MPBaseLibraryType platform)
{
    m_sendMutex = NULL;
	m_recvMutex = NULL;
	m_platform = platform;
}
void RPCBase::init(IN std::string name)
{
    m_sendMutex = MP_MutexFactory::createMutex(m_platform, name.c_str());
	m_recvMutex = MP_MutexFactory::createMutex(m_platform, name.c_str());
	
	// Init socket enviroment 
	if (!MP_Socket::initEnviroment())
	{
		LOG_ERROR("RPCBase::init: Fail to init socket enviroment \n");
	}
	LOG_INFO("RPCBase::init: Finish to init socket enviroment \n");
}

void RPCBase::terminate()
{
    if (m_sendMutex != NULL)
    {
        delete m_sendMutex;
    }
    if (m_recvMutex != NULL)
    {
        delete m_recvMutex;
    }
		
	MP_Socket::terminateEnviroment();
	LOG_INFO("RPCBase::terminate: Finish to terminate socket enviroment \n");
}

void RPCBase::setRecvLength(IN unsigned int recvLength)
{
    m_recvLength = recvLength;
}

bool RPCBase::checkCanHandle(IN unsigned int newBufSize)
{
    return m_rpcDataListener->onCheckLeftBufSize(newBufSize);
}

bool RPCBase::notifyNewData(IN char* pBuffer, IN unsigned int bufferLen)
{
    if (pBuffer == NULL || bufferLen <= 0)
    {
        return false;
    }

	m_rpcDataListener->onReceiveRPCData(pBuffer,  bufferLen);
    return true;
}

bool RPCBase::notifyNewStatus(IN bool isConnected)
{
	m_rpcStatusListener->onPeerConnectStatus(isConnected);
    return true;
}

bool RPCBase::registerListener(IN RPCStatusListener *pStatusListener, IN RPCDataListener *pDataListener)
{
    if (pStatusListener == NULL || pDataListener == NULL)
    {
        return false;
    }

    m_rpcStatusListener = pStatusListener;
    m_rpcDataListener = pDataListener;
    return true;
}

void RPCBase::lock()
{
    lockSend();
	lockRecv();
}

void RPCBase::unlock()
{
    unlockRecv();
	unlockSend();
}

void RPCBase::lockSend()
{
    m_sendMutex->lock();
}

void RPCBase::unlockSend()
{
    m_sendMutex->unlock();
}

void RPCBase::lockRecv()
{
    m_recvMutex->lock();
}

void RPCBase::unlockRecv()
{
    m_recvMutex->unlock();
}

}
