// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;
using namespace std;

PriorityI::PriorityI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
PriorityI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

int
PriorityI::getPriority(const Ice::Current&)
{
#ifdef _WIN32
    return GetThreadPriority(GetCurrentThread());
#else
    sched_param param;
    int sched_policy;
    pthread_t thread = pthread_self();
    pthread_getschedparam(thread, &sched_policy, &param);
    return param.sched_priority;
#endif
}
