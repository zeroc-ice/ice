//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef INTERCEPTOR_TEST_API_EXPORTS
#   define INTERCEPTOR_TEST_API_EXPORTS
#endif
#include <Ice/Ice.h>
#include <Test.h>
#include <iostream>

using namespace IceUtil;
using namespace std;

void
Test::RetryException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nretry dispatch";
}
