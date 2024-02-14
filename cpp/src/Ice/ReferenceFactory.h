//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_REFERENCE_FACTORY_H
#define ICE_REFERENCE_FACTORY_H

#include <Ice/ReferenceFactoryF.h>
#include <Ice/Reference.h> // For Reference::Mode
#include <Ice/ConnectionIF.h>
#include <Ice/BuiltinSequences.h>

namespace IceInternal
{

class ReferenceFactory : public std::enable_shared_from_this<ReferenceFactory>
{
public:

    ReferenceFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);

    //
    // Create a direct reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const ::std::string&, const ReferencePtr&,
                        const ::std::vector<EndpointIPtr>&);

    //
    // Create an indirect reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const ::std::string&, const ReferencePtr&, const std::string&);

    //
    // Create a fixed reference.
    //
    ReferencePtr create(const ::Ice::Identity&, const Ice::ConnectionIPtr&);

    //
    // Create a reference from a string.
    //
    ReferencePtr create(const ::std::string&, const std::string&);

    //
    // Create a reference by unmarshaling it from a stream.
    //
    ReferencePtr create(const ::Ice::Identity&, Ice::InputStream*);

    ReferenceFactoryPtr setDefaultRouter(const ::Ice::RouterPrxPtr&);
    ::Ice::RouterPrxPtr getDefaultRouter() const;

    ReferenceFactoryPtr setDefaultLocator(const ::Ice::LocatorPrxPtr&);
    ::Ice::LocatorPrxPtr getDefaultLocator() const;

private:

    void checkForUnknownProperties(const std::string&);
    RoutableReferencePtr create(const ::Ice::Identity&, const ::std::string&, Reference::Mode, bool,
                                const Ice::ProtocolVersion&, const Ice::EncodingVersion&,
                                const std::vector<EndpointIPtr>&, const std::string&, const std::string&);

    const InstancePtr _instance;
    const ::Ice::CommunicatorPtr _communicator;
    ::Ice::RouterPrxPtr _defaultRouter;
    ::Ice::LocatorPrxPtr _defaultLocator;
};

}

#endif
