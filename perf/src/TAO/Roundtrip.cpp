// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Roundtrip.h>

Roundtrip::Roundtrip(CORBA::ORB_ptr orb) : 
    _orb(CORBA::ORB::_duplicate(orb))
{
}

void
Roundtrip::sendByteSeq(const Test::ByteSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip::sendStringSeq(const Test::StringSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip::sendStringDoubleSeq(const Test::StringDoubleSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip::test_method(ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip::test_oneway(ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}

void
Roundtrip::test_oneway_with_data(const Test::ByteSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
    ACE_THROW_SPEC((CORBA::SystemException))
{
}


void
Roundtrip::shutdown(ACE_ENV_SINGLE_ARG_DECL)
    ACE_THROW_SPEC((CORBA::SystemException))
{
    _orb->shutdown(0 ACE_ENV_ARG_PARAMETER);
}
