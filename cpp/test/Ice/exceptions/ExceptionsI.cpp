// Copyright (c) ZeroC, Inc.

#ifdef ICE_AMD_TEST
#    include "TestAMD.h"
#else
#    include "Test.h"
#endif

using namespace std;

void
Test::F::ice_print(ostream& out) const
{
    UserException::ice_print(out);
    if (!data.empty())
    {
        out << " data:'F'";
    }
}
