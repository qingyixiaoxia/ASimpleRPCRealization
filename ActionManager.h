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
#ifndef REMOTE_ACTION_RPC_ACTION_MANAGER_H
#define REMOTE_ACTION_RPC_ACTION_MANAGER_H
#include "RPCBase.h"
#include <string>
#include <vector>
#include "ActionListener.h"
#include "ActionBuffer.h"
#include "MPCommon.h"

namespace RemoteActionRPC
{
class ActionManager : public RPCDataListener, public RPCStatusListener
{
public:
	ActionManager(IN MPBaseLibraryType platform);
	~ActionManager();
	bool start(IN bool isAsServer, IN std::string nodeName, IN std::string IP, IN unsigned long PORT, IN unsigned int maxRecvLen);
	void stop();
	bool send(IN ActionType type, IN const std::string &xmlBodyMsg);

	void registerEventListener(IN StatusListener *pStatusListener, IN EventListener *pDataListener);

private:
	void onPeerConnectStatus(IN bool isConnected);
	bool onCheckLeftBufSize(IN unsigned int neededBufferLen);
	void onReceiveRPCData(IN char* pBuffer, IN unsigned int bufferLen);
	void NotifyNewStatus(bool isConnected);
	void NotifyNewData(IN const std::string &peerName, IN ActionType type, IN const std::string &xmlBodyMsg);

private:
	RPCBase* m_pRPCActionHandler;
	std::vector<StatusListener*> m_statusListeners;
	std::vector<EventListener*> m_eventListeners;
	ActionBuffer m_actionBuffer;
	MPBaseLibraryType m_platform;
	std::string m_nodeName;
};
}
#endif