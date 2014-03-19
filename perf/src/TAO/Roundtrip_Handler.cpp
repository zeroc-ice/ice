// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Roundtrip_Handler.h>

Roundtrip_Handler::Roundtrip_Handler() :
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
Roundtrip_Handler::sendByteSeq(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _finished = true;
    notify();
}

void
Roundtrip_Handler::sendByteSeq_excep(Messaging::ExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    ACE_TRY
    {
        holder->raise_exception(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "sendByteSeq:");
    }
    ACE_ENDTRY;
}

void
Roundtrip_Handler::sendStringSeq(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip_Handler::sendStringSeq_excep(Messaging::ExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    ACE_TRY
    {
        holder->raise_exception(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "sendStringSeq:");
    }
    ACE_ENDTRY;
}

void
Roundtrip_Handler::sendStringDoubleSeq(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip_Handler::sendStringDoubleSeq_excep(Messaging::ExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    ACE_TRY
    {
        holder->raise_exception(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "sendStringDoubleSeq:");
    }
    ACE_ENDTRY;
}

void
Roundtrip_Handler::test_method(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _finished = true;
    notify();
}

void
Roundtrip_Handler::test_method_excep(Messaging::ExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    ACE_TRY
    {
        holder->raise_exception(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "test_method:");
    }
    ACE_ENDTRY;
}

void
Roundtrip_Handler::shutdown(ACE_ENV_SINGLE_ARG_DECL_NOT_USED)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _finished = true;
    notify();
}

void
Roundtrip_Handler::shutdown_excep(Messaging::ExceptionHolder *holder
                                      ACE_ENV_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    ACE_TRY
    {
        holder->raise_exception(ACE_ENV_SINGLE_ARG_PARAMETER);
        ACE_TRY_CHECK;
    }
    ACE_CATCHANY
    {
        ACE_PRINT_EXCEPTION(ACE_ANY_EXCEPTION, "shutdown:");
    }
    ACE_ENDTRY;
}
