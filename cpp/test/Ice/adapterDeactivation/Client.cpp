// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Application.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;
using namespace Test;

class TestClient : public Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    TestClient app;
    return app.main(argc, argv);
}

int
TestClient::run(int argc, char* argv[])
{
    TestIntfPrx allTests(const CommunicatorPtr&);
    TestIntfPrx obj = allTests(communicator());
    return EXIT_SUCCESS;
}
