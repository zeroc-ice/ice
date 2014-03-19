// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ROUNDTRIP_H
#define ROUNDTRIP_H

#include <TestS.h>

#if defined(_MSC_VER)
#   if (_MSC_VER >= 1200)
#      pragma warning(push)
#   endif /* _MSC_VER >= 1200 */
#   pragma warning (disable:4250)
#endif /* _MSC_VER */

//
// Implement the Test::Roundtrip interface
//
class Roundtrip : public virtual POA_Test::Roundtrip, 
    public virtual PortableServer::RefCountServantBase
{
public:

    Roundtrip(CORBA::ORB_ptr orb);

    // = The skeleton methods
    virtual void sendByteSeq(const Test::ByteSeq & seq ACE_ENV_ARG_DECL_WITH_DEFAULTS)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void sendStringSeq(const Test::StringSeq & seq ACE_ENV_ARG_DECL_WITH_DEFAULTS)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void sendStringDoubleSeq(const Test::StringDoubleSeq & seq ACE_ENV_ARG_DECL_WITH_DEFAULTS)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void test_method(ACE_ENV_SINGLE_ARG_DECL)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void test_oneway(ACE_ENV_SINGLE_ARG_DECL)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void test_oneway_with_data(const Test::ByteSeq& seq ACE_ENV_ARG_DECL_WITH_DEFAULTS)
        ACE_THROW_SPEC((CORBA::SystemException));

    virtual void shutdown(ACE_ENV_SINGLE_ARG_DECL)
        ACE_THROW_SPEC((CORBA::SystemException));

private:
    //
    // Use an ORB reference to conver strings to objects and shutdown
    // the application.
    // 
    CORBA::ORB_var _orb;
};

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma warning(pop)
#endif /* _MSC_VER */

#endif /* ROUNDTRIP_H */
