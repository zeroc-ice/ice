// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Props.h>

using namespace std;

//
// The servant implements the Slice interface Demo::Props as well as the
// native callback class Ice::PropertiesAdminUpdateCallback. Since both
// base classes inherit from IceUtil::Shared, the servant must use
// virtual inheritance.
//
class PropsI : virtual public Demo::Props,
               virtual public Ice::PropertiesAdminUpdateCallback,
               IceUtil::Monitor<IceUtil::Mutex>
{
public:

    PropsI() : _called(false)
    {
    }

    virtual Ice::PropertyDict getChanges(const Ice::Current&)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        //
        // Make sure that we have received the property updates before we
        // return the results.
        //
        while(!_called)
        {
            wait();
        }

        _called = false;
        return _changes;
    }

    virtual void shutdown(const Ice::Current& current)
    {
        current.adapter->getCommunicator()->shutdown();
    }

    virtual void updated(const Ice::PropertyDict& changes)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

        _changes = changes;
        _called = true;
        notify();
    }

private:

    Ice::PropertyDict _changes;
    bool _called;
};
typedef IceUtil::Handle<PropsI> PropsIPtr;

class Server : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
main(int argc, char* argv[])
{
    Server app;
    return app.main(argc, argv, "config.server");
}

int
Server::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    PropsIPtr props = new PropsI;

    //
    // Retrieve the PropertiesAdmin facet and register the servant as the update callback.
    //
    Ice::ObjectPtr obj = communicator()->findAdminFacet("Properties");
    Ice::NativePropertiesAdminPtr admin = Ice::NativePropertiesAdminPtr::dynamicCast(obj);
    admin->addUpdateCallback(props);

    Ice::ObjectAdapterPtr adapter = communicator()->createObjectAdapter("Props");
    adapter->add(props, communicator()->stringToIdentity("props"));
    adapter->activate();
    communicator()->waitForShutdown();
    return EXIT_SUCCESS;
}
