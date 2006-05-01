// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ProxyVerifier.h>

#ifdef _WIN32
#   include <pcreposix.h>
#else
#   include <sys/types.h>
#   include <regex.h>
#endif

#include <string>

using namespace std;
using namespace Ice;

//
// Proxy rule implementations.
//
namespace Glacier2
{

//
// RegexRule returns true if the proxy matches the configured regular
// expression.
//
class RegexRule : public ProxyRule
{
public:
    RegexRule(const CommunicatorPtr& communicator, const string& regex, int traceLevel) :
	_communicator(communicator),
	_expr(regex),
	_traceLevel(traceLevel)
    {
	int result;
	if((result = regcomp(&_r, regex.c_str(), REG_EXTENDED | REG_NOSUB)) != 0)
	{
	    int errbuf = regerror(result, &_r, 0, 0);
	    auto_ptr<char> buf(new char[errbuf + 1]);
	    result = regerror(result, &_r, buf.get(), errbuf + 1);
	    regfree(&_r);
	    string msg = "Unable to compile regular expression: ";
	    msg += buf.get();
	    InitializationException ex(__FILE__, __LINE__);
	    ex.reason = msg;
	    throw ex;
	}
    }

    ~RegexRule()
    {
	regfree(&_r);
    }

    bool
    check(const ObjectPrx& p) const
    {
	string s = _communicator->proxyToString(p);
	bool result = (regexec(&_r, s.c_str(), 0, 0, 0) == 0);
	if(_traceLevel >= 3)
	{
	    Trace out(_communicator->getLogger(), "Glacier2");
	    out << s << (result ? " matched " : " didn't match ") << _expr << "\n";
	}
	return result;
    }

private:
    const CommunicatorPtr _communicator;
    const string _expr;
    const int _traceLevel;
    regex_t _r;
};

//
// MaxEndpointsRule returns 'true' if the proxy exceeds the configured
// number of endpoints.
//
class MaxEndpointsRule : public ProxyRule
{
public:
    MaxEndpointsRule(const CommunicatorPtr communicator, const string& count, int traceLevel) :
	_communicator(communicator),
	_traceLevel(traceLevel)
    {
	istringstream s(count);
	if(!(s >> _count) || !s.eof())
	{
	    InitializationException ex(__FILE__, __LINE__);
	    ex.reason = "Error parsing MaxEndpoints property";
	    throw ex;
	}
	if(_count <= 0)
	{
	    InitializationException ex(__FILE__, __LINE__);
	    ex.reason = "MaxEndpoints must be greater than 1";
	    throw ex;
	}
    }

    bool
    check(const ObjectPrx& p) const
    {
	EndpointSeq endpoints = p->ice_getEndpoints();
	bool result = (endpoints.size() > _count);
	if(_traceLevel >= 3)
	{
	    Trace out(_communicator->getLogger(), "Glacier2");
	    out << _communicator->proxyToString(p) << (result ? " exceeds " : " meets ") 
		<< "endpoint count restriction\n";
	}
	return result;
    }

private:
    const CommunicatorPtr _communicator;
    const int _traceLevel;
    unsigned long _count;
};

} // End proxy rule implementations.

Glacier2::ProxyVerifier::ProxyVerifier(const CommunicatorPtr& communicator, const char* ruleSet):
    _communicator(communicator),
    _traceLevel(communicator->getProperties()->getPropertyAsInt("Glacier2.Client.Trace.Reject"))
{
    //
    // Evaluation order is dependant on how the rules are stored to the
    // rules vectors. 
    //
    string s = communicator->getProperties()->getProperty("Glacier2.Client.Filter.Regex.Accept");
    if(s != "")
    {
	_acceptRules.push_back(new RegexRule(communicator, s, _traceLevel));
    }

    s = communicator->getProperties()->getProperty("Glacier2.Client.Filter.Regex.Reject");
    if(s != "")
    {
	_rejectRules.push_back(new RegexRule(communicator, s, _traceLevel));
    }

    s = communicator->getProperties()->getProperty("Glacier2.Client.Filter.MaxProxyLength");
    if(s != "")
    {
	_rejectRules.push_back(new MaxEndpointsRule(communicator, s, _traceLevel));
    }
}

Glacier2::ProxyVerifier::~ProxyVerifier()
{
    for(vector<ProxyRule*>::const_iterator i = _acceptRules.begin(); i != _acceptRules.end(); ++i)
    {
	delete (*i);
    }
    for(vector<ProxyRule*>::const_iterator j = _rejectRules.begin(); j != _rejectRules.end(); ++j)
    {
	delete (*j);
    }
}

bool
Glacier2::ProxyVerifier::verify(const ObjectPrx& proxy)
{
    //
    // No rules have been defined so we accept all.
    //
    if(_acceptRules.size() == 0 && _rejectRules.size() == 0)
    {
	return true;
    }

    bool result;

    if(_rejectRules.size() == 0)
    {
	//
	// If there are no reject rules, we assume "reject all".
	//
	bool matched = false;
	for(vector<ProxyRule*>::const_iterator i = _acceptRules.begin(); i != _acceptRules.end() && !matched; ++i)
	{
	    matched = (*i)->check(proxy);
	}
	result = matched;
    }
    else if(_acceptRules.size() == 0)
    {
	//
	// If no accept rules are defined we assume accept all.
	//
	bool matched = false;
	for(vector<ProxyRule*>::const_iterator i = _rejectRules.begin(); i != _rejectRules.end() && !matched; ++i)
	{
	    matched = (*i)->check(proxy);
	}
	result = !matched;
    }
    else
    {
	//
	// Reject first, then accept.
	//
	bool matched = false;
	for(vector<ProxyRule*>::const_iterator i = _rejectRules.begin(); i != _rejectRules.end() && !matched; ++i)
	{
	    matched = (*i)->check(proxy);
	}

	if(!matched)
	{
	    //
	    // The proxy wasn't rejected, so there isn't any point in
	    // running the accept rules.
	    //
	    return true;
	}

	matched = false;
	for(vector<ProxyRule*>::const_iterator j = _acceptRules.begin(); j != _acceptRules.end() && !matched; ++j)
	{
	    matched = (*j)->check(proxy);
	}
	result = matched;
    }

    //
    // The proxy rules take care of the tracing for higher trace levels.
    //
    if(_traceLevel > 0)
    {
	Trace out(_communicator->getLogger(), "Glacier2");
	if(result)
	{
	    out << "accepted proxy " << _communicator->proxyToString(proxy) << '\n';
	}
	else
	{
	    out << "rejected proxy " << _communicator->proxyToString(proxy) << '\n';
	}
    }
    return result;
}
