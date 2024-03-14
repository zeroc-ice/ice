//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EXCEPTION_HELPERS_H
#define ICE_EXCEPTION_HELPERS_H

#include <Ice/InputStream.h>
#include <Ice/OutputStream.h>

namespace Ice
{
    /**
     * Helper template for the implementation of Ice::UserException. It implements ice_id and ice_throw.
     * \headerfile Ice/Ice.h
     */
    template<typename T, typename B> class UserExceptionHelper : public B
    {
    public:
        using B::B;

        std::string ice_id() const override { return std::string{T::ice_staticId()}; }

        void ice_throw() const override { throw static_cast<const T&>(*this); }

    protected:
        /// \cond STREAM
        virtual void _writeImpl(Ice::OutputStream* os) const override
        {
            os->startSlice(T::ice_staticId(), -1, std::is_same<B, Ice::LocalException>::value ? true : false);
            Ice::StreamWriter<T, Ice::OutputStream>::write(os, static_cast<const T&>(*this));
            os->endSlice();
            B::_writeImpl(os);
        }

        virtual void _readImpl(Ice::InputStream* is) override
        {
            is->startSlice();
            Ice::StreamReader<T, ::Ice::InputStream>::read(is, static_cast<T&>(*this));
            is->endSlice();
            B::_readImpl(is);
        }
        /// \endcond
    };
}

#endif
