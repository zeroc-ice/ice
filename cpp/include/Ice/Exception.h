//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <Ice/Config.h>
#include <Ice/Format.h>
#include <Ice/ObjectF.h>
#include <Ice/ValueF.h>

namespace Ice
{
    class OutputStream;
    class InputStream;

    using Exception = IceUtil::Exception;

    /**
     * Base class for all Ice user exceptions.
     * \headerfile Ice/Ice.h
     */
    class ICE_API UserException : public IceUtil::Exception
    {
    public:
        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static std::string_view ice_staticId() noexcept;

        /**
         * Throws this exception.
         */
        virtual void ice_throw() const = 0;

        /// \cond STREAM
        virtual void _write(::Ice::OutputStream*) const;
        virtual void _read(::Ice::InputStream*);

        virtual bool _usesClasses() const;
        /// \endcond

    protected:
        /// \cond STREAM
        virtual void _writeImpl(::Ice::OutputStream*) const {}
        virtual void _readImpl(::Ice::InputStream*) {}
        /// \endcond
    };
}

namespace IceInternal
{
    namespace Ex
    {
        ICE_API void throwUOE(const ::std::string&, const std::shared_ptr<Ice::Value>&);
        ICE_API void throwMemoryLimitException(const char*, int, size_t, size_t);
        ICE_API void throwMarshalException(const char*, int, const std::string&);

    }
}

#endif
