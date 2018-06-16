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
#include "ActionBuffer.h"
#include "RPCLog.h"
#include "MP_MutexFactory.h"

using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
ActionBuffer::ActionBuffer(IN MPBaseLibraryType platform)
{
    m_platform = platform;
    m_nLength = 0;
	m_nStartPos = 0;
	m_pMutex = MP_MutexFactory::createMutex(platform, "RemoteActionBuffer");
}

ActionBuffer::~ActionBuffer()
{
    m_nLength = 0;
	if (m_pMutex != NULL)
	{
	    delete m_pMutex;
	}
}

bool ActionBuffer::isFull()
{
	m_pMutex->lock();
    if (m_nLength >= MAX_ACTION_BUFFER_LENGTH)
    {
        m_pMutex->unlock();
        return true;
    }
	m_pMutex->unlock();

	return false;
}

bool ActionBuffer::clear()
{
    m_pMutex->lock();
	memset(m_buffer, 0x00, sizeof(m_buffer));
	m_nStartPos = 0;
	m_nLength = 0;
	m_pMutex->unlock();

	return true;
}

unsigned int ActionBuffer::getLeftLen()
{
    int result = 0;
	
    m_pMutex->lock();
	result = MAX_ACTION_BUFFER_LENGTH - m_nLength;
	m_pMutex->unlock();
	
    return result;
}

unsigned int ActionBuffer::getDataLen()
{
    int result = 0;
	
    m_pMutex->lock();
	result = m_nLength;
	m_pMutex->unlock();
	
    return result;
}

const char* ActionBuffer::getData()
{
    const char* result = NULL;
	
    m_pMutex->lock();
	result = m_buffer + m_nStartPos;
	m_pMutex->unlock();
	
    return result;
}

bool ActionBuffer::save(IN const char* pData, IN unsigned int hopeGiveLen)
{
    if (pData == NULL || hopeGiveLen <= 0)
    {
        return false;
    }

	m_pMutex->lock();
    if ((m_nLength + hopeGiveLen) > MAX_ACTION_BUFFER_LENGTH)
    {
        m_pMutex->unlock();
        return false;
    }

    if ((m_nStartPos + m_nLength + hopeGiveLen) > MAX_ACTION_BUFFER_LENGTH)
    {
        reconstruct();
    }

	memcpy((m_buffer + m_nStartPos) + m_nLength, pData, hopeGiveLen);
	m_nLength += hopeGiveLen;
	
	m_pMutex->unlock();
	return true;
}

bool ActionBuffer::take(IN unsigned int hopeTakeLen, OUT char* pData)
{
    m_pMutex->lock();
	if (m_nLength < hopeTakeLen)
	{
	    m_pMutex->unlock();
	    return false;
	}

    // Save the data
    if (pData != NULL && hopeTakeLen > 0)
    {
        memcpy(pData, (m_buffer + m_nStartPos), hopeTakeLen);
    }

    // Resize the m_buffer 
    m_nLength -= hopeTakeLen;
	m_nStartPos += hopeTakeLen;
	
    m_pMutex->unlock();
	return true;
}

void ActionBuffer::reconstruct()
{
    if (m_nLength == 0)
    {
        memset(m_buffer, 0x00, sizeof(m_buffer));
		m_nStartPos = 0;
		m_nLength = 0;
		return;
    }
	
	char temp[MAX_ACTION_BUFFER_LENGTH] = {0};
	memcpy(temp, (m_buffer + m_nStartPos), m_nLength);
	memcpy(m_buffer, temp, m_nLength);
	m_nStartPos = 0;
}

}
