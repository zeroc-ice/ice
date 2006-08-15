// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ROUNDTRIP_HANDLER_H
#define ROUNDTRIP_HANDLER_H

#include <TestS.h>

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>

#if defined (_MSC_VER)
#   if (_MSC_VER >= 1200)
#       pragma warning(push)
#   endif /* _MSC_VER >= 1200 */
#   pragma warning (disable:4250)
#endif /* _MSC_VER */

//
// Implement the Test::Roundtrip interface
//
class Roundtrip_Handler : public virtual POA_Test::AMI_RoundtripHandler, 
    public virtual PortableServer::RefCountServantBase,
    public IceUtil::Monitor<IceUtil::Mutex>
{
public:
    Roundtrip_Handler();

    void waitFinished();

    // = The skeleton methods
    virtual void sendByteSeq(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void sendByteSeq_excep(Messaging::ExceptionHolder *holder
				   ACE_ENV_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

    virtual void sendStringSeq(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void sendStringSeq_excep(Messaging::ExceptionHolder *holder
				     ACE_ENV_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

    virtual void sendStringDoubleSeq(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void sendStringDoubleSeq_excep(Messaging::ExceptionHolder *holder
					   ACE_ENV_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

    virtual void test_method(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));
    virtual void test_method_excep(Messaging::ExceptionHolder *holder
				    ACE_ENV_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

    virtual void shutdown(ACE_ENV_SINGLE_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

    virtual void shutdown_excep(Messaging::ExceptionHolder *holder
				 ACE_ENV_ARG_DECL)
	ACE_THROW_SPEC ((CORBA::SystemException));

private:

    bool _finished;
};

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#    pragma warning(pop)
#endif /* _MSC_VER */

#endif /* ROUNDTRIP_H */
