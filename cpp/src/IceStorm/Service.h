// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESTORM_SERVICE_H
#define ICESTORM_SERVICE_H

#include <IceBox/IceBox.h>
#include <IceStorm/IceStorm.h>

#ifdef _MSC_VER
//
// Automatically link with IceStormService[D].lib
//
#   if defined(ICE_STATIC_LIBS)
#      pragma comment(lib, "IceStormService.lib")
#   elif !defined(ICE_STORM_SERVICE_API_EXPORTS)
#      if defined(_DEBUG)
#          pragma comment(lib, "IceStormServiceD.lib")
#      else
#          pragma comment(lib, "IceStormService.lib")
#      endif
#   endif
#endif

#ifndef ICE_STORM_SERVICE_API
#   ifdef ICE_STORM_SERVICE_API_EXPORTS
#       define ICE_STORM_SERVICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_STORM_SERVICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif


// This API is internal to Ice, and should not be used by external
// applications.
namespace IceStormInternal
{

class Service;
typedef ::IceInternal::Handle< IceStormInternal::Service> ServicePtr;

class Service : public ::IceBox::Service
{
public:

    ICE_STORM_SERVICE_API static ServicePtr create(const Ice::CommunicatorPtr&,
                                                   const Ice::ObjectAdapterPtr&,
                                                   const Ice::ObjectAdapterPtr&,
                                                   const std::string&,
                                                   const Ice::Identity&,
                                                   const std::string&);

    virtual void start(const std::string&, const Ice::CommunicatorPtr&, const Ice::StringSeq&) = 0;

    ICE_STORM_SERVICE_API virtual IceStorm::TopicManagerPrx getTopicManager() const = 0;

    virtual void stop() = 0;
};

};

#endif
