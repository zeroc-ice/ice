// Copyright (c) ZeroC, Inc.

#ifndef ICE_PROXY_VERIFIER_H
#define ICE_PROXY_VERIFIER_H

#include "Ice/Ice.h"
#include <vector>

namespace Glacier2
{
    //
    // Base class for proxy rule implementations.
    //

    class ProxyRule
    {
    public:
        virtual ~ProxyRule() = default;

        //
        // Checks to see if the proxy passes.
        //
        [[nodiscard]] virtual bool check(const Ice::ObjectPrx&) const = 0;
    };

    class ProxyVerifier final
    {
    public:
        ProxyVerifier(Ice::CommunicatorPtr);
        ~ProxyVerifier();

        //
        // Verifies that the proxy is permissible under the configured
        // rules.
        //
        bool verify(const Ice::ObjectPrx&);

    private:
        const Ice::CommunicatorPtr _communicator;
        const int _traceLevel;

        std::vector<ProxyRule*> _acceptRules;
        std::vector<ProxyRule*> _rejectRules;
    };
}
#endif
