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
#include "RPCClient.h"
#include "RPCLog.h"
#include "MP_ThreadFactory.h"
#include "MP_SocketFactory.h"
using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
RPCClient::RPCClient(IN std::string nodeName, IN std::string serverIP, IN unsigned long serverPort, IN MPBaseLibraryType platform)
	: RPCBase(platform)
{
    m_serverIP = serverIP;
	m_serverPort = serverPort;
	m_receiveSocket = NULL;
	m_pConnectThread = NULL;
	m_pReceiveThread = NULL;
	m_isReceiving = false;
	m_isConnecting = false;
}

RPCClient::~RPCClient()
{
    // Nothing is needed here 
}

bool RPCClient::start()
{
	init("RPCClient_Mutex");

	if (!restartConnectServerThread())
	{
		LOG_ERROR("RPCClient::start: Fail to call restartConnectServerThread \n");
		return false;
	}
	
	LOG_INFO("RPCClient::start: Finish to call start \n");
	return true;
}

bool RPCClient::stop()
{
	stopConnectServerThread();
	stopReceiveDataThread();

	lock();
    if (m_receiveSocket != NULL)
    {
	    delete m_receiveSocket;
		m_receiveSocket = NULL;
    }
	unlock();
	
	terminate();

	LOG_INFO("RPCClient::stop: Finish to call stop \n");
	return true;
}

bool RPCClient::send(IN char* pBuffer, IN unsigned int bufferLen)
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
        LOG_ERROR("RPCClient::send: error occurs when send\n");
		return false;
    }
	LOG_DEBUG("RPCClient::send: succes to send, sent size = %d\n", result);

	return true;
}

bool RPCClient::restartConnectServerThread()
{
	// If there are already connect thread, stop and close it first 
	stopConnectServerThread();

	// Start the connect thread
	lock();
	m_isConnecting = true;
	m_pConnectThread = MP_ThreadFactory::createThread(m_platform);
	if (m_pConnectThread == NULL)
	{
	    unlock();
		LOG_ERROR("RPCClient::restartConnectServerThread: fail to create connecting thread \n");
		return false;
	}
	if (!m_pConnectThread->start(run_connectServer, this, NULL))
	{
	    unlock();
		LOG_ERROR("RPCClient::restartConnectServerThread: fail to start server connecting thread \n");
		return false;
	}
	unlock();

    LOG_INFO("RPCClient::restartConnectServerThread: Success to start server connecting thread \n");
	return true;
}

void RPCClient::stopConnectServerThread()
{
    lock();
    m_isConnecting = false; 
	if (m_pConnectThread != NULL)
	{
	    m_pConnectThread->stop();
		delete m_pConnectThread;
		m_pConnectThread = NULL;
		LOG_INFO("RPCClient::stopConnectServerThread: Finish to stop server connecting thread \n");
	}
	unlock();
}

bool RPCClient::restartReceiveDataThread()
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
		LOG_ERROR("RPCClient::restartReceiveDataThread: Fail to start data receiving thread !!!\n");
		return false;
	}
	unlock();
	
	LOG_INFO("RPCClient::restartReceiveDataThread: Success to start data receiving thread!!!\n");
	return true;
}

void RPCClient::stopReceiveDataThread()
{
    lock();
    m_isReceiving = false;
	if (m_pReceiveThread != NULL)
	{
		m_pReceiveThread->stop();
		delete m_pReceiveThread;
		m_pReceiveThread = NULL;
		LOG_INFO("RPCClient::stopReceiveDataThread: Finish to stop data receiving thread \n");
	}
	unlock();
}

int RPCClient::run_connectServer(IN void *pUser1, IN void *pUser2)
{
	RPCClient *pThis = (RPCClient*)pUser1;
	if (pThis == NULL)
	{
		LOG_ERROR("RPCClient::run_connectServer: (pThis == NULL) \n");
		return 0;
	}

	// Create client socket 
	pThis->lock();
	pThis->m_receiveSocket = MP_SocketFactory::createTCPSocket(pThis->m_platform);
	if (pThis->m_receiveSocket == NULL)
	{
	    pThis->unlock();
	    LOG_ERROR("RPCClient::run_connectServer: Fail to create m_receiveSocket!!! \n");
	    return 0;
	}
	pThis->unlock();
	LOG_ERROR("RPCClient::run_connectServer: Success to create m_receiveSocket!!! \n");
	
	while (pThis->m_isConnecting)
	{
		pThis->lock();
		bool isConnected = pThis->m_receiveSocket->connect(pThis->m_serverIP.c_str(), pThis->m_serverPort);
		pThis->unlock();
		
		if (isConnected)
		{
		    
		    pThis->restartReceiveDataThread();
			LOG_INFO("RPCClient::run_connectServer: Success to connect to server!!! \n");

			pThis->notifyNewStatus(true);
			break;
		}

        LOG_INFO("RPCClient::run_connectServer: Fail to connect to server!!! \n");
		MP_Thread::sleep(500); 
	}

	return 0;
}

int RPCClient::run_receiveMessage(IN void *pUser1, IN void *pUser2)
{
	RPCClient *pThis = (RPCClient*)pUser1;
	if (pThis == NULL)
	{
		LOG_ERROR("RPCClient::run_ReceiveMessage: (pThis == NULL) \n");
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
				LOG_ERROR("RPCClient::run_receiveMessage: diconnect with server when receive a data!!! \n");
			}
		}
		else
		{
			receiveFailCount = 0;  // Once a data received success, we reset this fail count data
			LOG_INFO("RPCClient::run_receiveMessage: success to receive a data, size = %d\n", recvLen);
		}
		
		// We will check that if the receive fail count alread achieve MAX number
		// If Yes, we need to stop this run_receiveMessage Thread and restart the run_connectServer thread 
		if (receiveFailCount > MAX_RECEIVE_FAIL_COUNT)
		{
			LOG_INFO("RPCClient::run_receiveMessage: m_receiveFailCount goes to the MAX, need to restart server connecting thread!!! \n");
			pThis->lock();
			delete pThis->m_receiveSocket;
			pThis->m_receiveSocket = NULL;
			pThis->unlock();

			pThis->notifyNewStatus(false);
			pThis->restartConnectServerThread();  // Restart the server connection thread 
			LOG_INFO("RPCClient::run_receiveMessage: Finish to start server conecting thread !!! \n");
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
