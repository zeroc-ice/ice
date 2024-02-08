//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESTORM_SERVICE_H
#define ICESTORM_SERVICE_H

#include <IceBox/IceBox.h>
#include <IceStorm/IceStorm.h>

//
// Automatically link with IceStormService[D].lib
//

#if !defined(ICE_BUILDING_ICE_STORM_SERVICE) && defined(ICESTORM_SERVICE_API_EXPORTS)
#   define ICE_BUILDING_ICE_STORM_SERVICE
#endif

#if defined(_MSC_VER) && !defined(ICE_BUILDING_ICE_STORM_SERVICE)
#   pragma comment(lib, ICE_LIBNAME("IceStormService"))
#endif

// These IceStorm APIs are exported because they are used by IceGrid
#ifndef ICESTORM_SERVICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESTORM_SERVICE_API /**/
#   elif defined(ICESTORM_SERVICE_API_EXPORTS)
#       define ICESTORM_SERVICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESTORM_SERVICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

// This API is internal to Ice, and should not be used by external applications.
namespace IceStormInternal
{

class Service : public IceBox::Service
{
public:

    ICESTORM_SERVICE_API static std::shared_ptr<Service> create(const std::shared_ptr<Ice::Communicator>&,
                                                                 const std::shared_ptr<Ice::ObjectAdapter>&,
                                                                 const std::shared_ptr<Ice::ObjectAdapter>&,
                                                                 const std::string&,
                                                                 const Ice::Identity&,
                                                                 const std::string&);

    ICESTORM_SERVICE_API virtual std::shared_ptr<IceStorm::TopicManagerPrx> getTopicManager() const = 0;
};

};

#endif
