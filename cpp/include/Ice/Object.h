//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include "IncomingRequest.h"
#include "ObjectF.h"
#include "OutgoingResponse.h"

#include <functional>
#include <string_view>

namespace Ice
{
    /**
     * The base class for servants.
     * @remarks Object is a stateless polymorphic base class. Its copy constructor, move constructor, copy assignment
     * operator and move assignment operator are all deleted to prevent accidental slicing. Derived classes can
     * define these constructors and assignment operators to reenable copying, moving and slicing.
     * \headerfile Ice/Ice.h
     */
    class ICE_API Object
    {
    public:
        Object() noexcept = default;
        virtual ~Object() = default;

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) = delete;
        Object& operator=(Object&&) = delete;

        /**
         * Dispatches an incoming request and returns the corresponding outgoing response.
         * @param request The incoming request.
         * @param sendResponse A callback that the implementation calls to return the response. sendResponse does not
         * throw any exception and any sendResponse wrapper must not throw any exception. sendResponse can be called by
         * the thread that called dispatch (the "dispatch thread") or by another thread. The implementation must call
         * sendResponse exactly once or throw an exception.
         * @remarks Calling sendResponse can be thought as returning the outgoing response. Just like when you return a
         * value from a remote operation, you can only return it once and you don't know if the client receives this
         * value. In practice, the Ice-provided sendResponse attempts to send the response to the client synchronously,
         * but may send it asynchronously. It can also silently fail to send the response back to the client.
         * This function is the main building block for the Ice dispatch pipeline. The implementation provided by the
         * base Object class dispatches incoming requests to the four Object operations (ice_isA, ice_ping, ice_ids and
         * ice_id), and throws OperationNotExistException for all other operations. This base implementation is trivial
         * and should be overridden and fully replaced by all derived classes.
         */
        virtual void dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse);

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
        void _iceD_ice_isA(IncomingRequest&, std::function<void(OutgoingResponse)>);
        /// \endcond

        /**
         * Tests whether this object can be reached.
         * @param current The Current object for the invocation.
         */
        virtual void ice_ping(const Current& current) const;
        /// \cond INTERNAL
        void _iceD_ice_ping(IncomingRequest&, std::function<void(OutgoingResponse)>);
        /// \endcond

        /**
         * Gets the Slice type IDs of the interfaces supported by this object.
         * @param current The Current object for the invocation.
         * @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
         */
        [[nodiscard]] virtual std::vector<std::string> ice_ids(const Current& current) const;
        /// \cond INTERNAL
        void _iceD_ice_ids(IncomingRequest&, std::function<void(OutgoingResponse)>);
        /// \endcond

        /**
         * Gets the Slice type ID of the most-derived interface supported by this object.
         * @param current The Current object for the invocation.
         * @return The Slice type ID of the most-derived interface.
         */
        [[nodiscard]] virtual std::string ice_id(const Current& current) const;
        /// \cond INTERNAL
        void _iceD_ice_id(IncomingRequest&, std::function<void(OutgoingResponse)>);
        /// \endcond

        /**
         * Gets the Slice type ID of this type.
         * @return The return value is always "::Ice::Object".
         */
        static const char* ice_staticId() noexcept;

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
    class ICE_API Blobject : public Object
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
        virtual bool
        ice_invoke(std::vector<std::byte> inEncaps, std::vector<std::byte>& outEncaps, const Current& current) = 0;

        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };

    /**
     * Base class for dynamic dispatch servants that uses the array mapping. A server application
     * derives a concrete servant class from Blobject that implements the ice_invoke method.
     * \headerfile Ice/Ice.h
     */
    class ICE_API BlobjectArray : public Object
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
        virtual bool ice_invoke(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::vector<std::byte>& outEncaps,
            const Current& current) = 0;

        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };

    /**
     * Base class for asynchronous dynamic dispatch servants. A server application derives a concrete
     * servant class from Blobject that implements the ice_invokeAsync method.
     * \headerfile Ice/Ice.h
     */
    class ICE_API BlobjectAsync : public Object
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
        virtual void ice_invokeAsync(
            std::vector<std::byte> inEncaps,
            std::function<void(bool, const std::vector<std::byte>&)> response,
            std::function<void(std::exception_ptr)> error,
            const Current& current) = 0;

        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };

    /**
     * Base class for asynchronous dynamic dispatch servants that uses the array mapping. A server application
     * derives a concrete servant class from Blobject that implements the ice_invokeAsync method.
     * \headerfile Ice/Ice.h
     */
    class ICE_API BlobjectArrayAsync : public Object
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
        virtual void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)> response,
            std::function<void(std::exception_ptr)> error,
            const Current& current) = 0;

        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };
}

#endif
