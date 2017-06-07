// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>

using namespace std;

namespace
{

//
// A no-op Logger, used when testing the Logger Admin
//

class NullLogger : public Ice::Logger
#ifdef ICE_CPP11_MAPPING
                 , public std::enable_shared_from_this<NullLogger>
#endif
{
public:

    virtual void print(const string&)
    {
    }

    virtual void trace(const string&, const string&)
    {
    }

    virtual void warning(const string&)
    {
    }

    virtual void error(const string&)
    {
    }

    virtual string getPrefix()
    {
        return "NullLogger";
    }

    virtual Ice::LoggerPtr cloneWithPrefix(const string&)
    {
        return ICE_SHARED_FROM_THIS;
    }
};

}

RemoteCommunicatorI::RemoteCommunicatorI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator),
#ifdef ICE_CPP11_MAPPING
    _removeCallback(nullptr)
#else
    _hasCallback(false)
#endif
{
}

Ice::ObjectPrxPtr
RemoteCommunicatorI::getAdmin(const Ice::Current&)
{
    return _communicator->getAdmin();
}

Ice::PropertyDict
RemoteCommunicatorI::getChanges(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

#ifdef ICE_CPP11_MAPPING
    if(_removeCallback)
#else
    if(_hasCallback)
#endif
    {
       return _changes;
    }
    else
    {
        return Ice::PropertyDict();
    }
}

void
RemoteCommunicatorI::addUpdateCallback(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    Ice::ObjectPtr propFacet = _communicator->findAdminFacet("Properties");
    if(propFacet)
    {
        Ice::NativePropertiesAdminPtr admin = ICE_DYNAMIC_CAST(Ice::NativePropertiesAdmin, propFacet);
        assert(admin);
#ifdef ICE_CPP11_MAPPING
        _removeCallback =
            admin->addUpdateCallback([this](const Ice::PropertyDict& changes) { updated(changes); });
#else
        admin->addUpdateCallback(this);
        _hasCallback = true;
#endif
    }
}

void
RemoteCommunicatorI::removeUpdateCallback(const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    Ice::ObjectPtr propFacet = _communicator->findAdminFacet("Properties");
    if(propFacet)
    {
        Ice::NativePropertiesAdminPtr admin = ICE_DYNAMIC_CAST(Ice::NativePropertiesAdmin, propFacet);
        assert(admin);
#ifdef ICE_CPP11_MAPPING
        if(_removeCallback)
        {
            _removeCallback();
            _removeCallback = nullptr;
        }
#else
        admin->removeUpdateCallback(this);
        _hasCallback = false;
#endif
    }

}

void
RemoteCommunicatorI::print(ICE_IN(std::string) message, const Ice::Current&)
{
    _communicator->getLogger()->print(message);
}
void
RemoteCommunicatorI::trace(ICE_IN(std::string) category,
                           ICE_IN(std::string) message, const Ice::Current&)
{
    _communicator->getLogger()->trace(category, message);
}
void
RemoteCommunicatorI::warning(ICE_IN(std::string) message, const Ice::Current&)
{
    _communicator->getLogger()->warning(message);
}
void
RemoteCommunicatorI::error(ICE_IN(std::string) message, const Ice::Current&)
{
    _communicator->getLogger()->error(message);
}

void
RemoteCommunicatorI::shutdown(const Ice::Current&)
{
    _communicator->shutdown();
}

void
RemoteCommunicatorI::waitForShutdown(const Ice::Current&)
{
    //
    // Note that we are executing in a thread of the *main* communicator,
    // not the one that is being shut down.
    //
    _communicator->waitForShutdown();
}

void
RemoteCommunicatorI::destroy(const Ice::Current&)
{
    _communicator->destroy();
}

void
RemoteCommunicatorI::updated(const Ice::PropertyDict& changes)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    _changes = changes;
}

Test::RemoteCommunicatorPrxPtr
RemoteCommunicatorFactoryI::createCommunicator(ICE_IN(Ice::PropertyDict) props, const Ice::Current& current)
{
    //
    // Prepare the property set using the given properties.
    //
    Ice::InitializationData init;
    init.properties = Ice::createProperties();
    for(Ice::PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        init.properties->setProperty(p->first, p->second);
    }

    if(init.properties->getPropertyAsInt("NullLogger") > 0)
    {
        init.logger = ICE_MAKE_SHARED(NullLogger);
    }

    //
    // Initialize a new communicator.
    //
    Ice::CommunicatorPtr communicator = Ice::initialize(init);

    //
    // Install a custom admin facet.
    //
    communicator->addAdminFacet(ICE_MAKE_SHARED(TestFacetI), "TestFacet");

    //
    // The RemoteCommunicator servant also implements PropertiesAdminUpdateCallback.
    // Set the callback on the admin facet.
    //
    RemoteCommunicatorIPtr servant = ICE_MAKE_SHARED(RemoteCommunicatorI, communicator);
    servant->addUpdateCallback(Ice::emptyCurrent);

    Ice::ObjectPrxPtr proxy = current.adapter->addWithUUID(servant);
    return ICE_UNCHECKED_CAST(Test::RemoteCommunicatorPrx, proxy);
}

void
RemoteCommunicatorFactoryI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
