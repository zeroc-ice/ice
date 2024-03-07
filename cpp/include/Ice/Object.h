//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_OBJECT_H
#define ICE_OBJECT_H

#include <Ice/ObjectF.h>
#include <Ice/ProxyF.h>
#include <Ice/SlicedDataF.h>
#include <Ice/Current.h>
#include <Ice/Format.h>

namespace IceInternal
{

    class Incoming;
    class Direct;

}

namespace Ice
{

    /** A default-initialized Current instance. */
    ICE_API extern const Current emptyCurrent;

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
        bool _iceD_ice_isA(IceInternal::Incoming&);
        /// \endcond

        /**
         * Tests whether this object can be reached.
         * @param current The Current object for the invocation.
         */
        virtual void ice_ping(const Current& current) const;
        /// \cond INTERNAL
        bool _iceD_ice_ping(IceInternal::Incoming&);
        /// \endcond

        /**
         * Returns the Slice type IDs of the interfaces supported by this object.
         * @param current The Current object for the invocation.
         * @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
         */
        virtual std::vector<std::string> ice_ids(const Current& current) const;
        /// \cond INTERNAL
        bool _iceD_ice_ids(IceInternal::Incoming&);
        /// \endcond

        /**
         * Returns the Slice type ID of the most-derived interface supported by this object.
         * @param current The Current object for the invocation.
         * @return The Slice type ID of the most-derived interface.
         */
        virtual std::string ice_id(const Current& current) const;
        /// \cond INTERNAL
        bool _iceD_ice_id(IceInternal::Incoming&);
        /// \endcond

        /**
         * Obtains the Slice type ID of this type.
         * @return The return value is always "::Ice::Object".
         */
        static std::string_view ice_staticId();

        /// \cond INTERNAL
        virtual bool _iceDispatch(IceInternal::Incoming&);
        /// \endcond

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
        virtual bool ice_invoke(
            std::vector<std::uint8_t> inEncaps,
            std::vector<std::uint8_t>& outEncaps,
            const Current& current) = 0;

        /// \cond INTERNAL
        bool _iceDispatch(IceInternal::Incoming&) final;
        /// \endcond
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
            std::pair<const std::uint8_t*, const std::uint8_t*> inEncaps,
            std::vector<std::uint8_t>& outEncaps,
            const Current& current) = 0;

        /// \cond INTERNAL
        bool _iceDispatch(IceInternal::Incoming&) final;
        /// \endcond
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
            std::vector<std::uint8_t> inEncaps,
            std::function<void(bool, const std::vector<std::uint8_t>&)> response,
            std::function<void(std::exception_ptr)> error,
            const Current& current) = 0;

        /// \cond INTERNAL
        bool _iceDispatch(IceInternal::Incoming&) final;
        /// \endcond
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
            std::pair<const std::uint8_t*, const std::uint8_t*> inEncaps,
            std::function<void(bool, const std::pair<const std::uint8_t*, const std::uint8_t*>&)> response,
            std::function<void(std::exception_ptr)> error,
            const Current& current) = 0;
        /// \cond INTERNAL
        bool _iceDispatch(IceInternal::Incoming&) final;
        /// \endcond
    };

}

#endif
