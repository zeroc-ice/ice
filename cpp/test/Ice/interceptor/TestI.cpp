//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INTERCEPTOR_TEST_API_EXPORTS
#   define INTERCEPTOR_TEST_API_EXPORTS
#endif

#include <TestI.h>

using namespace IceUtil;
using namespace std;

#ifdef ICE_CPP11_MAPPING
Test::RetryException::~RetryException()
{
}

const ::std::string&
Test::RetryException::ice_staticId()
{
    static const ::std::string typeId = "::Test::RetryException";
    return typeId;
}
#else
Test::RetryException::RetryException(const char* file, int line) :
    ::Ice::LocalException(file, line)
{
}

Test::RetryException::~RetryException() throw()
{
}

::std::string
Test::RetryException::ice_id() const
{
    return "::Test::RetryException";
}

Test::RetryException*
Test::RetryException::ice_clone() const
{
    return new RetryException(*this);
}

void
Test::RetryException::ice_throw() const
{
    throw* this;
}
#endif

void
Test::RetryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nretry dispatch";
}
