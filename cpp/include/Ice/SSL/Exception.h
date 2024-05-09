//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_EXCEPTION_H
#define ICE_SSL_EXCEPTION_H

#include "../Config.h"
#include "../Exception.h"

#include <string>

namespace Ice::SSL
{
    /**
     * Thrown if the certificate cannot be read.
     */
    class ICE_API CertificateReadException : public Ice::Exception
    {
    public:
        using Ice::Exception::Exception;

        CertificateReadException(const char*, int, std::string) noexcept;

        std::string ice_id() const override;

        /** The reason for the exception. */
        std::string reason;

    private:
        static const char* _name;
    };

    /**
     * Thrown if the certificate cannot be encoded.
     */
    class ICE_API CertificateEncodingException : public Ice::Exception
    {
    public:
        using Ice::Exception::Exception;

        CertificateEncodingException(const char*, int, std::string) noexcept;

        std::string ice_id() const override;

        /** The reason for the exception. */
        std::string reason;

    private:
        static const char* _name;
    };

    /**
     * This exception is thrown if a distinguished name cannot be parsed.
     */
    class ICE_API ParseException : public Ice::Exception
    {
    public:
        using Ice::Exception::Exception;

        ParseException(const char*, int, std::string) noexcept;

        std::string ice_id() const override;

        /** The reason for the exception. */
        std::string reason;

    private:
        static const char* _name;
    };
}

#endif
