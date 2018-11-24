// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <IceUtil/Options.h>
#include <BlobjectI.h>

using namespace std;

class ServantLocatorI : public Ice::ServantLocator
{
public:

    ServantLocatorI(bool array, bool async)
    {
        if(array)
        {
            if(async)
            {
                _blobject = ICE_MAKE_SHARED(BlobjectArrayAsyncI);
            }
            else
            {
                _blobject = ICE_MAKE_SHARED(BlobjectArrayI);
            }
        }
        else
        {
            if(async)
            {
                _blobject = ICE_MAKE_SHARED(BlobjectAsyncI);
            }
            else
            {
                _blobject = ICE_MAKE_SHARED(BlobjectI);
            }
        }
    }

#ifdef ICE_CPP11_MAPPING
    virtual Ice::ObjectPtr
    locate(const Ice::Current&, shared_ptr<void>&)
    {
        return _blobject;
    }

    virtual void
    finished(const Ice::Current&, const Ice::ObjectPtr&, const shared_ptr<void>&)
    {
    }
#else
    virtual Ice::ObjectPtr
    locate(const Ice::Current&, Ice::LocalObjectPtr&)
    {
        return _blobject;
    }

    virtual void
    finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
    {
    }
#endif
    virtual void
    deactivate(const string&)
    {
    }

private:

    Ice::ObjectPtr _blobject;
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
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, array, async), "");
    adapter->activate();

    serverReady();

    communicator->waitForShutdown();
}

DEFINE_TEST(Server)
