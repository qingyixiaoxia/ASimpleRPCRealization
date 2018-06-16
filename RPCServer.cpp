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
#include "RPCServer.h"
#include "RPCLog.h"
#include "MP_ThreadFactory.h"
#include "MP_SocketFactory.h"
using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
RPCServer::RPCServer(IN std::string nodeName, IN std::string serverIP, IN unsigned long serverPort, IN MPBaseLibraryType platform)
	: RPCBase(platform)
{
    m_serverIP = serverIP;
	m_serverPort = serverPort;
	m_listenSocket = NULL;
	m_receiveSocket = NULL;
	m_pListenThread = NULL;
	m_pReceiveThread = NULL;
	m_isListening = false;
	m_isReceiving = false;
}

RPCServer::~RPCServer()
{
}

bool RPCServer::start()
{
	init("RPCServer_Mutex");

	if (!restartListenClientThread())
	{
		LOG_ERROR("RPCServer::start: fail to call restartListenClientThread \n");
		return false;
	}

    LOG_INFO("RPCServer::start: Finish to start \n");
	return true;
}

bool RPCServer::stop()
{
	stopListenClientThread();
	stopReceiveDataThread();
	
	lock();
	if (m_listenSocket != NULL)
	{
		delete m_listenSocket;
		m_listenSocket = NULL;
	}
	if (m_receiveSocket != NULL)
	{
		delete m_receiveSocket;
		m_receiveSocket = NULL;
	}
	unlock();
	
	terminate();

	LOG_INFO("RPCServer::stop: Finish to stop \n");
	return true;
}

bool RPCServer::send(IN char* pBuffer, IN unsigned int bufferLen)
{
    if (pBuffer == NULL || bufferLen <= 0)
    {
        return false;
    }

	lockSend();
	if (m_receiveSocket == NULL)
	{
	    unlockSend();
		return false;
	}

    int result = m_receiveSocket->send(pBuffer, bufferLen, SEND_FLAG_NOTHING);
	unlockSend();
    if (result != bufferLen)
    {
        LOG_ERROR("RPCServer::send: error occurs when send\n");
		return false;
    }
	LOG_DEBUG("RPCServer::send: succes to send, sent size = %d\n", result);

	return true;
}

bool RPCServer::restartListenClientThread()
{
	// If there are already connect thread, stop and close it first 
	stopListenClientThread();

	// Start the connect thread
	lock();
	m_isListening = true;
	m_pListenThread = MP_ThreadFactory::createThread(m_platform);
	if (m_pListenThread == NULL)
	{
	    unlock();
		LOG_ERROR("RPCClient::restartListenClientThread: fail to create listening thread \n");
		return false;
	}
	if (!m_pListenThread->start(run_listenClient, this, NULL))
	{
	    unlock();
		LOG_ERROR("RPCServer::restartListenClientThread: fail to start connection listeneing thread \n");
		return false;
	}
	unlock();

	LOG_INFO("RPCServer::restartListenClientThread: success to start connection listeneing thread \n");
	return true;
}

void RPCServer::stopListenClientThread()
{
    lock();
    m_isListening = false;  
	if (m_pListenThread != NULL)
	{
		m_pListenThread->stop();
		delete m_pListenThread;
		m_pListenThread = NULL;
		LOG_INFO("RPCServer::stopListenClientThread: Fnish to stop listening client thread \n");
	}
	unlock();
}

bool RPCServer::restartReceiveDataThread()
{
	// If there are already dta receiving thread, stop and close it first 
	stopReceiveDataThread();

	// Start the data receiving thread
	lock();
	m_isReceiving = true;
	m_pReceiveThread = MP_ThreadFactory::createThread(m_platform);
	if (m_pReceiveThread == NULL)
	{
	    unlock();
		LOG_ERROR("RPCClient::restartReceiveDataThread: fail to create receiving thread \n");
		return false;
	}
	if (!m_pReceiveThread->start(run_receiveMessage, this, NULL))
	{
	    unlock();
		LOG_ERROR("RPCServer::restartReceiveDataThread: Fail to start data receiving thread!!!\n");
		return false;
	}
	unlock();
	
	LOG_INFO("RPCServer::restartReceiveDataThread: success to start data receiving thread!!!\n");
	return true;
}

void RPCServer::stopReceiveDataThread()
{
    lock();
    m_isReceiving = false;
	if (m_pReceiveThread != NULL)
	{
		m_pReceiveThread->stop();
		delete m_pReceiveThread;
		m_pReceiveThread = NULL;
		LOG_INFO("RPCServer::stopReceiveDataThread: Fnish to stop data receiving thread \n");
	}
	unlock();
}

