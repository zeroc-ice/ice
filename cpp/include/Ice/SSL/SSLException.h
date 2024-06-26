//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
        CertificateReadException(const char*, int, std::string) noexcept;

        const char* ice_id() const noexcept override;

        /** The reason for the exception. */
        std::string reason;
    };

    /**
     * Thrown if the certificate cannot be encoded.
     */
    class ICE_API CertificateEncodingException final : public Ice::LocalException
    {
    public:
        CertificateEncodingException(const char*, int, std::string) noexcept;

        const char* ice_id() const noexcept override;

        /** The reason for the exception. */
        std::string reason;
    };
}

#endif
