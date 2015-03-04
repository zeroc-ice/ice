// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_ENDPOINT_FACTORY_H
#define TEST_ENDPOINT_FACTORY_H

#include <Ice/EndpointFactory.h>

class EndpointFactory : public IceInternal::EndpointFactory
{
public:

    virtual ~EndpointFactory() { }

    virtual ::Ice::Short type() const;
    virtual ::std::string protocol() const;
    virtual IceInternal::EndpointIPtr create(const std::string&, bool) const;
    virtual IceInternal::EndpointIPtr read(IceInternal::BasicStream*) const;
    virtual void destroy();

protected:

    EndpointFactory(const IceInternal::EndpointFactoryPtr&);
    friend class TestPluginI;

    IceInternal::EndpointFactoryPtr _factory;
};

#endif
