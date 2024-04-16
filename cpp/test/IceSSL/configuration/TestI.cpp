//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "TestI.h"
#include "Ice/Ice.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;

ServerI::ServerI(const CommunicatorPtr& communicator) : _communicator(communicator) {}

void
ServerI::noCert(const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfoPtr info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(c.con->getInfo());
        test(info->certs.size() == 0);
    }
    catch (const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
}

void
ServerI::checkCert(string subjectDN, string issuerDN, const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfoPtr info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(c.con->getInfo());
        test(info->verified);
        test(info->certs.size() == 2);
        test(info->certs[0]->getSubjectDN() == IceSSL::DistinguishedName(subjectDN));
        test(info->certs[0]->getIssuerDN() == IceSSL::DistinguishedName(issuerDN));
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }
}

void
ServerI::checkCipher(string cipher, const Ice::Current& c)
{
    try
    {
        IceSSL::ConnectionInfoPtr info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(c.con->getInfo());
        test(info->cipher.compare(0, cipher.size(), cipher) == 0);
    }
    catch (const Ice::LocalException&)
    {
        test(false);
    }
}

void
ServerI::destroy()
{
    _communicator->destroy();
}

ServerFactoryI::ServerFactoryI(const string& defaultDir) : _defaultDir(defaultDir) {}

optional<Test::ServerPrx>
ServerFactoryI::createServer(Test::Properties props, const Current&)
{
    InitializationData initData;
    initData.properties = createProperties();
    for (Test::Properties::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        initData.properties->setProperty(p->first, p->second);
    }
    initData.properties->setProperty("IceSSL.DefaultDir", _defaultDir);

    CommunicatorPtr communicator = initialize(initData);
    ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("ServerAdapter", "ssl");
    ServerIPtr server = make_shared<ServerI>(communicator);
    Test::ServerPrx obj(adapter->addWithUUID(server));
    _servers[obj->ice_getIdentity()] = server;
    adapter->activate();

    return obj;
}

void
ServerFactoryI::destroyServer(optional<Test::ServerPrx> srv, const Ice::Current&)
{
    map<Identity, ServerIPtr>::iterator p = _servers.find(srv->ice_getIdentity());
    if (p != _servers.end())
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
