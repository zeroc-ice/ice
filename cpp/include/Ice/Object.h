//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/IncomingAsyncF.h>
#include <Ice/SlicedDataF.h>
#include <Ice/Current.h>
#include <Ice/Format.h>

namespace Ice
{

class OutputStream;
class InputStream;

}

namespace IceInternal
{

class Incoming;
class Direct;
class GCVisitor;

}

namespace Ice
{

/** A default-initialized Current instance. */
ICE_API extern const Current emptyCurrent;

/**
 * Encapsulates details about a dispatch request.
 * \headerfile Ice/Ice.h
 */
class ICE_API Request
{
public:

    virtual ~Request();

    /**
     * Obtains the Current object associated with the request.
     * @return The Current object.
     */
    virtual const Current& getCurrent() = 0;
};

/**
 * The base class for servants.
 * \headerfile Ice/Ice.h
 */
class ICE_API Object
{
public:

    virtual ~Object() = default;

    /**
     * Tests whether this object supports a specific Slice interface.
     * @param s The type ID of the Slice interface to test against.
     * @param current The Current object for the invocation.
     * @return True if this object has the interface
     * specified by s or derives from the interface
     * specified by s.
     */
    virtual bool ice_isA(std::string s, const Current& current) const;
    /// \cond INTERNAL
    bool _iceD_ice_isA(IceInternal::Incoming&, const Current&);
    /// \endcond

    /**
     * Tests whether this object can be reached.
     * @param current The Current object for the invocation.
     */
    virtual void ice_ping(const Current& current) const;
    /// \cond INTERNAL
    bool _iceD_ice_ping(IceInternal::Incoming&, const Current&);
    /// \endcond

    /**
     * Returns the Slice type IDs of the interfaces supported by this object.
     * @param current The Current object for the invocation.
     * @return The Slice type IDs of the interfaces supported by this object, in base-to-derived
     * order. The first element of the returned array is always "::Ice::Object".
     */
    virtual std::vector< std::string> ice_ids(const Current& current) const;
    /// \cond INTERNAL
    bool _iceD_ice_ids(IceInternal::Incoming&, const Current&);
    /// \endcond

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     * @param current The Current object for the invocation.
     * @return The Slice type ID of the most-derived interface.
     */
    virtual std::string ice_id(const Current& current) const;
    /// \cond INTERNAL
    bool _iceD_ice_id(IceInternal::Incoming&, const Current&);
    /// \endcond

    /**
     * Obtains the Slice type ID of this type.
     * @return The return value is always "::Ice::Object".
     */
    static const std::string& ice_staticId();

    /**
     * Dispatches an invocation to a servant. This method is used by dispatch interceptors to forward an invocation
     * to a servant (or to another interceptor).
     * @param request The details of the invocation.
     * @param response A function that should return true if Ice should send the response to the client. A null
     * value is equivalent to a function that returns true.
     * @param error A function that should return true if Ice should send the exception to the client. A null
     * value is equivalent to a function that returns true.
     * @return True if the request completed synchronously, false if the request will be completed asynchronously.
     * @throws UserException A user exception that propagates out of this method will be marshaled as the result.
     */
    virtual bool ice_dispatch(Ice::Request& request,
                              std::function<bool()> response = nullptr,
                              std::function<bool(std::exception_ptr)> error = nullptr);

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond

    /**
     * Holds the results of a call to ice_invoke.
     */
    struct Ice_invokeResult
    {
        /**
         * Indicates whether the invocation resulted in success (true) or a user exception (false).
         */
        bool returnValue;

        /**
         * Holds an encapsulation of the encoded results. If returnValue is true, this contains the encoded
         * out parameters. If returnValue is false, this contains the encoded user exception.
         */
        std::vector<::Ice::Byte> outParams;
    };

protected:

    /// \cond INTERNAL
    static void _iceCheckMode(OperationMode, OperationMode);
    /// \endcond
};

/**
 * Base class for dynamic dispatch servants. A server application derives a concrete servant class
 * from Blobject that implements the ice_invoke method.
 * \headerfile Ice/Ice.h
 */
class ICE_API Blobject : public virtual Object
{
public:

    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
     * @param outEncaps An encapsulation containing the encoded results for the operation.
     * @param current The Current object for the invocation.
     * @return True if the operation completed successfully, in which case outEncaps contains
     * an encapsulation of the encoded results, or false if the operation raised a user exception,
     * in which case outEncaps contains an encapsulation of the encoded user exception.
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     */
    virtual bool ice_invoke(ICE_IN(std::vector<Byte>) inEncaps, std::vector<Byte>& outEncaps,
                            const Current& current) = 0;

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond
};

/**
 * Base class for dynamic dispatch servants that uses the array mapping. A server application
 * derives a concrete servant class from Blobject that implements the ice_invoke method.
 * \headerfile Ice/Ice.h
 */
class ICE_API BlobjectArray : public virtual Object
{
public:

    /**
     * Dispatch an incoming request.
     *
     * @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
     * @param outEncaps An encapsulation containing the encoded results for the operation.
     * @param current The Current object for the invocation.
     * @return True if the operation completed successfully, in which case outEncaps contains
     * an encapsulation of the encoded results, or false if the operation raised a user exception,
     * in which case outEncaps contains an encapsulation of the encoded user exception.
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     */
    virtual bool ice_invoke(ICE_IN(std::pair<const Byte*, const Byte*>) inEncaps, std::vector<Byte>& outEncaps,
                            const Current& current) = 0;

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond
};

/**
 * Base class for asynchronous dynamic dispatch servants. A server application derives a concrete
 * servant class from Blobject that implements the ice_invokeAsync method.
 * \headerfile Ice/Ice.h
 */
class ICE_API BlobjectAsync : public virtual Object
{
public:

    /**
     * Dispatch an incoming request asynchronously.
     *
     * @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
     * @param response A callback the implementation should invoke when the invocation completes
     * successfully or with a user exception. See the description of Blobject::ice_invoke for
     * the semantics.
     * @param error A callback the implementation should invoke when the invocation completes
     * with an exception.
     * @param current The Current object for the invocation.
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     */
    virtual void ice_invokeAsync(std::vector<Byte> inEncaps,
                                 std::function<void(bool, const std::vector<Byte>&)> response,
                                 std::function<void(std::exception_ptr)> error,
                                 const Current& current) = 0;

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond
};

/**
 * Base class for asynchronous dynamic dispatch servants that uses the array mapping. A server application
 * derives a concrete servant class from Blobject that implements the ice_invokeAsync method.
 * \headerfile Ice/Ice.h
 */
class ICE_API BlobjectArrayAsync : public virtual Object
{
public:

    /**
     * Dispatch an incoming request asynchronously.
     *
     * @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
     * @param response A callback the implementation should invoke when the invocation completes
     * successfully or with a user exception. See the description of Blobject::ice_invoke for
     * the semantics.
     * @param error A callback the implementation should invoke when the invocation completes
     * with an exception.
     * @param current The Current object for the invocation.
     * @throws UserException A user exception can be raised directly and the
     * run time will marshal it.
     */
    virtual void ice_invokeAsync(std::pair<const Byte*, const Byte*> inEncaps,
                                 std::function<void(bool, const std::pair<const Byte*, const Byte*>&)> response,
                                 std::function<void(std::exception_ptr)> error,
                                 const Current& current) = 0;

    /// \cond INTERNAL
    virtual bool _iceDispatch(IceInternal::Incoming&, const Current&);
    /// \endcond
};

}

#endif
