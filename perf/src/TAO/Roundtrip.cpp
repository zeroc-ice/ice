//
// Roundtrip.cpp,v 1.4 2004/01/11 05:06:02 bala Exp
//
#include "Roundtrip.h"

#include <iostream>

ACE_RCSID(Thread_Pool_Latency, Roundtrip, "Roundtrip.cpp,v 1.4 2004/01/11 05:06:02 bala Exp")

Roundtrip::Roundtrip (CORBA::ORB_ptr orb)
  : orb_ (CORBA::ORB::_duplicate (orb))
{
}

void
Roundtrip::sendByteSeq (const Test::ByteSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip::sendStringSeq (const Test::StringSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip::sendStringDoubleSeq (const Test::StringDoubleSeq & ACE_ENV_ARG_DECL_WITH_DEFAULTS)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip::test_method (ACE_ENV_SINGLE_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip::test_oneway (ACE_ENV_SINGLE_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
}

void
Roundtrip::shutdown (ACE_ENV_SINGLE_ARG_DECL)
  ACE_THROW_SPEC ((CORBA::SystemException))
{
  this->orb_->shutdown (0 ACE_ENV_ARG_PARAMETER);
}
