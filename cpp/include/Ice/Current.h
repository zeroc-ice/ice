//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef __Ice_Current_h__
#define __Ice_Current_h__

#include <Ice/ProxyF.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>
#include <Ice/Exception.h>
#include <Ice/LocalObject.h>
#include <Ice/StreamHelpers.h>
#include <Ice/Comparable.h>
#include <IceUtil/ScopedArray.h>
#include <Ice/Optional.h>
#include <Ice/ObjectAdapterF.h>
#include <Ice/ConnectionF.h>
#include <Ice/Identity.h>
#include <Ice/Version.h>
#include <Ice/Context.h>
#include <Ice/OperationMode.h>
#include <IceUtil/UndefSysMacros.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

/**
 * Information about the current method invocation for servers. Each
 * operation on the server has a <code>Current</code> as its implicit final
 * parameter. <code>Current</code> is mostly used for Ice services. Most
 * applications ignore this parameter.
 * \headerfile Ice/Ice.h
 */
struct Current
{
    /**
     * The object adapter.
     */
    ::std::shared_ptr<::Ice::ObjectAdapter> adapter;
    /**
     * Information about the connection over which the current method
     * invocation was received. If the invocation is direct due to
     * collocation optimization, this value is set to null.
     */
    ::std::shared_ptr<::Ice::Connection> con;
    /**
     * The Ice object identity.
     */
    ::Ice::Identity id;
    /**
     * The facet.
     */
    ::std::string facet;
    /**
     * The operation name.
     */
    ::std::string operation;
    /**
     * The mode of the operation.
     */
    ::Ice::OperationMode mode;
    /**
     * The request context, as received from the client.
     */
    ::Ice::Context ctx;
    /**
     * The request id unless oneway (0).
     */
    int requestId;
    /**
     * The encoding version used to encode the input and output parameters.
     */
    ::Ice::EncodingVersion encoding;

    /**
     * Obtains a tuple containing all of the struct's data members.
     * @return The data members in a tuple.
     */
    std::tuple<const ::std::shared_ptr<::Ice::ObjectAdapter>&, const ::std::shared_ptr<::Ice::Connection>&, const ::Ice::Identity&, const ::std::string&, const ::std::string&, const ::Ice::OperationMode&, const ::Ice::Context&, const int&, const ::Ice::EncodingVersion&> ice_tuple() const
    {
        return std::tie(adapter, con, id, facet, operation, mode, ctx, requestId, encoding);
    }
};

using Ice::operator<;
using Ice::operator<=;
using Ice::operator>;
using Ice::operator>=;
using Ice::operator==;
using Ice::operator!=;

}

#else // C++98 mapping

namespace Ice
{

/**
 * Information about the current method invocation for servers. Each
 * operation on the server has a <code>Current</code> as its implicit final
 * parameter. <code>Current</code> is mostly used for Ice services. Most
 * applications ignore this parameter.
 * \headerfile Ice/Ice.h
 */
struct Current
{
    /**
     * The object adapter.
     */
    ::Ice::ObjectAdapterPtr adapter;
    /**
     * Information about the connection over which the current method
     * invocation was received. If the invocation is direct due to
     * collocation optimization, this value is set to null.
     */
    ::Ice::ConnectionPtr con;
    /**
     * The Ice object identity.
     */
    ::Ice::Identity id;
    /**
     * The facet.
     */
    ::std::string facet;
    /**
     * The operation name.
     */
    ::std::string operation;
    /**
     * The mode of the operation.
     */
    ::Ice::OperationMode mode;
    /**
     * The request context, as received from the client.
     */
    ::Ice::Context ctx;
    /**
     * The request id unless oneway (0).
     */
    ::Ice::Int requestId;
    /**
     * The encoding version used to encode the input and output parameters.
     */
    ::Ice::EncodingVersion encoding;
};

}

#endif

#endif
