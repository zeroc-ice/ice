//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_SSL_CONTEXT_H
#define ICE_SSL_SSL_CONTEXT_H

#include "SSLConfig.h"

#ifdef ICE_SSL_OPENSSL

namespace Ice::SSL
{
    /**
     * The SslContext class encapsulates an SSL_CTX pointer, and keeps it alive until the SslContext object is
     * destroyed or released.
     */
    class SslContext
    {
    public:
        /**
         * Construct an SslContext object with the given SSL_CTX pointer. The reference count of the SSL_CTX object is
         * incremented.
         */
        SslContext(SSL_CTX* ctx) : _ctx(ctx) { SSL_CTX_up_ref(_ctx); }

        /**
         * Destroy the SslContext object and release the SSL_CTX object.
         */
        ~SslContext()
        {
            if (_ctx)
            {
                SSL_CTX_free(_ctx);
                _ctx = nullptr;
            }
        }

        /**
         * Return the SSL_CTX pointer and release ownership of the pointer. The caller is responsible for freeing the
         * SSL_CTX object.
         */
        SSL_CTX* release()
        {
            SSL_CTX* ctx = _ctx;
            _ctx = nullptr;
            return ctx;
        }

    private:
        SSL_CTX* _ctx;
    };
}
#endif

#endif
