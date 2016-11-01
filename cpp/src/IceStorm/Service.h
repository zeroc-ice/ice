// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICESTORM_SERVICE_H
#define ICESTORM_SERVICE_H

#include <IceBox/IceBox.h>
#include <IceStorm/IceStorm.h>

//
// Automatically link with IceStormService[D|++11|++11D].lib
//

#if !defined(ICE_BUILDING_ICE_STORM_SERVICE) && defined(ICESTORM_SERVICE_API_EXPORTS)
#   define ICE_BUILDING_ICE_STORM_SERVICE
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_STORM_SERVICE)
#   pragma comment(lib, ICE_LIBNAME("IceStormService"))
#endif

#ifndef ICESTORM_SERVICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESTORM_SERVICE_API /**/
#   elif defined(ICESTORM_SERVICE_API_EXPORTS)
#       define ICESTORM_SERVICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESTORM_SERVICE_API ICE_DECLSPEC_IMPORT
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

    ICESTORM_SERVICE_API static ServicePtr create(const Ice::CommunicatorPtr&,
                                                   const Ice::ObjectAdapterPtr&,
                                                   const Ice::ObjectAdapterPtr&,
                                                   const std::string&,
                                                   const Ice::Identity&,
                                                   const std::string&);

    virtual void start(const std::string&, const Ice::CommunicatorPtr&, const Ice::StringSeq&) = 0;

    ICESTORM_SERVICE_API virtual IceStorm::TopicManagerPrx getTopicManager() const = 0;

    virtual void stop() = 0;
};

};

#endif
