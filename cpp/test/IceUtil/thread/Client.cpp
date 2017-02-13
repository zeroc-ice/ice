// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <stdlib.h>
#include <TestCommon.h>

#include <TestSuite.h>

using namespace std;

int
main(int, char**)
{
    try
    {
        initializeTestSuite();

        for(list<TestBasePtr>::const_iterator p = allTests.begin(); p != allTests.end(); ++p)
        {
            (*p)->start();
        }
    }
    catch(const TestFailed& e)
    {
        cout << "test " << e.name << " failed" << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
