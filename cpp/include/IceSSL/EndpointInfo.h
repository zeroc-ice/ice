//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICESSL_ENDPOINT_INFO_H
#define ICESSL_ENDPOINT_INFO_H

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>
#include <Ice/Endpoint.h>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace IceSSL
{

/**
 * Provides access to an SSL endpoint information.
 * \headerfile IceSSL/IceSSL.h
 */
class ICE_CLASS(ICESSL_API) EndpointInfo : public ::Ice::EndpointInfo
{
public:

    ICE_MEMBER(ICESSL_API) virtual ~EndpointInfo();

    EndpointInfo() = default;

    EndpointInfo(const EndpointInfo&) = default;
    EndpointInfo(EndpointInfo&&) = default;
    EndpointInfo& operator=(const EndpointInfo&) = default;
    EndpointInfo& operator=(EndpointInfo&&) = default;

    /**
        * One-shot constructor to initialize all data members.
        * @param underlying The information of the underyling endpoint of null if there's no underlying endpoint.
        * @param timeout The timeout for the endpoint in milliseconds.
        * @param compress Specifies whether or not compression should be used if available when using this endpoint.
        */
    EndpointInfo(const ::std::shared_ptr<::Ice::EndpointInfo> & underlying, int timeout, bool compress) :
        ::Ice::EndpointInfo(::std::move(underlying), timeout, compress)
    {
    }
};

}

/// \cond INTERNAL
namespace IceSSL
{

using EndpointInfoPtr = ::std::shared_ptr<EndpointInfo>;

}
/// \endcond

#else // C++98 mapping

namespace IceSSL
{

class EndpointInfo;
/// \cond INTERNAL
ICESSL_API ::Ice::LocalObject* upCast(EndpointInfo*);
/// \endcond
typedef ::IceInternal::Handle< EndpointInfo> EndpointInfoPtr;

}

namespace IceSSL
{

/**
 * Provides access to an SSL endpoint information.
 * \headerfile IceSSL/IceSSL.h
 */
class ICESSL_API EndpointInfo : public ::Ice::EndpointInfo
{
public:

    typedef EndpointInfoPtr PointerType;

    virtual ~EndpointInfo();

    EndpointInfo()
    {
    }

    /**
     * One-shot constructor to initialize all data members.
     * @param underlying The information of the underyling endpoint of null if there's no underlying endpoint.
     * @param timeout The timeout for the endpoint in milliseconds.
     * @param compress Specifies whether or not compression should be used if available when using this endpoint.
     */
    EndpointInfo(const ::Ice::EndpointInfoPtr& underlying, ::Ice::Int timeout, bool compress) :
        ::Ice::EndpointInfo(underlying, timeout, compress)
    {
    }
};

/// \cond INTERNAL
inline bool operator==(const EndpointInfo& lhs, const EndpointInfo& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) == static_cast<const ::Ice::LocalObject&>(rhs);
}

inline bool operator<(const EndpointInfo& lhs, const EndpointInfo& rhs)
{
    return static_cast<const ::Ice::LocalObject&>(lhs) < static_cast<const ::Ice::LocalObject&>(rhs);
}
/// \endcond

}

#endif

#endif
