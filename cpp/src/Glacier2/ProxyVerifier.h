//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_VERIFIER_H
#define ICE_PROXY_VERIFIER_H

#include <Ice/Ice.h>
#include <vector>

namespace Glacier2
{

//
// Base class for proxy rule implementations.
//

class ProxyRule
{
public:

    virtual ~ProxyRule() {}

    //
    // Checks to see if the proxy passes.
    //
    virtual bool check(const Ice::ObjectPrx&) const = 0;
};

class ProxyVerifier : public IceUtil::Shared
{
public:

    ProxyVerifier(const Ice::CommunicatorPtr&);
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
typedef IceUtil::Handle<ProxyVerifier> ProxyVerifierPtr;

}
#endif
