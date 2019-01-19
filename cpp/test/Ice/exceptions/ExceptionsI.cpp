//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifdef ICE_AMD_TEST
#   include <TestAMD.h>
#else
#   include <Test.h>
#endif

using namespace std;

void
Test::F::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(!data.empty())
    {
        out << " data:'F'";
    }
}

void
Test::H::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    if(!data.empty())
    {
        out << " data:'H'";
    }
}
