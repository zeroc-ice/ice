//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REFERENCE_FACTORY_H
#define ICE_REFERENCE_FACTORY_H

#include "Ice/BuiltinSequences.h"
#include "Ice/ConnectionIF.h"
#include "Reference.h" // For Reference::Mode
#include "ReferenceFactoryF.h"

namespace IceInternal
{
    class ReferenceFactory : public std::enable_shared_from_this<ReferenceFactory>
    {
    public:
        ReferenceFactory(const InstancePtr&, const Ice::CommunicatorPtr&);

        //
        // Create a direct reference.
        //
        ReferencePtr
        create(const Ice::Identity&, const std::string&, const ReferencePtr&, const std::vector<EndpointIPtr>&);

        //
        // Create an indirect reference.
        //
        ReferencePtr create(const Ice::Identity&, const std::string&, const ReferencePtr&, const std::string&);

        //
        // Create a fixed reference.
        //
        ReferencePtr create(const Ice::Identity&, const Ice::ConnectionIPtr&);

        //
        // Create a reference from a string.
        //
        ReferencePtr create(std::string_view proxyString, const std::string& prefix);

        //
        // Create a reference by unmarshaling it from a stream.
        //
        ReferencePtr create(const Ice::Identity&, Ice::InputStream*);

        ReferenceFactoryPtr setDefaultRouter(const std::optional<Ice::RouterPrx>&);
        std::optional<Ice::RouterPrx> getDefaultRouter() const;

        ReferenceFactoryPtr setDefaultLocator(const std::optional<Ice::LocatorPrx>&);
        std::optional<Ice::LocatorPrx> getDefaultLocator() const;

    private:
        void checkForUnknownProperties(const std::string&);
        RoutableReferencePtr create(
            const Ice::Identity&,
            const std::string&,
            Reference::Mode,
            bool,
            const Ice::ProtocolVersion&,
            const Ice::EncodingVersion&,
            const std::vector<EndpointIPtr>&,
            const std::string&,
            const std::string&);

        const InstancePtr _instance;
        const Ice::CommunicatorPtr _communicator;
        std::optional<Ice::RouterPrx> _defaultRouter;
        std::optional<Ice::LocatorPrx> _defaultLocator;
    };
}

#endif
