// Copyright (c) ZeroC, Inc.

#ifndef ICE_IAP_CONNECTOR_H
#define ICE_IAP_CONNECTOR_H

#include "../Connector.h"
#include "../ProtocolInstanceF.h"
#include "../TraceLevelsF.h"
#include "../TransceiverF.h"
#include "Ice/Logger.h"

#import <ExternalAccessory/ExternalAccessory.h>
#import <Foundation/Foundation.h>

namespace IceObjC
{
    class iAPEndpointI;

    class Instance;
    using InstancePtr = std::shared_ptr<Instance>;

    class iAPConnector final : public IceInternal::Connector
    {
    public:
        iAPConnector(
            const IceInternal::ProtocolInstancePtr&,
            std::int32_t,
            const std::string&,
            NSString*,
            EAAccessory*);
        ~iAPConnector();
        IceInternal::TransceiverPtr connect() final;

        std::int16_t type() const final;
        std::string toString() const final;

        bool operator==(const IceInternal::Connector&) const final;
        bool operator<(const IceInternal::Connector&) const final;

    private:
        const IceInternal::ProtocolInstancePtr _instance;
        const std::int32_t _timeout;
        const std::string _connectionId;
        NSString* _protocol;
        EAAccessory* _accessory;
    };
}

#endif
