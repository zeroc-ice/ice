//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INTERCEPTOR_TEST_API_EXPORTS
#   define INTERCEPTOR_TEST_API_EXPORTS
#endif

#include <TestI.h>

using namespace IceUtil;
using namespace std;

Test::RetryException::~RetryException()
{
}

const ::std::string&
Test::RetryException::ice_staticId()
{
    static const ::std::string typeId = "::Test::RetryException";
    return typeId;
}

void
Test::RetryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nretry dispatch";
}
