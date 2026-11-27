// Copyright (c) ZeroC, Inc.

#ifndef ICE_APPLE_OBJECT_REF_H
#define ICE_APPLE_OBJECT_REF_H

#include <Network/Network.h>
#include <dispatch/dispatch.h>

namespace IceInternal
{
    // Ownership of the objects of Network.framework (nw_*), of the Security protocol layer (sec_*) and of dispatch,
    // in the spirit of UniqueRef for Core Foundation objects.
    //
    // The wrapper releases the reference it owns and does nothing else. Cancelling a connection, a listener or a
    // dispatch source, and closing a socket, are separate operations that the owner performs explicitly: releasing
    // a reference does not stop the callbacks of the object, and the descriptor a dispatch source monitors can only
    // be closed once the source's cancellation handler ran.
    //
    // Adopting takes over the owned reference a create or copy function returns; retaining takes an additional
    // reference to a borrowed object, for example the parameter of a handler block.
    struct NetworkObjectTraits
    {
        template<typename T> static void retain(T object) { nw_retain(object); }
        template<typename T> static void release(T object) { nw_release(object); }
    };

    struct SecObjectTraits
    {
        template<typename T> static void retain(T object) { sec_retain(object); }
        template<typename T> static void release(T object) { sec_release(object); }
    };

    struct DispatchObjectTraits
    {
        template<typename T> static void retain(T object) { dispatch_retain(object); }
        template<typename T> static void release(T object) { dispatch_release(object); }
    };

    template<typename T, typename Traits> class ObjectRef
    {
    public:
        ObjectRef() = default;

        // Takes over an owned reference.
        static ObjectRef adopt(T object) { return ObjectRef(object); }

        // Takes an additional reference to a borrowed object.
        static ObjectRef retain(T object)
        {
            if (object)
            {
                Traits::retain(object);
            }
            return ObjectRef(object);
        }

        ObjectRef(ObjectRef&& other) noexcept : _object(other._object) { other._object = nullptr; }

        ObjectRef& operator=(ObjectRef&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                _object = other._object;
                other._object = nullptr;
            }
            return *this;
        }

        ObjectRef(const ObjectRef&) = delete;
        ObjectRef& operator=(const ObjectRef&) = delete;

        ~ObjectRef() { reset(); }

        [[nodiscard]] T get() const { return _object; }

        explicit operator bool() const { return _object != nullptr; }

        // Releases the owned reference, if any.
        void reset()
        {
            if (_object)
            {
                Traits::release(_object);
                _object = nullptr;
            }
        }

    private:
        explicit ObjectRef(T object) : _object(object) {}

        T _object{nullptr};
    };

    template<typename T> using NetworkRef = ObjectRef<T, NetworkObjectTraits>;
    template<typename T> using SecRef = ObjectRef<T, SecObjectTraits>;
    template<typename T> using DispatchRef = ObjectRef<T, DispatchObjectTraits>;
}

#endif
