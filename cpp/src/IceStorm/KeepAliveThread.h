// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef KEEP_ALIVE_THREAD_I_H
#define KEEP_ALIVE_THREAD_I_H

#include <IceStorm/LinkRecord.h>
#include <IceUtil/Thread.h>
#include <IceUtil/Monitor.h>
#include <list>

namespace IceStorm
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class KeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    KeepAliveThread(const Ice::CommunicatorPtr&, const TraceLevelsPtr&, const IceUtil::Time&);
    ~KeepAliveThread();

    void add(const IceStorm::TopicUpstreamLinkPrx&);
    void remove(const IceStorm::TopicUpstreamLinkPrx&);
    void destroy();
    bool filter(IceStorm::TopicUpstreamLinkPrxSeq&);
    virtual void run();

private:

    void failed(const IceStorm::TopicUpstreamLinkPrx&);

    const Ice::CommunicatorPtr _communicator;
    const TraceLevelsPtr _traceLevels;
    const IceUtil::Time _timeout;
    bool _destroy;
    std::list<IceStorm::TopicUpstreamLinkPrx> _upstream;
    std::list<IceStorm::TopicUpstreamLinkPrx> _failed;
};
typedef IceUtil::Handle<KeepAliveThread> KeepAliveThreadPtr;

}

#endif
