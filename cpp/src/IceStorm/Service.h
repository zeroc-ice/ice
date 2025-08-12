// Copyright (c) ZeroC, Inc.

#ifndef ICESTORM_SERVICE_H
#define ICESTORM_SERVICE_H

#include "IceBox/IceBox.h"
#include "IceStorm/IceStorm.h"

// These IceStorm APIs are exported because they are used by IceGrid
#ifndef ICESTORM_SERVICE_API
#    if defined(ICESTORM_SERVICE_API_EXPORTS)
#        define ICESTORM_SERVICE_API ICE_DECLSPEC_EXPORT
#    else
#        define ICESTORM_SERVICE_API ICE_DECLSPEC_IMPORT
#    endif
#endif

#if defined(_MSC_VER) && !defined(ICESTORM_SERVICE_API_EXPORTS) && !defined(ICE_DISABLE_PRAGMA_COMMENT)
#    pragma comment(lib, ICE_LIBNAME("IceStormService")) // Automatically link with IceStormService[D].lib
#endif

// This API is internal to Ice, and should not be used by external applications.
namespace IceStormInternal
{
    class ICESTORM_SERVICE_API Service : public IceBox::Service
    {
    public:
        ~Service() override;

        static std::shared_ptr<Service> create(
            const Ice::CommunicatorPtr&,
            const Ice::ObjectAdapterPtr&,
            const Ice::ObjectAdapterPtr&,
            const Ice::Identity&);

        [[nodiscard]] virtual IceStorm::TopicManagerPrx getTopicManager() const = 0;
    };
}

#endif
