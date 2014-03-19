// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <WorkerThread.h>

WorkerThread::WorkerThread(CORBA::ORB_ptr orb) : 
    _orb(CORBA::ORB::_duplicate(orb))
{
}

int
WorkerThread::svc()
{
  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
  {
      _orb->run(ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
  }
  ACE_CATCHANY {} ACE_ENDTRY;
  return 0;
}
