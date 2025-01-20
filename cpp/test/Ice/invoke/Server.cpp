// Copyright (c) ZeroC, Inc.

#include "../../src/Ice/Options.h"
#include "BlobjectI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;

class ServantLocatorI final : public Ice::ServantLocator
{
public:
    ServantLocatorI(bool array, bool async)
    {
        if (array)
        {
            if (async)
            {
                _blobject = make_shared<BlobjectArrayAsyncI>();
            }
            else
            {
                _blobject = make_shared<BlobjectArrayI>();
            }
        }
        else
        {
            if (async)
            {
                _blobject = make_shared<BlobjectAsyncI>();
            }
            else
            {
                _blobject = make_shared<BlobjectI>();
            }
        }
    }

    Ice::ObjectPtr locate(const Ice::Current&, shared_ptr<void>&) final { return _blobject; }

    void finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>&) final {}

    void deactivate(string_view) final {}

private:
    Ice::ObjectPtr _blobject;
};

class Server : public Test::TestHelper
{
public:
    void run(int, char**) override;
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceInternal::Options opts;
    opts.addOpt("", "array");
    opts.addOpt("", "async");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch (const IceInternal::BadOptException& e)
    {
        cout << argv[0] << ": error: " << e.what() << endl;
        throw;
    }
    bool array = opts.isSet("array");
    bool async = opts.isSet("async");

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint());
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->addServantLocator(make_shared<ServantLocatorI>(array, async), "");
    adapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
