// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
