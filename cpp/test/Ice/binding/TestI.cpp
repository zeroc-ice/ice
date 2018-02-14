// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

RemoteCommunicatorI::RemoteCommunicatorI() : _nextPort(10001)
{
}

RemoteObjectAdapterPrx
RemoteCommunicatorI::createObjectAdapter(const string& name, const string& endpts, const Current& current)
{
    string endpoints = endpts;
    if(endpoints.find("-p") == string::npos)
    {
        // Use a fixed port if none is specified (bug 2896)
        ostringstream os;
        os << endpoints << " -h \""
           << (current.adapter->getCommunicator()->getProperties()->getPropertyWithDefault(
                                                                                    "Ice.Default.Host", "127.0.0.1"))
           << "\" -p " << _nextPort++;
        endpoints = os.str();
    }
    
    Ice::CommunicatorPtr com = current.adapter->getCommunicator();
    com->getProperties()->setProperty(name + ".ThreadPool.Size", "1");
    ObjectAdapterPtr adapter = com->createObjectAdapterWithEndpoints(name, endpoints);
    return RemoteObjectAdapterPrx::uncheckedCast(current.adapter->addWithUUID(new RemoteObjectAdapterI(adapter)));
}

void
RemoteCommunicatorI::deactivateObjectAdapter(const RemoteObjectAdapterPrx& adapter, const Current&)
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

