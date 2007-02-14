// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace std;
using namespace Ice;
using namespace Test;

RemoteObjectAdapterPrx
RemoteCommunicatorI::createObjectAdapter(const string& name, const string& endpoints, const Current& current)
{
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    if(com->getProperties()->getPropertyAsIntWithDefault("Ice.ThreadPerConnection", 0) == 0)
    {
        com->getProperties()->setProperty( name + ".ThreadPool.Size", "1");
    }
    ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, endpoints);
    return RemoteObjectAdapterPrx::uncheckedCast(current.adapter->addWithUUID(new RemoteObjectAdapterI(adapter)));
}

void
RemoteCommunicatorI::deactivateObjectAdapter(const RemoteObjectAdapterPrx& adapter, const Current& current)
{
    adapter->deactivate(); // Collocated call
}

void
RemoteCommunicatorI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}

RemoteObjectAdapterI::RemoteObjectAdapterI(const Ice::ObjectAdapterPtr& adapter) : 
    _adapter(adapter), 
    _testIntf(TestIntfPrx::uncheckedCast(_adapter->add(new TestI(), 
                                         adapter->getCommunicator()->stringToIdentity("test"))))
{
    _adapter->activate();
}

TestIntfPrx
RemoteObjectAdapterI::getTestIntf(const Ice::Current&)
{
    return _testIntf;
}

void
RemoteObjectAdapterI::deactivate(const Ice::Current&)
{
    try
    {
        _adapter->destroy();
    }
    catch(const ObjectAdapterDeactivatedException&)
    {
    }
}

std::string
TestI::getAdapterName(const Ice::Current& current)
{
    return current.adapter->getName();
}

