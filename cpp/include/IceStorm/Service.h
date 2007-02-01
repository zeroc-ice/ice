// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESTORM_SERVICE_H
#define ICESTORM_SERVICE_H

#include <IceBox/IceBox.h>
#include <IceStorm/IceStorm.h>

#ifndef ICE_STORM_API
#   ifdef ICE_STORM_API_EXPORTS
#       define ICE_STORM_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_STORM_API ICE_DECLSPEC_IMPORT
#   endif
#endif

namespace IceStorm
{

class Service;
typedef ::IceInternal::Handle< ::IceStorm::Service> ServicePtr;

class ICE_STORM_API Service : public ::IceBox::Service
{
public:

    static ServicePtr create(const Ice::CommunicatorPtr&,
                             const Ice::ObjectAdapterPtr&,
                             const Ice::ObjectAdapterPtr&,
                             const std::string&,
                             const Ice::Identity&,
                             const std::string&);

    virtual void start(const std::string&, const Ice::CommunicatorPtr&, const Ice::StringSeq&) = 0;

    virtual TopicManagerPrx getTopicManager() const = 0;

    virtual void stop() = 0;
};

};

#endif