int RPCServer::run_listenClient(IN void *pUser1, IN void *pUser2)
{
	RPCServer *pThis = (RPCServer*)pUser1;
	if (pThis == NULL)
	{
		LOG_ERROR("RPCServer::run_listenClient: (pThis == NULL) \n");
		return 0;
	}

	// Create the listen socket 
	pThis->lock();
	pThis->m_listenSocket = MP_SocketFactory::createTCPSocket(pThis->m_platform);
	if (pThis->m_listenSocket == NULL)
	{
		LOG_ERROR("RPCServer::run_listenClient: Fail to create listen socket!!! \n");
		pThis->unlock();
        return 0;
	}
	LOG_INFO("RPCServer::run_listenClient: Success to create listen socket!!! \n");
	
    if (!pThis->m_listenSocket->bind(pThis->m_serverIP.c_str(), pThis->m_serverPort)) 
	{
        delete pThis->m_listenSocket;
		pThis->m_listenSocket = NULL;
		
		LOG_ERROR("RPCServer::run_listenClient: Fail to bind listen socket!!! \n");
		pThis->unlock();
        return 0;
    }
	LOG_INFO("RPCServer::run_listenClient: Success to bind listen socket!!! \n");
	
	// Start to listen 
	if (!pThis->m_listenSocket->listen())
	{
		LOG_ERROR("RPCServer::run_listenClient: Fail to listen connection !!!\n");
		pThis->unlock();
        return 0;
	}
	LOG_INFO("RPCServer::run_listenClient: Success to begin to listen !!! \n");
    pThis->unlock();
	
    // Loop to accept connection request 
	while (pThis->m_isListening)
	{
	    pThis->lock();
	    if (!pThis->m_listenSocket->select_r(0))
	    {
	        pThis->unlock();
			
	        MP_Thread::sleep(500);
			continue;
	    }
		
	    unsigned long remotePort = 0;
	    pThis->m_receiveSocket = pThis->m_listenSocket->accept(NULL, remotePort);
		if (pThis->m_receiveSocket == NULL)
		{
		    pThis->unlock();
			
			LOG_INFO("RPCServer::run_listenClient: error occurs when accepting a new connection!!! \n");
			MP_Thread::sleep(500);
			continue;
		}
		pThis->unlock();
		
		pThis->restartReceiveDataThread();
		LOG_INFO("RPCServer::run_listenClient: Success to connect to a new client!!!");

        pThis->notifyNewStatus(true);
		MP_Thread::sleep(500);
	}

	return 0;
}

int RPCServer::run_receiveMessage(IN void *pUser1, IN void *pUser2)
{
	RPCServer *pThis = (RPCServer*)pUser1;
	if (pThis == NULL)
	{
		LOG_ERROR("RPCServer::run_ReceiveMessage: (pThis == NULL) \n");
		return 0;
	}

    char *pRecvBuffer = new char[pThis->m_recvLength];
	int receiveFailCount = 0;
	while (pThis->m_isReceiving)
	{
		pThis->lockRecv();
		if (!pThis->checkCanHandle(pThis->m_recvLength))
		{
	        pThis->unlockRecv();
			
	        MP_Thread::sleep(50);
			continue;
		}
		
	    if (!pThis->m_receiveSocket->select_r(0))
	    {
	        pThis->unlockRecv();
			
	        MP_Thread::sleep(100);
			continue;
	    }

        int recvLen = 0;
		recvLen = pThis->m_receiveSocket->recv(pRecvBuffer, pThis->m_recvLength, RECV_FLAG_NOTHING);
		pThis->unlockRecv();
		if (recvLen <= 0)
		{
			if (recvLen == MP_SOCKET_ERROR_NET_DOWN || recvLen == MP_SOCKET_ERROR_PEER_DOWN)
			{
				receiveFailCount++;
				LOG_ERROR("RPCServer::run_receiveMessage: diconnect with client when receive a data!!! \n");
			}
		}
		else
		{
			receiveFailCount = 0;
			LOG_INFO("RPCServer::run_receiveMessage: success to receive a data, size = %d\n", recvLen);
		}
		
		if (receiveFailCount > MAX_RECEIVE_FAIL_COUNT)
		{
			LOG_INFO("RPCServer::run_receiveMessage: m_receiveFailCount goes to the MAX, need to stop data receiving thread!!! \n");

			pThis->lock();
			delete pThis->m_receiveSocket;
			pThis->m_receiveSocket = NULL;
			pThis->unlock();

			pThis->notifyNewStatus(false);
			//pThis->restartListenClientThread(); 
			LOG_INFO("RPCClient::run_receiveMessage: Finish to stop data receiving thread !!! \n");
			break;
		}

        if (recvLen > 0)  /* Notify should be out of lock <-> unlock */
        {
            pThis->notifyNewData(pRecvBuffer, recvLen);
        }
	}
    delete pRecvBuffer;
	
	return 0;
}

}

