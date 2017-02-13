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

ICE_DECLSPEC_IMPORT void
allTests(const Ice::ObjectAdapterPtr&);

#if defined(_MSC_VER)
#   pragma comment(lib, ICE_LIBNAME("alltests"))
#endif

DEFINE_TEST("client")

using namespace std;

int
main(int argc, char* argv[])
{
    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        Ice::CommunicatorHolder ich = Ice::initialize(argc, argv, initData);

        // Collocated-only OA
        Ice::ObjectAdapterPtr oa = ich->createObjectAdapter("");

        oa->activate();
        allTests(oa);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
