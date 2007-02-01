// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/Thread.h>
#include <TestI.h>
#include <TestCommon.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Ice;

ServerI::ServerI(const CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
ServerI::noCert(const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfo info = IceSSL::getConnectionInfo(c.con);
        test(info.certs.size() == 0);
    }
    catch(const IceSSL::ConnectionInvalidException&)
    {
        test(false);
    }
}

void
ServerI::checkCert(const string& subjectDN, const string& issuerDN, const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfo info = IceSSL::getConnectionInfo(c.con);
        test(info.certs.size() == 2 &&
             info.certs[0]->getSubjectDN() == IceSSL::DistinguishedName(subjectDN) &&
             info.certs[0]->getIssuerDN() == IceSSL::DistinguishedName(issuerDN));
    }
    catch(const IceSSL::ConnectionInvalidException&)
    {
        test(false);
    }
}

void
ServerI::checkCipher(const string& cipher, const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfo info = IceSSL::getConnectionInfo(c.con);
        test(info.cipher.compare(0, cipher.size(), cipher) == 0);
    }
    catch(const IceSSL::ConnectionInvalidException&)
    {
        test(false);
    }
}

void
ServerI::destroy()
{
    _communicator->destroy();
}

Test::ServerPrx
ServerFactoryI::createServer(const Test::Properties& props, const Current& current)
{
    InitializationData initData;
    initData.properties = createProperties();
    for(Test::Properties::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        initData.properties->setProperty(p->first, p->second);
    }

    CommunicatorPtr communicator = initialize(initData);
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
    ServerIPtr server = new ServerI(communicator);
    ObjectPrx obj = adapter->addWithUUID(server);
    _servers[obj->ice_getIdentity()] = server;
    adapter->activate();

    return Test::ServerPrx::uncheckedCast(obj);
}

void
ServerFactoryI::destroyServer(const Test::ServerPrx& srv, const Ice::Current&)
{
    map<Identity, ServerIPtr>::iterator p = _servers.find(srv->ice_getIdentity());
    if(p != _servers.end())
    {
        p->second->destroy();
        _servers.erase(p);
    }
}

void
ServerFactoryI::shutdown(const Ice::Current& current)
{
    test(_servers.empty());
    current.adapter->getCommunicator()->shutdown();
}
