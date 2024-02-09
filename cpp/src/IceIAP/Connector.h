//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_IAP_CONNECTOR_H
#define ICE_IAP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/ProtocolInstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Connector.h>

#import <Foundation/Foundation.h>
#import <ExternalAccessory/ExternalAccessory.h>

namespace IceObjC
{

class iAPEndpointI;

class Instance;
using InstancePtr = std::shared_ptr<Instance>;

class iAPConnector final : public IceInternal::Connector
{
public:

    iAPConnector(const IceInternal::ProtocolInstancePtr&, Ice::Int, const std::string&, NSString*, EAAccessory*);
    ~iAPConnector();
    IceInternal::TransceiverPtr connect() final;

    Ice::Short type() const final;
    std::string toString() const final;

    bool operator==(const IceInternal::Connector&) const final;
    bool operator<(const IceInternal::Connector&) const final;

private:

    const IceInternal::ProtocolInstancePtr _instance;
    const Ice::Int _timeout;
    const std::string _connectionId;
    NSString* _protocol;
    EAAccessory* _accessory;
};

}

#endif
