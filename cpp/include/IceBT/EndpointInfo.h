//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __IceBT_EndpointInfo_h__
#define __IceBT_EndpointInfo_h__

#include <IceUtil/PushDisableWarnings.h>
#include <Ice/ProxyF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/Comparable.h>
#include <optional>
#include <Ice/Endpoint.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICEBT_API
#    if defined(ICE_STATIC_LIBS)
#        define ICEBT_API /**/
#    elif defined(ICEBT_API_EXPORTS)
#        define ICEBT_API ICE_DECLSPEC_EXPORT
#    else
#        define ICEBT_API ICE_DECLSPEC_IMPORT
#    endif
#endif

namespace IceBT
{
    class EndpointInfo;
}

namespace IceBT
{
    /**
     * Provides access to Bluetooth endpoint information.
     * \headerfile IceBT/IceBT.h
     */
    class ICE_CLASS(ICEBT_API) EndpointInfo : public Ice::EndpointInfo
    {
    public:
        ICE_MEMBER(ICEBT_API) virtual ~EndpointInfo();

        EndpointInfo() = default;

        EndpointInfo(const EndpointInfo&) = default;
        EndpointInfo(EndpointInfo&&) = default;
        EndpointInfo& operator=(const EndpointInfo&) = default;
        EndpointInfo& operator=(EndpointInfo&&) = default;

        /**
         * One-shot constructor to initialize all data members.
         * @param underlying The information of the underlying endpoint or null if there's no underlying endpoint.
         * @param timeout The timeout for the endpoint in milliseconds.
         * @param compress Specifies whether or not compression should be used if available when using this endpoint.
         * @param addr The address configured with the endpoint.
         * @param uuid The UUID configured with the endpoint.
         */
        EndpointInfo(
            const std::shared_ptr<Ice::EndpointInfo>& underlying,
            int timeout,
            bool compress,
            const std::string& addr,
            const std::string& uuid)
            : Ice::EndpointInfo(underlying, timeout, compress),
              addr(addr),
              uuid(uuid)
        {
        }

        /**
         * The address configured with the endpoint.
         */
        std::string addr;
        /**
         * The UUID configured with the endpoint.
         */
        std::string uuid;
    };
}

/// \cond INTERNAL
namespace IceBT
{
    using EndpointInfoPtr = std::shared_ptr<EndpointInfo>;
}
/// \endcond

#include <IceUtil/PopDisableWarnings.h>
#endif
