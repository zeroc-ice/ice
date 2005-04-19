// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef REAP_THREAD_H
#define REAP_THREAD_H

#include <IceUtil/Thread.h>
#include <IceUtil/StaticMutex.h>
#include <SessionI.h>

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;
class ReapThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    static ReapThreadPtr& instance();

    virtual ~ReapThread();

    virtual void run();

    // XXX Rename to destroy().
    //
    // I named it terminate because destroy() methods in Java result
    // in a deprecation warning. If you want to use destroy() in Java
    // then I would have to use a runnable which means the demo isn't
    // the same code. Given the difference is a method name, I didn't
    // think it was worth the cost of different code.
    //
    void terminate();

    // XXX: The alternative here is to make timestamp() a slice
    // method. However, this means that we're adding methods to the
    // slice interface which is only required for the reaping thread,
    // and it obviously couldn't return an IceUtil::Time but instead
    // some other representation...
    //
    void add(const ::Demo::SessionPrx&, const SessionIPtr&);

private:

    ReapThread();

    const IceUtil::Time _timeout;
    bool _terminated;
    std::map< ::Demo::SessionPrx, SessionIPtr> _sessions;

    static ReapThreadPtr _instance;
    static IceUtil::StaticMutex _instanceMutex;
};

#endif
