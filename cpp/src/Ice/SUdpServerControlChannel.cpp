// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Security.h>
#include <Ice/Buffer.h>
#include <Ice/SUdpServerControlChannel.h>
#include <Ice/SUdpClient.h>
#include <Ice/Instance.h>
#include <Ice/Communicator.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/IdentityUtil.h>
#include <Ice/SUdpTransceiver.h>
#include <Ice/MessageAuthenticator.h>
#include <Ice/Cryptor.h>
#include <Ice/BasicStream.h>
#include <Ice/TraceLevels.h>
#include <Ice/Logger.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceSecurity::SecureUdp;
using IceInternal::Buffer;
using IceInternal::BasicStream;

void
IceSecurity::SecureUdp::ServerControlChannel::clientHello(const ClientChannelPrx& client,
                                                          const ByteSeq& MACkey,
                                                          const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    ICE_METHOD_INV("ServerControlChannel::clientHello()");

    Long clientID = getNewClientID();

    MessageAuthenticatorPtr messageAuthenticator = new MessageAuthenticator(MACkey);

    ICE_DEV_DEBUG("Creating new SUdpClient()");

    SUdpClientPtr sudpClient = new SUdpClient(clientID, client, messageAuthenticator);

    ICE_DEV_DEBUG("Creating new CryptKey()");

    assert(_cryptor);

    CryptKeyPtr cryptKey = _cryptor->getNewKey();

    ICE_DEV_DEBUG("Sending serverHello()");

    sudpClient->serverHello(cryptKey);

    ICE_DEV_DEBUG("adding new SUdpClient() to map.");

    newSUdpClient(sudpClient);

    ICE_METHOD_RET("ServerControlChannel::clientHello()");
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientKeyAcknowledge(Long clientID,
                                                                   Long msgID,
                                                                   const ByteSeq& key,
                                                                   const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    ICE_METHOD_INV("ServerControlChannel::clientKeyAcknowledge()");

    SUdpClientPtr sudpClient = getSUdpClient(clientID);

    CryptKeyPtr cryptKey = _cryptor->getKey(key);

    sudpClient->setNewCryptKey(msgID, cryptKey);

    ICE_METHOD_RET("ServerControlChannel::clientKeyAcknowledge()");
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientKeyRequest(Long clientID, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    ICE_METHOD_INV("ServerControlChannel::clientKeyRequest()");

    SUdpClientPtr sudpClient = getSUdpClient(clientID);

    CryptKeyPtr cryptKey = _cryptor->getNewKey();

    sudpClient->serverKeyChange(cryptKey);

    ICE_METHOD_RET("ServerControlChannel::clientKeyRequest()");
}

void
IceSecurity::SecureUdp::ServerControlChannel::clientGoodbye(Long clientID, const Current&)
{
    IceUtil::Mutex::Lock sync(_mutex);

    ICE_METHOD_INV("ServerControlChannel::clientGoodbye()");

    deleteSUdpClient(clientID);

    ICE_METHOD_RET("ServerControlChannel::clientGoodbye()");
}

// IceSecurity::SecureUdp::ServerControlChannel::ServerControlChannel(const SUdpTransceiverPtr& transceiver,
IceSecurity::SecureUdp::ServerControlChannel::ServerControlChannel(SUdpTransceiver* transceiver,
                                                                   const InstancePtr& instance,
                                                                   int port) :
                                             ControlChannel(transceiver, instance)
{
    ICE_METHOD_INV("ServerControlChannel::ServerControlChannel()");

    _clientIDGenerator = 0L;

    // Create the Server Channel's name
    ostringstream objectName;
    objectName << "sudpServer" << dec << port;

    // This MUST be an SSL endpoint - secure handshake take place over this.
    ostringstream endpt;
    endpt << "ssl -p " << dec << port << " -h 127.0.0.1";

    // Create the ObjectAdapter's name
    ostringstream objectAdapterName;
    objectAdapterName << "sudpServerControl" << dec << port;

    Ice::CommunicatorPtr communicator = _instance->communicator();

    ICE_DEV_DEBUG("Creating ObjectAdapter.");

    // Create our ObjectAdapter
    _adapter = communicator->createObjectAdapterWithEndpoints(objectAdapterName.str(), endpt.str());

    ICE_DEV_DEBUG("ObjectAdapter created.");

    // The server control channel is the implemenation.
    ServerChannelPtr serverChannel = this;

    ICE_DEV_DEBUG("Adding serverChannel to ObjectAdapter.");

    _adapter->add(serverChannel, Ice::stringToIdentity(objectName.str()));

    ICE_DEV_DEBUG("Added serverChannel named: " + objectName.str());

    ICE_DEV_DEBUG("Activating ObjectAdapter.");

    // Okay, allow the object to begin accepting requests
    _adapter->activate();

    ICE_DEV_DEBUG("ObjectAdapter activated.");

    ICE_METHOD_RET("ServerControlChannel::ServerControlChannel()");
}

IceSecurity::SecureUdp::ServerControlChannel::~ServerControlChannel()
{
    // Make it impossible for the control channel to access the Transceiver
    // after transceiver destruction.
    unsetTransceiver();
}

void
IceSecurity::SecureUdp::ServerControlChannel::decryptPacket(Buffer& encryptedPacket, Buffer& decryptedPacket)
{
    IceUtil::Mutex::Lock sync(_mutex);

    // Header information at the beginning of the packet.
    Byte version;
    Long clientID;
    Long msgID;
    Long length;

    // The encrypted message, final portion of the packet.
    ByteSeq encryptedMessage;


    // 1. Break apart header and encrypted Message
    BasicStream sudpPacket(0);
    sudpPacket.read(version);
    sudpPacket.read(clientID);
    sudpPacket.read(msgID);
    sudpPacket.read(length);
    sudpPacket.read(encryptedMessage);
    
    // TODO: Perform version check here.

    // Check to make sure the encrypted message is the correct length
    Long messageLength = encryptedMessage.end() - encryptedMessage.begin();
    if (messageLength != length)
    {
        // TODO: Bad scene, this message is not the right length!
    }


    // 2. Decrypt the Encrypted message
    ByteSeq signedMessage;
    SUdpClientPtr sudpClient = getSUdpClient(clientID);
    CryptKeyPtr cryptKey = sudpClient->getCryptKey(msgID);
    _cryptor->decrypt(cryptKey, encryptedMessage, signedMessage);


    // 3. Split out the message and MAC code from the signedMessage
    // Note: We're reusing the messageLength variable here.
    BasicStream signedMessageStream(0);
    ByteSeq messageAndMAC;
    signedMessageStream.write(signedMessage);
    signedMessageStream.read(messageLength);
    signedMessageStream.read(messageAndMAC);
    ByteSeq::iterator current = messageAndMAC.begin();
    ByteSeq::iterator macPos = current + messageLength;
    ByteSeq::iterator end = messageAndMAC.end();
    ByteSeq message(current, macPos);
    ByteSeq macCode(macPos, end);


    // 4. Compute MAC and compare
    MessageAuthenticatorPtr messageAuthenticator = sudpClient->getMessageAuthenticator();
    if (messageAuthenticator->authenticate(message, macCode))
    {
        // TODO: Bad scene, the MAC codes don't match, this message is invalid!
    }

    decryptedPacket.b = message;
}

Long
IceSecurity::SecureUdp::ServerControlChannel::getNewClientID()
{
    IceUtil::Mutex::Lock sync(_clientIDMutex);
    return ++_clientIDGenerator;
}

SUdpClientPtr&
IceSecurity::SecureUdp::ServerControlChannel::getSUdpClient(Long clientID)
{
    IceUtil::Mutex::Lock sync(_clientMapMutex);

    return _clientMap[clientID];
}

void
IceSecurity::SecureUdp::ServerControlChannel::newSUdpClient(const SUdpClientPtr& sudpClient)
{
    IceUtil::Mutex::Lock sync(_clientMapMutex);

    pair<Long, SUdpClientPtr> clientPair(sudpClient->getClientID(), sudpClient);

    _clientMap.insert(clientPair);
}

void
IceSecurity::SecureUdp::ServerControlChannel::deleteSUdpClient(Long clientID)
{
    IceUtil::Mutex::Lock sync(_clientMapMutex);

    _clientMap.erase(clientID);
}

