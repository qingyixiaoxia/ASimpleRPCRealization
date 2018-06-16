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
#include "ActionManager.h"
#include "RPCClient.h"
#include "RPCServer.h"
#include "RPCMessage.h"
#include "RPCMsgHead.h"
#include "RPCLog.h"

namespace RemoteActionRPC
{
ActionManager::ActionManager(IN MPBaseLibraryType platform) : m_actionBuffer(platform)
{
	m_platform = platform;
    m_pRPCActionHandler = NULL;
}

ActionManager::~ActionManager()
{
    if (m_pRPCActionHandler != NULL)
    {
        delete m_pRPCActionHandler;
    }
}

bool ActionManager::start(IN bool isAsServer, IN std::string nodeName, IN std::string IP, IN unsigned long PORT, IN unsigned int maxRecvLen)
{
    m_nodeName = nodeName;
    if (isAsServer)
    {
        m_pRPCActionHandler = new RPCServer(nodeName, IP, PORT, m_platform);
    }
	else
	{
	    m_pRPCActionHandler = new RPCClient(nodeName, IP, PORT, m_platform);
	}

	m_pRPCActionHandler->setRecvLength(maxRecvLen);
	m_pRPCActionHandler->registerListener(this, this);
	m_pRPCActionHandler->start();

	return true;
}

void ActionManager::stop()
{
    if (m_pRPCActionHandler == NULL)
    {
        LOG_ERROR("ActionManager::stop: m_pRPCActionHandler == NULL");
		return;
    }
	
    m_pRPCActionHandler->stop();
}

bool ActionManager::send(IN ActionType type, IN const std::string &xmlBodyMsg)
{
    if (m_pRPCActionHandler == NULL)
    {
        LOG_ERROR("ActionManager::send: m_pRPCActionHandler == NULL");
		return false;
    }

    int sentBufferMaxLen = xmlBodyMsg.length() + MAX_HEAD_LENGHT + 1;
    char* sentBuffer = new char[sentBufferMaxLen];  
	memset(sentBuffer, 0x00, sentBufferMaxLen);
	unsigned int sentBufferLen = 0;
	
	/* Form the sent buffer */
	RPCMessage message;
	if (!message.form((unsigned int)type, m_nodeName, xmlBodyMsg.c_str(), xmlBodyMsg.length(), sentBuffer, sentBufferMaxLen, sentBufferLen))
	{
	    delete sentBuffer;
	    return false;
	}
	
	/* Send out the message */
    m_pRPCActionHandler->send(sentBuffer, sentBufferLen);
	
	delete sentBuffer;
	return true;
}

void ActionManager::onPeerConnectStatus(IN bool isConnected)
{
    NotifyNewStatus(isConnected);
}

bool ActionManager::onCheckLeftBufSize(IN unsigned int neededBufferLen)
{
    if (neededBufferLen > m_actionBuffer.getLeftLen())
    {
        return false;
    }

	return true;
}

void ActionManager::onReceiveRPCData(IN char* pBuffer, IN unsigned int bufferLen)
{
    if (pBuffer == NULL || bufferLen <= 0)
    {
        return;
    }
	
    /* Try to Save the new buffer */
    bool isSuccessSave = m_actionBuffer.save(pBuffer, bufferLen);
	bool isSuccessErase = false;
    if (!isSuccessSave)
    {
       LOG_ERROR("ActionManager::onReceiveRPCData: fail to save the buffer");
    }

    /* Parse the buffer */
	char* pBodyBuf = NULL;
    do 
    {
		/* Parse the head */
		RPCMessage message;
		unsigned int actionType = 0;
		unsigned int bodyBufLen = 0;
		unsigned int fullMsgLen = 0;
		std::string nodeName = "";
		unsigned int dataBufLen = m_actionBuffer.getDataLen();
		const char *pDataBuf = m_actionBuffer.getData();
		if (dataBufLen <= 0)
		{
			break;
		}

		const char *pStartPos = message.parseHead(pDataBuf, dataBufLen, actionType, nodeName, bodyBufLen, fullMsgLen);
		if (pStartPos == NULL)  /* Need to wait more data */
		{
		    if (!isSuccessSave) /* If fail to pass head and fail to save new buf, that means we need to clear all buffer to recover from error*/
		    {
		        isSuccessErase = m_actionBuffer.clear();
		    }
		    break;
		}

		/* Check that if all data already arrived */
		unsigned int invalidCharsNum = pStartPos - pDataBuf;
		if (dataBufLen < (invalidCharsNum + fullMsgLen))  /* Need to wait more data */
		{
		    if (invalidCharsNum > 0)  /* Erase all invalid chars */
		    {
		        isSuccessErase = m_actionBuffer.take(invalidCharsNum, NULL);  
		    }
		    break;
		}

		/* Erase all invalid chars */
		if (invalidCharsNum > 0)
		{
		    isSuccessErase = m_actionBuffer.take(invalidCharsNum, NULL);  
		}

		/* Parse the body */
		if (bodyBufLen > 0)
		{
			pBodyBuf = new char[bodyBufLen + 1];  
			memset(pBodyBuf, 0x00, bodyBufLen + 1);
			dataBufLen = m_actionBuffer.getDataLen();
			pDataBuf = m_actionBuffer.getData();
			if (!message.parseBody(pDataBuf, dataBufLen, pBodyBuf, bodyBufLen)) 
			{
				LOG_ERROR("ActionManager::onReceiveRPCData: UNKNOWN ERROR ~~~~ ");
			}
		}
		
		/* Erase the message chars */
		bool isSuccess = m_actionBuffer.take(fullMsgLen, NULL);
		isSuccessErase = isSuccessErase ? isSuccessErase : isSuccess;
		
		/* Notify to all listeners */
		NotifyNewData(nodeName, (ActionType)actionType, pBodyBuf != NULL ? pBodyBuf : "");
    } while (true);

    /* If fail to save the buffer, save it again after success to take some */
    if (!isSuccessSave && isSuccessErase)
    {
       m_actionBuffer.save(pBuffer, bufferLen);
    }

    /* Terminate the buffer */
	if (pBodyBuf != NULL)
	{
	    delete pBodyBuf;
	}
}

void ActionManager::registerEventListener(IN StatusListener *pStatusListener, IN EventListener *pDataListener)
{
    if (pStatusListener == NULL || pDataListener == NULL)
    {
        return;
    }

    bool isFind = false;
	for (int i = 0; i < m_statusListeners.size(); i++)
	{
	    if (m_statusListeners.at(i) == pStatusListener)
	    {
	        return;
	    }
	}
    if (!isFind)
    {
        m_statusListeners.push_back(pStatusListener);
    }

    isFind = false;
    for (int i = 0; i < m_eventListeners.size(); i++)
    {
    	if (m_eventListeners.at(i) == pDataListener)
    	{
    		return;
    	}
    }
    if (!isFind)
    {
    	m_eventListeners.push_back(pDataListener);
    }

}

void ActionManager::NotifyNewStatus(bool isConnected)
{
	for (int i = 0; i < m_statusListeners.size(); i++)
	{
	    StatusListener* pCurListener = m_statusListeners.at(i);
		pCurListener->onPeerConnectStatus(isConnected);
	}
}

void ActionManager::NotifyNewData(IN const std::string &peerName, IN ActionType type, IN const std::string &xmlBodyMsg)
{
	for (int i = 0; i < m_eventListeners.size(); i++)
	{
	    EventListener* pCurListener = m_eventListeners.at(i);
		pCurListener->onReceiveNewEvent(type, xmlBodyMsg);
	}
}

}
