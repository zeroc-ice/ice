// Copyright (c) ZeroC, Inc.

#ifndef ICE_SSL_EXCEPTION_H
#define ICE_SSL_EXCEPTION_H

#include "../Config.h"
#include "../LocalException.h"

#include <string>

namespace Ice::SSL
{
    /**
     * Thrown if the certificate cannot be read.
     */
    class ICE_API CertificateReadException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept final;
    };

    /**
     * Thrown if the certificate cannot be encoded.
     */
    class ICE_API CertificateEncodingException final : public Ice::LocalException
    {
    public:
        using Ice::LocalException::LocalException;

        [[nodiscard]] const char* ice_id() const noexcept override;
    };
}

#endif
