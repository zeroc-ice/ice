// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

DEFINE_TEST("client")

int
run(int, char**)
{
    void allTests();
    allTests();

    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;

    try
    {
        status = run(argc, argv);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    return status;
}
