// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPERF_WORKERTHREAD_H
#define ICEPERF_WORKERTHREAD_H

#include <tao/ORB.h>
#include <ace/Task.h>

//
// Worker thread for running the TAO ORB. 
//

class WorkerThread : public ACE_Task_Base
{
public:
    WorkerThread(CORBA::ORB_ptr orb);

    virtual int svc();

private:
    CORBA::ORB_var _orb;
};

#endif /* ICEPERF_WORKERTHREAD_H */ 
