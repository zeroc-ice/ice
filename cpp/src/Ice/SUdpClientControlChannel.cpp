// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SUdpClientControlChannel.h>
#include <Ice/Instance.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/IdentityUtil.h>
#include <Ice/SUdpTransceiver.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceSecurity::SecureUdp;

void
IceSecurity::SecureUdp::ClientControlChannel::serverHello(Long clientID, const ByteSeq& key, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->serverHello(clientID, key);
    }
}

void
IceSecurity::SecureUdp::ClientControlChannel::serverKeyChange(const ByteSeq& key, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->serverKeyChange(key);
    }
}

void
IceSecurity::SecureUdp::ClientControlChannel::serverGoodbye(const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->serverGoodbye();
    }
}

IceSecurity::SecureUdp::ClientControlChannel::ClientControlChannel(const SUdpTransceiverPtr& transceiver,
                                                                   const InstancePtr& instance,
                                                                   const std::string& host,
                                                                   int port) :
                                             ControlChannel(transceiver, instance),
                                             _host(host),
                                             _port(port)
{
    // Create the Client Channel's name
    ostringstream objectName;
    objectName << "sudpClient" << hex << (void *) this;

    // This MUST be an SSL endpoint - secure handshake takes place over this.
    ostringstream endpt;
    endpt << "ssl";

    // Create the ObjectAdapter's name
    ostringstream objectAdapterName;
    objectAdapterName << "sudpClientControl" << hex << (void *) this;

    Ice::CommunicatorPtr communicator = _instance->communicator();

    // Create our ObjectAdapter
    _adapter = communicator->createObjectAdapterWithEndpoints(objectAdapterName.str(), endpt.str());

    // The client control channel is the implementaion.
    ClientChannelPtr clientChannel = this;

    _adapter->add(clientChannel, Ice::stringToIdentity(objectName.str()));

    // Okay, allow the object to begin accepting requests
    _adapter->activate();
}

IceSecurity::SecureUdp::ClientControlChannel::~ClientControlChannel()
{
    // Make it impossible for the control channel to access the Transceiver
    // after transceiver destruction.
    unsetTransceiver();
}
