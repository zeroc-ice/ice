//
// Roundtrip_Handler.cpp,v 1.4 2003/11/01 11:15:10 dhinton Exp
//
#include "Roundtrip_Handler.h"
#include "ace/OS_NS_time.h"

#include <iostream>

using namespace std;

ACE_RCSID(AMI_Latency, Roundtrip, "Roundtrip_Handler.cpp,v 1.4 2003/11/01 11:15:10 dhinton Exp")

Roundtrip_Handler::Roundtrip_Handler () :
    _finished(false)
{
}

void
Roundtrip_Handler::waitFinished()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    while(!_finished)
    {
        wait();
    }
    _finished = false;
}

void
Roundtrip_Handler::sendByteSeq (ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip_Handler::sendByteSeq_excep (Test::AMI_RoundtripExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  ACE_TRY
    {
      holder->raise_test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "sendByteSeq:");
    }
  ACE_ENDTRY;
}

void
Roundtrip_Handler::sendStringSeq (ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip_Handler::sendStringSeq_excep (Test::AMI_RoundtripExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  ACE_TRY
    {
      holder->raise_test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "sendStringSeq:");
    }
  ACE_ENDTRY;
}

void
Roundtrip_Handler::sendStringDoubleSeq (ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip_Handler::sendStringDoubleSeq_excep (Test::AMI_RoundtripExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  ACE_TRY
    {
      holder->raise_test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "sendStringDoubleSeq:");
    }
  ACE_ENDTRY;
}

void
Roundtrip_Handler::test_method (ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _finished = true;
    notify();
}

void
Roundtrip_Handler::test_method_excep (Test::AMI_RoundtripExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  ACE_TRY
    {
      holder->raise_test_method (ACE_ENV_SINGLE_ARG_PARAMETER);
      ACE_TRY_CHECK;
    }
  ACE_CATCHANY
    {
      ACE_PRINT_EXCEPTION (ACE_ANY_EXCEPTION, "test_method:");
    }
  ACE_ENDTRY;
}
