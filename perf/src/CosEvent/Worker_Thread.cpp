//
// Worker_Thread.cpp,v 1.3 2002/01/30 20:16:57 okellogg Exp
//
#include "Worker_Thread.h"

ACE_RCSID(Thread_Pool_Latency, Worker_Thread, "Worker_Thread.cpp,v 1.3 2002/01/30 20:16:57 okellogg Exp")

Worker_Thread::Worker_Thread (CORBA::ORB_ptr orb)
  : orb_ (CORBA::ORB::_duplicate (orb))
{
}

int
Worker_Thread::svc (void)
{
  ACE_DECLARE_NEW_CORBA_ENV;
  ACE_TRY
    {
      this->orb_->run (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY {} ACE_ENDTRY;
  return 0;
}
