//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <IceUtil/Options.h>
#include <BlobjectI.h>

using namespace std;

class ServantLocatorI final : public Ice::ServantLocator
{
public:

    ServantLocatorI(bool array, bool async)
    {
        if(array)
        {
            if(async)
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
            if(async)
            {
                _blobject = make_shared<BlobjectAsyncI>();
            }
            else
            {
                _blobject = make_shared<BlobjectI>();
            }
        }
    }

    shared_ptr<Ice::Object> locate(const Ice::Current&, shared_ptr<void>&) final
    {
        return _blobject;
    }

    void finished(const Ice::Current&, const shared_ptr<Ice::Object>&, const shared_ptr<void>&) final
    {
    }

    void deactivate(const string&) final
    {
    }

private:

    shared_ptr<Ice::Object> _blobject;
};

class Server : public Test::TestHelper
{
public:

    void run(int, char**);
};

void
Server::run(int argc, char** argv)
{
    Ice::CommunicatorHolder communicator = initialize(argc, argv);
    IceUtilInternal::Options opts;
    opts.addOpt("", "array");
    opts.addOpt("", "async");

    vector<string> args;
    try
    {
        args = opts.parse(argc, (const char**)argv);
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        cout << argv[0] << ": error: " << e.reason << endl;
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
