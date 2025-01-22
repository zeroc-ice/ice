// Copyright (c) ZeroC, Inc.

#ifndef ICE_UNIQUE_REF_H
#define ICE_UNIQUE_REF_H

#include <cassert>

#ifdef __APPLE__

#    include <CoreFoundation/CoreFoundation.h>

namespace IceInternal
{
    //
    // UniqueRef helper class for CoreFoundation classes, comparable to std::unique_ptr
    //
    template<typename R> class UniqueRef
    {
    public:
        explicit UniqueRef(R ref = nullptr) : _ref(ref) {}

        ~UniqueRef()
        {
            if (_ref != nullptr)
            {
                CFRelease(_ref);
            }
        }

        UniqueRef(UniqueRef&) = delete;
        UniqueRef& operator=(UniqueRef&) = delete;

        R release()
        {
            R r = _ref;
            _ref = nullptr;
            return r;
        }

        void reset(R ref = nullptr)
        {
            //
            // Support "self-reset" for CF objects. This is useful if CF allocation methods return
            // the same object with an increased reference count.
            //
            // assert(ref == nullptr || ref != _ref);

            if (_ref != nullptr)
            {
                CFRelease(_ref);
            }
            _ref = ref;
        }

        void retain(R ref) { reset(ref ? (R)CFRetain(ref) : ref); }

        R& get() { return _ref; }

        [[nodiscard]] R get() const { return _ref; }

        operator bool() const { return _ref != nullptr; }

        void swap(UniqueRef& a) noexcept
        {
            R tmp = a._ref;
            a._ref = _ref;
            _ref = tmp;
        }

    private:
        R _ref;
    };
}

#endif

#endif
