// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    cout << "testing whether server is gone... " << flush;
    try
    {
	obj->ice_ping();
	test(false);
    }
    catch(const LocalException&)
    {
	cout << "ok" << endl;
    }

    return EXIT_SUCCESS;
}
