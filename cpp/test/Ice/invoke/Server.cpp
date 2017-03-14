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
#include <IceUtil/Options.h>
#include <BlobjectI.h>

DEFINE_TEST("server")

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


int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
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
        cout << argv[0] << ": " << e.reason << endl;
        return false;
    }
    bool array = opts.isSet("array");
    bool async = opts.isSet("async");

    communicator->getProperties()->setProperty("TestAdapter.Endpoints", getTestEndpoint(communicator, 0) + ":udp");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("TestAdapter");
    adapter->addServantLocator(ICE_MAKE_SHARED(ServantLocatorI, array, async), "");
    adapter->activate();

    TEST_READY

    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL(false);
#endif

    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
        Ice::InitializationData initData = getTestInitData(argc, argv);
        communicator = Ice::initialize(argc, argv, initData);
        status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        status = EXIT_FAILURE;
    }

    if(communicator)
    {
        communicator->destroy();
    }

    return status;
}
