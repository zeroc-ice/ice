// **********************************************************************
//
// Copyright (c) 2003
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

#include <Ice/Application.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;

class TestClient : public Application
{
public:

    virtual int run(int, char*[]);

#if defined(__SUNPRO_CC) && (__SUNPRO_CC==0x550)
//
// Sun C++ 5.5 needs a dummy data member here.
//
private:
    char _dummy;
#endif


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
    TestPrx allTests(const CommunicatorPtr&);
    TestPrx obj = allTests(communicator());
    return EXIT_SUCCESS;
}
