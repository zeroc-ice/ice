// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/SUdpServerControlChannel.h>
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
IceSecurity::SecureUdp::ServerControlChannel::clientHello(const ClientChannelPtr& client,
                                                           const ByteSeq& MACkey,
                                                           const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->clientHello(client, MACkey);
    }
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientKeyAcknowledge(Long clientID,
                                                                    Long msgID,
                                                                    const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->clientKeyAcknowledge(clientID, msgID);
    }
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientKeyRequest(Long clientID,
                                                                const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->clientKeyRequest(clientID);
    }
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientGoodbye(Long clientID,
                                                             const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if (_transceiver != 0)
    {
        _transceiver->clientGoodbye(clientID);
    }
}

IceSecurity::SecureUdp::ServerControlChannel::ServerControlChannel(const SUdpTransceiverPtr& transceiver,
                                                                   const InstancePtr& instance,
                                                                   int port) :
                                             ControlChannel(transceiver, instance),
                                             _port(port)
{
    // Create the Server Channel's name
    ostringstream objectName;
    objectName << "sudpServer" << dec << _port;

    // This MUST be an SSL endpoint - secure handshake take place over this.
    ostringstream endpt;
    endpt << "ssl -p " << dec << _port;

    // Create the ObjectAdapter's name
    ostringstream objectAdapterName;
    objectAdapterName << "sudpServerControl" << dec << _port;

    Ice::CommunicatorPtr communicator = _instance->communicator();

    // Create our ObjectAdapter
    _adapter = communicator->createObjectAdapterWithEndpoints(objectAdapterName.str(), endpt.str());

    // The server control channel is the implemenation.
    ServerChannelPtr serverChannel = this;

    _adapter->add(serverChannel, Ice::stringToIdentity(objectName.str()));

    // Okay, allow the object to begin accepting requests
    _adapter->activate();
}

IceSecurity::SecureUdp::ServerControlChannel::~ServerControlChannel()
{
    // Make it impossible for the control channel to access the Transceiver
    // after transceiver destruction.
    unsetTransceiver();
}

