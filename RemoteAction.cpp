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
#include "RemoteAction.h"
#include "ActionManager.h"

namespace RemoteActionRPC
{
RemoteAction::RemoteAction(IN MPBaseLibraryType platform)
{
    m_pActionManager = new ActionManager(platform);
}

RemoteAction::~RemoteAction()
{
    ActionManager* pActionMgr = (ActionManager*)m_pActionManager;
    if (pActionMgr != NULL)
    {
        delete pActionMgr;
    }
}

bool RemoteAction::start(IN bool isAsServer, IN std::string IP, IN unsigned long PORT, IN unsigned int maxRecvLen)
{
    ActionManager* pActionMgr = (ActionManager*)m_pActionManager;
    if (pActionMgr != NULL)
    {
        return pActionMgr->start(isAsServer, "NAME", IP, PORT, maxRecvLen);
    }
	return false;
}

void RemoteAction::stop()
{
    ActionManager* pActionMgr = (ActionManager*)m_pActionManager;
    if (pActionMgr != NULL)
    {
        pActionMgr->stop();
    }
}

bool RemoteAction::send(IN ActionType type, IN const std::string &xmlBodyMsg)
{
    ActionManager* pActionMgr = (ActionManager*)m_pActionManager;
    if (pActionMgr != NULL)
    {
        return pActionMgr->send(type, xmlBodyMsg);
    }
	return false;
}

void RemoteAction::registerEventListener(IN StatusListener *pStatusListener, IN EventListener *pDataListener)
{
    ActionManager* pActionMgr = (ActionManager*)m_pActionManager;
    if (pActionMgr != NULL)
    {
        pActionMgr->registerEventListener(pStatusListener, pDataListener);
    }
}
}
