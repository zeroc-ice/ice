// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <stdlib.h>
#include <TestCommon.h>

#include <TestSuite.h>

using namespace std;

int
main(int argc, char** argv)
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
