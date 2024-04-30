//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_SSL_SSL_CERTIFICATE_CHAIN_H
#define ICE_SSL_SSL_CERTIFICATE_CHAIN_H

#include "SSLConfig.h"

#ifdef ICE_USE_SECURE_TRANSPORT

namespace Ice::SSL
{
    /**
     * The CertificateChain class encapsulates a CFArrayRef that holds a Secure Transport certificate chain, and
     * keeps it alive until the CertificateChain object is destroyed or released.
     */
    class CertificateChain
    {
    public:
        /**
         * Construct a ßCertificateChain object with the given CFArrayRef. The reference count of the CFArrayRef
         * object is incremented.
         */
        CertificateChain(CFArrayRef certificateChain) : _certificateChain(certificateChain)
        {
            if (_certificateChain)
            {
                CFRetain(_certificateChain);
            }
        }

        /**
         * Destroy the CertificateChain object and release the CFArrayRef object.
         */
        ~CertificateChain()
        {
            if (_certificateChain)
            {
                CFRelease(_certificateChain);
                _certificateChain = nullptr;
            }
        }

        /**
         * Return the CFArrayRef holding the certificate chain and release ownership of the object. The caller is
         * responsible for releasing the object.
         */
        CFArrayRef release()
        {
            CFArrayRef certificateChain = _certificateChain;
            _certificateChain = nullptr;
            return certificateChain;
        }

    private:
        CFArrayRef _certificateChain;
    };
}
#endif

#endif
