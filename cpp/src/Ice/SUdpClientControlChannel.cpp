// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Stream.h>
#include <Ice/SUdpClientControlChannel.h>
#include <Ice/Instance.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/IdentityUtil.h>
#include <Ice/SUdpTransceiver.h>
#include <Ice/MessageAuthenticator.h>
#include <Ice/Cryptor.h>
#include <Ice/CryptKey.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceSecurity::SecureUdp;
using IceInternal::BasicStream;

////////////////////////////////////////////////////////////////////////////////
// Public Incoming Methods (from Ice Client Control Channel)
////////////////////////////////////////////////////////////////////////////////

void
IceSecurity::SecureUdp::ClientControlChannel::serverHello(Long clientID, const ByteSeq& key, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    _clientID = clientID;

    serverKeyChange(key);
}

void
IceSecurity::SecureUdp::ClientControlChannel::serverKeyChange(const ByteSeq& key, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    serverKeyChange(key);
}

void
IceSecurity::SecureUdp::ClientControlChannel::serverGoodbye(const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    // TODO: Should find some way to shut down the Transceiver here.
}

////////////////////////////////////////////////////////////////////////////////
// Protected Methods
////////////////////////////////////////////////////////////////////////////////

IceSecurity::SecureUdp::ClientControlChannel::ClientControlChannel(const SUdpTransceiverPtr& transceiver,
                                                                   const InstancePtr& instance,
                                                                   const std::string& host,
                                                                   int port) :
                                             ControlChannel(transceiver, instance)
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

    // Create our connection to the Server channel
    ostringstream ref;
    ref << "sudpServer" << dec << port << ":ssl -p " << dec << port << " -h " << host;
    Ice::ObjectPrx base = communicator->stringToProxy(ref.str());
    ServerChannelPrx twoway = ServerChannelPrx::checkedCast(base);
    _serverChannel = ServerChannelPrx::uncheckedCast(twoway->ice_oneway());

    _messageAuthenticator = new MessageAuthenticator();

    _clientID = 0L;
    _msgID = 0L;

    clientHello();
}

IceSecurity::SecureUdp::ClientControlChannel::~ClientControlChannel()
{
    // Make it impossible for the control channel to access the Transceiver
    // after transceiver destruction.
    unsetTransceiver();
}

void
IceSecurity::SecureUdp::ClientControlChannel::serverKeyChange(const ByteSeq& key)
{
    Long msgID = _msgID + 1;

    // From this msgID onwards, use the indicated key
    _encryptionKey = _cryptor->getOrCreateKey(key);

    _serverChannel->clientKeyAcknowledge(_clientID, msgID, _encryptionKey->toByteSeq());
}

void
IceSecurity::SecureUdp::ClientControlChannel::clientHello()
{
    _serverChannel->clientHello(ClientChannelPtr(this), _messageAuthenticator->getMACKey());
}


////////////////////////////////////////////////////////////////////////////////
// Incoming Methods (from SUdpTransceiver)
////////////////////////////////////////////////////////////////////////////////

void
IceSecurity::SecureUdp::ClientControlChannel::encryptPacket(Buffer& buffer, Buffer& encryptedPacket)
{
    IceUtil::Mutex::Lock sync(_mutex);

    // We always, and ONLY, increment here.
    ++_msgID;

    Long messageLength = buffer.b.end() - buffer.b.begin();

    // 1. Compute the MAC
    ByteSeq macBuffer = _messageAuthenticator->computeMAC(buffer.b);

    // 2. Append MAC to Message, produce signed message
    BasicStream signedMessageStream(0);
    signedMessageStream.write(messageLength);
    signedMessageStream.write(buffer.b);
    signedMessageStream.write(macBuffer);

    // 3. Encrypt signed message
    ByteSeq signedMessage;
    ByteSeq encryptedMessage;
    signedMessageStream.read(signedMessage);
    _cryptor->encrypt(_encryptionKey, signedMessage, encryptedMessage);

    // 4. Create record header
    BasicStream headerStream(0);
    Byte version = 1;
    Long length = encryptedMessage.end() - encryptedMessage.begin();
    headerStream.write(version);
    headerStream.write(_clientID);
    headerStream.write(_msgID);
    headerStream.write(length);

    // 5. Prepend header to encrypted message, create SUDP Packet
    BasicStream sudpPacket(0);
    ByteSeq header;
    headerStream.read(header);
    sudpPacket.write(header);
    sudpPacket.write(encryptedMessage);

    // Pass the encrypted packet back.
    sudpPacket.read(encryptedPacket.b);
}

void
IceSecurity::SecureUdp::ClientControlChannel::clientKeyRequest()
{
    _serverChannel->clientKeyRequest(_clientID);
}


