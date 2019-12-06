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
    virtual bool check(const std::shared_ptr<Ice::ObjectPrx>&) const = 0;
};

class ProxyVerifier final
{
public:

    ProxyVerifier(std::shared_ptr<Ice::Communicator>);
    ~ProxyVerifier();

    //
    // Verifies that the proxy is permissible under the configured
    // rules.
    //
    bool verify(const std::shared_ptr<Ice::ObjectPrx>&);

private:

    const std::shared_ptr<Ice::Communicator> _communicator;
    const int _traceLevel;

    std::vector<ProxyRule*> _acceptRules;
    std::vector<ProxyRule*> _rejectRules;
};

}
#endif
