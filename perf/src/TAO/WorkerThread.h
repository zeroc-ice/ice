// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include <tao/ORB.h>
#include <ace/Task.h>

class WorkerThread : public ACE_Task_Base
{
public:
    WorkerThread(CORBA::ORB_ptr orb);

    virtual int svc();

private:
    CORBA::ORB_var _orb;
};

#endif /* WORKER_THREAD_H */
