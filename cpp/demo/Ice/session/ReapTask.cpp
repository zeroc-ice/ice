// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <ReapTask.h>

using namespace std;
using namespace Demo;

ReapTask::ReapTask() :
    _timeout(IceUtil::Time::seconds(10))
{
}

void
ReapTask::runTimerTask()
{
    Lock sync(*this);

    list<SessionProxyPair>::iterator p = _sessions.begin();
    while(p != _sessions.end())
    {
        try
        {
            //
            // Session destruction may take time in a
            // real-world example. Therefore the current time
            // is computed for each iteration.
            //
            if((IceUtil::Time::now(IceUtil::Time::Monotonic) - p->session->timestamp()) > _timeout)
            {
                string name = p->proxy->getName();
                p->proxy->destroy();
                cout << "The session " << name << " has timed out." << endl;
                p = _sessions.erase(p);
            }
            else
            {
                ++p;
            }
        }
        catch(const Ice::ObjectNotExistException&)
        {
            p = _sessions.erase(p);
        }
    }
}

void
ReapTask::add(const SessionPrx& proxy, const SessionIPtr& session)
{
    Lock sync(*this);
    _sessions.push_back(SessionProxyPair(proxy, session));
}
