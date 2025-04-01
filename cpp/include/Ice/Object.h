// Copyright (c) ZeroC, Inc.

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include "IncomingRequest.h"
#include "ObjectF.h"
#include "OutgoingResponse.h"

#include <functional>
#include <string_view>

namespace Ice
{
    /// The base class for servants.
    /// @remark Object is a stateless polymorphic base class. Its copy constructor, move constructor, copy assignment
    /// operator and move assignment operator are all deleted to prevent accidental slicing. Derived classes can
    /// define these constructors and assignment operators to reenable copying, moving and slicing.
    /// @headerfile Ice/Ice.h
    class ICE_API Object
    {
    public:
        /// Default constructor.
        Object() noexcept = default;

        virtual ~Object() = default;

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;
        Object(Object&&) = delete;
        Object& operator=(Object&&) = delete;

        /// Dispatches an incoming request and returns the corresponding outgoing response.
        /// @param request The incoming request.
        /// @param sendResponse A callback that the implementation calls to return the response. @p sendResponse does
        /// not throw any exception and any @p sendResponse wrapper must not throw any exception. @p sendResponse can be
        /// called by the thread that called dispatch (the "dispatch thread") or by another thread. The implementation
        /// must call @p sendResponse exactly once or throw an exception.
        /// @remark Calling @p sendResponse can be thought as returning the outgoing response. Just like when you return
        /// a value from a remote operation, you can only return it once and you don't know if the client receives this
        /// value. In practice, the Ice-provided @p sendResponse attempts to send the response to the client
        /// synchronously, but may send it asynchronously. It can also silently fail to send the response back to the
        /// client. This function is the main building block for the Ice dispatch pipeline. The implementation provided
        /// by the base class (Object) dispatches incoming requests to the four `Object` operations (`ice_isA`,
        /// `ice_ping`, `ice_ids` and `ice_id`), and throws OperationNotExistException for all other operations. This
        /// base implementation is trivial and should be overridden and fully replaced by all derived classes.
        virtual void dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse);

        /// Tests whether this object supports a specific Slice interface.
        /// @param typeId The type ID of the Slice interface to test against.
        /// @param current The Current object of the incoming request.
        /// @return `true` if this object implements the Slice interface specified by @p typeId or implements a derived
        /// interface, `false` otherwise.
        [[nodiscard]] virtual bool ice_isA(std::string typeId, const Current& current) const;

        /// @private
        void _iceD_ice_isA(IncomingRequest&, std::function<void(OutgoingResponse)>);

        /// Tests whether this object can be reached.
        /// @param current The Current object of the incoming request.
        virtual void ice_ping(const Current& current) const;

        /// @private
        void _iceD_ice_ping(IncomingRequest&, std::function<void(OutgoingResponse)>);

        /// Gets the Slice interfaces supported by this object as a list of type IDs.
        /// @param current The Current object of the incoming request.
        /// @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
        [[nodiscard]] virtual std::vector<std::string> ice_ids(const Current& current) const;

        /// @private
        void _iceD_ice_ids(IncomingRequest&, std::function<void(OutgoingResponse)>);

        /// Gets the type ID of the most-derived Slice interface supported by this object.
        /// @param current The Current object of the incoming request.
        /// @return The Slice type ID of the most-derived interface.
        [[nodiscard]] virtual std::string ice_id(const Current& current) const;

        /// @private
        void _iceD_ice_id(IncomingRequest&, std::function<void(OutgoingResponse)>);

        /// Gets the type ID of the associated Slice interface.
        /// @return The string `"::Ice::Object"`.
        static const char* ice_staticId() noexcept;
    };

    /// Base class for dynamic dispatch servants.
    /// @remark This class is provided for backward compatibility. You should consider deriving directly from Object
    /// and overriding the Object::dispatch function.
    /// @headerfile Ice/Ice.h
    class ICE_API Blobject : public Object
    {
    public:
        /// Dispatches an incoming request.
        /// @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
        /// @param outEncaps An encapsulation containing the encoded result for the operation.
        /// @param current The Current object of the incoming request.
        /// @return `true` if the dispatch completes successfully, `false` if the dispatch completes with a user
        /// exception encoded in @p outEncaps.
        virtual bool
        ice_invoke(std::vector<std::byte> inEncaps, std::vector<std::byte>& outEncaps, const Current& current) = 0;

        /// @private
        void dispatch(IncomingRequest& request, std::function<void(OutgoingResponse)> sendResponse) final;
    };

    /// Base class for dynamic dispatch servants that uses the array mapping.
    /// @remark This class is provided for backward compatibility. You should consider deriving directly from Object
    /// and overriding the Object::dispatch function.
    /// @headerfile Ice/Ice.h
    class ICE_API BlobjectArray : public Object
    {
    public:
        /// @copydoc Blobject::ice_invoke
        virtual bool ice_invoke(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::vector<std::byte>& outEncaps,
            const Current& current) = 0;

        /// @private
        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };

    /// Base class for asynchronous dynamic dispatch servants.
    /// @remark This class is provided for backward compatibility. You should consider deriving directly from Object
    /// and overriding the Object::dispatch function.
    /// @headerfile Ice/Ice.h
    class ICE_API BlobjectAsync : public Object
    {
    public:
        /// Dispatches an incoming request asynchronously.
        /// @param inEncaps An encapsulation containing the encoded in-parameters for the operation.
        /// @param response The response callback. It accepts:
        /// - `returnValue` `true` if the operation completed successfully, `false` if it completed with a user
        ///   exception encoded in @p outEncaps.
        /// - `outEncaps` An encapsulation containing the encoded result.
        /// @param exception The exception callback.
        /// @param current The Current object of the incoming request.
        virtual void ice_invokeAsync(
            std::vector<std::byte> inEncaps,
            std::function<void(bool, const std::vector<std::byte>&)> response,
            std::function<void(std::exception_ptr)> exception,
            const Current& current) = 0;

        /// @private
        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };

    /// Base class for asynchronous dynamic dispatch servants that uses the array mapping.
    /// @remark This class is provided for backward compatibility. You should consider deriving directly from Object
    /// and overriding the Object::dispatch function.
    /// @headerfile Ice/Ice.h
    class ICE_API BlobjectArrayAsync : public Object
    {
    public:
        /// @copydoc BlobjectAsync::ice_invokeAsync
        virtual void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*> inEncaps,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)> response,
            std::function<void(std::exception_ptr)> exception,
            const Current& current) = 0;

        /// @private
        void dispatch(IncomingRequest&, std::function<void(OutgoingResponse)>) final;
    };
}

#endif
