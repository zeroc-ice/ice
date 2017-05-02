// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REFERENCE_FACTORY_H
#define ICE_REFERENCE_FACTORY_H

#include <IceUtil/Shared.h>
#include <Ice/ReferenceFactoryF.h>
#include <Ice/Reference.h> // For Reference::Mode
#include <Ice/ConnectionIF.h>
#include <Ice/BuiltinSequences.h>

namespace IceInternal
{

class ReferenceFactory : public ::IceUtil::Shared
{
public:

    //
    // Make a polymorphic copy of a reference.
    //
    ReferencePtr copy(const Reference* r) const;

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

    ReferenceFactory(const InstancePtr&, const ::Ice::CommunicatorPtr&);
    friend class Instance;

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
