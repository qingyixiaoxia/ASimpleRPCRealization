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
#ifndef REMOTE_ACTION_RPC_ACTION_BUFFER_H
#define REMOTE_ACTION_RPC_ACTION_BUFFER_H
#include <string>
#include <vector>
#include "MP_Mutex.h"
using namespace MultiPlatformWrapper;

namespace RemoteActionRPC
{
#define MAX_ACTION_BUFFER_LENGTH   65535

class ActionBuffer  
{
public:
	ActionBuffer(IN MPBaseLibraryType platform);
	~ActionBuffer();

	bool isFull();
	bool clear();
	unsigned int getLeftLen();
	unsigned int getDataLen();
	const char* getData();
	
    bool save(IN const char* pData, IN unsigned int hopeGiveLen);
	bool take(IN unsigned int hopeTakeLen, OUT char* pData);

private:
	void reconstruct();
	
private:
	char m_buffer[MAX_ACTION_BUFFER_LENGTH];
	unsigned int m_nLength;
	unsigned int m_nStartPos;
	
	MP_Mutex* m_pMutex;
	
	MPBaseLibraryType m_platform;
};
}
#endif