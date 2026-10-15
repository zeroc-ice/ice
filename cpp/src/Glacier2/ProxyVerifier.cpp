//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Glacier2/ProxyVerifier.h>
#include <Ice/ConsoleUtil.h>
#include <IceUtil/StringUtil.h>

#include <cctype>
#include <cstdlib>
#include <vector>
#include <string>

using namespace std;
using namespace Ice;
using namespace IceInternal;

//
// TODO: Some of the address matching helper classes can probably be
// refactored out. It's a question of whether to keep really simple
// differences refactored into separate classes or go to fewer but
// slightly smarter classes.
//

//
// Proxy rule implementations.
//
namespace Glacier2
{

//
// A numeric range that can be encoded in a filter.
//
struct Range
{
    long start;
    long end;
};

static void
parseGroup(const string& parameter, vector<int>& validPorts, vector<Range>& ranges)
{
    istringstream istr(parameter);
    while(!istr.eof())
    {
        ws(istr);
        int value;
        if(!(istr >> value))
        {
            throw invalid_argument("expected number");
        }
        ws(istr);
        if(!istr.eof())
        {
            char c;
            if(istr >> c)
            {
                if(c == ',')
                {
                    validPorts.push_back(value);
                }
                else if(c == '-')
                {
                    Range r;
                    r.start = value;
                    ws(istr);
                    if(istr.eof())
                    {
                        throw invalid_argument("Unterminated range");
                    }
                    if(!(istr >> value))
                    {
                        throw invalid_argument("expected number");
                    }
                    r.end = value;
                    ws(istr);
                    if(!istr.eof())
                    {
                        istr >> c;
                        if(c != ',')
                        {
                            throw invalid_argument("expected comma separator");
                        }
                    }
                    ranges.push_back(r);
                }
                else if(!istr.eof())
                {
                    throw invalid_argument("unexpected trailing character");
                }
            }
        }
        else
        {
            validPorts.push_back(value);
        }
    }
}

//
// Base class for address matching operations.
//
class AddressMatcher
{
public:
    virtual ~AddressMatcher() {}

    //
    // Matches space, the string to search, starting at position pos. Returns true when the matcher finds a
    // match, and false otherwise. When this function returns true, it sets pos to the position immediately
    // after the matched portion of space.
    //
    virtual bool match(const string& space, string::size_type& pos) = 0;

    //
    // Searches for the next match in space after a previous match that ended at position pos. Returns true
    // when the matcher finds another match, and sets pos to the position immediately after it. The default
    // implementation returns false: most matchers can match at a single position only. Matchers created for
    // the portion of a rule that follows a wildcard can match at multiple positions and override this
    // function.
    //
    virtual bool
    retry(const string&, string::size_type&)
    {
        return false;
    }

    virtual const char* toString() const = 0;

protected:
};

class MatchesAny : public AddressMatcher
{
public:
    MatchesAny()
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        pos = space.size();
        return true;
    }

    const char*
    toString() const
    {
        return "(ANY)";
    }
};

//
// Match the start of a string (i.e. position == 0). Occurs when filter
// string starts with a set of characters followed by a wildcard or
// numeric range.
//
class StartsWithString : public AddressMatcher
{
public:
    StartsWithString(const string& criteria):
        _criteria(criteria),
        _description("starts with " + criteria)
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        assert(pos == 0);
        bool result = strncmp(space.c_str(), _criteria.c_str(), _criteria.size()) == 0;
        if(result)
        {
            pos += _criteria.size();
        }
        return result;
    }

    const char*
    toString() const
    {
        return _description.c_str();
    }

private:
    string _criteria;
    string _description;
};

//
// Match the end portion of a string. Occurs when a filter string starts
// with a wildcard or numeric range, but ends with a string.
//
class EndsWithString : public AddressMatcher
{
public:
    EndsWithString(const string& criteria):
        _criteria(criteria),
        _description("ends with " + criteria)
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        if(space.size() - pos < _criteria.size())
        {
            return false;
        }

        string::size_type spaceEnd = space.size();
        for(string::size_type i = _criteria.size(); i > 0; --i)
        {
            if(space[spaceEnd - 1] != _criteria[i-1])
            {
                return false;
            }
            --spaceEnd;
        }
        pos = space.size();
        return true;
    }

    virtual const char*
    toString() const
    {
        return _description.c_str();
    }

private:
    string _criteria;
    string _description;
};

class MatchesString : public AddressMatcher
{
public:
    MatchesString(const string& criteria):
        _criteria(criteria),
        _description("matches " + criteria)
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        if(pos + _criteria.size() <= space.size() &&
           strncmp(space.c_str() + pos, _criteria.c_str(), _criteria.size()) == 0)
        {
            pos += _criteria.size();
            return true;
        }
        return false;
    }

    virtual const char*
    toString() const
    {
        return _description.c_str();
    }

private:
    string _criteria;
    string _description;
};

//
// Match against somewhere within a string. Occurs when a filter
// contains a string bounded by wildcards, or numeric ranges. e.g. *bar*.com.
//
class ContainsString : public AddressMatcher
{
public:
    ContainsString(const string& criteria):
        _criteria(criteria),
        _description("contains " + criteria)
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        string::size_type offset = space.find(_criteria, pos);
        if(offset == string::npos)
        {
            return false;
        }
        pos = offset + _criteria.size();
        return true;
    }

    bool
    retry(const string& space, string::size_type& pos)
    {
        // Resume the search one character past the start of the previous match.
        pos -= _criteria.size() - 1;
        return match(space, pos);
    }

    virtual const char*
    toString() const
    {
        return _description.c_str();
    }

private:
    string _criteria;
    string _description;
};

//
// Match a number against a range of values. This occurs when the filter
// contains a numeric range or group of numeric values. e.g. foo[1-3,
// 10].bar.com. Also used to match port numbers and ranges.
//
class MatchesNumber : public AddressMatcher
{
public:
    MatchesNumber(const vector<int>& values, const vector<Range>& ranges,
                  const char* descriptionPrefix = "matches "):
        _values(values),
        _ranges(ranges)
    {
        ostringstream ostr;
        ostr << descriptionPrefix;
        {
            bool start = true;
            for(vector<int>::const_iterator i = values.begin(); i != values.end(); ++i)
            {
                if(start)
                {
                    if(values.size() > 1)
                    {
                        ostr << "one of ";
                    }
                    start = false;
                }
                else
                {
                    ostr << ", ";
                }

                ostr << *i;
            }
        }
        if(values.size() > 0 && ranges.size() > 0)
        {
            ostr << " or ";
        }
        {
            bool start = true;
            for(vector<Range>::const_iterator i = ranges.begin(); i != ranges.end(); ++i)
            {
                if(start)
                {
                    start = false;
                }
                else
                {
                    ostr << ", or";
                }
                ostr << i->start << " up to " << i->end;
            }
        }
        _description = ostr.str();
    }

    bool
    match(const string & space, string::size_type& pos)
    {
        istringstream istr(space.substr(pos));
        int val;
        if(!(istr >> val))
        {
            return false;
        }

        // tellg() returns -1 once the extraction reached the end of the string.
        pos = istr.eof() ? space.size() : pos + static_cast<string::size_type>(istr.tellg());
        {
            for(vector<int>::const_iterator i = _values.begin(); i != _values.end(); ++i)
            {
                if(val == *i)
                {
                    return true;
                }
            }
        }
        {
            for(vector<Range>::const_iterator i = _ranges.begin(); i != _ranges.end(); ++i)
            {
                if((val >= i->start) && (val <= i->end))
                {
                    return true;
                }
            }
        }
        return false;
    }

    virtual const char*
    toString() const
    {
        return _description.c_str();
    }

private:
    const vector<int> _values;
    const vector<Range> _ranges;
    string _description;
};

//
// Occurs when a numeric range is preceded by a wildcard.
//
class ContainsNumberMatch : public MatchesNumber
{
public:
    ContainsNumberMatch(const vector<int>& values, const vector<Range>& ranges):
        MatchesNumber(values, ranges, "contains ")
    {
    }

    bool
    match(const string& space, string::size_type& pos)
    {
        while(true)
        {
            pos = space.find_first_of("0123456789", pos);
            if(pos == string::npos)
            {
                return false;
            }

            if(MatchesNumber::match(space, pos))
            {
                return true;
            }

            //
            // MatchesNumber::match does not advance pos when the extraction fails (the digit run is too
            // long to fit in an int); skip the digit run so that the scan makes progress.
            //
            pos = space.find_first_not_of("0123456789", pos);
            if(pos == string::npos)
            {
                return false;
            }
        }
        return false;
    }

    bool
    retry(const string& space, string::size_type& pos)
    {
        // Resume the scan after the previously matched number.
        return match(space, pos);
    }
};

//
// AddressMatcher factories abstract away the logic of which matching
// objects need to be created depending on the state of the filter
// string parsing. Similar to changing a tool that produces the right
// result depending on how far along you are in the job, the factories
// are selected according to what transition has occurred while parsing
// the filter string.
//
class AddressMatcherFactory
{
public:
    virtual ~AddressMatcherFactory() {}

    virtual AddressMatcher*
    create(const string& criteria) = 0;

    virtual AddressMatcher*
    create(const vector<int>& ports, const vector<Range>& ranges) = 0;
};

class StartFactory : public AddressMatcherFactory
{
public:
    AddressMatcher*
    create(const string& criteria)
    {
        return new StartsWithString(criteria);
    }

    AddressMatcher*
    create(const vector<int>& ports, const vector<Range>& ranges)
    {
        return new MatchesNumber(ports, ranges);
    }
};

class WildCardFactory : public AddressMatcherFactory
{
public:
    AddressMatcher*
    create(const string& criteria)
    {
        return new ContainsString(criteria);
    }

    AddressMatcher*
    create(const vector<int>& ports, const vector<Range>& ranges)
    {
        return new ContainsNumberMatch(ports, ranges);
    }
};

class FollowingFactory : public AddressMatcherFactory
{
public:
    AddressMatcher*
    create(const string& criteria)
    {
        return new MatchesString(criteria);
    }

    AddressMatcher*
    create(const vector<int>& ports, const vector<Range>& ranges)
    {
        return new MatchesNumber(ports, ranges);
    }
};

class EndsWithFactory : public AddressMatcherFactory
{
public:
    AddressMatcher*
    create(const string& criteria)
    {
        return new EndsWithString(criteria);
    }

    AddressMatcher*
    create(const vector<int>&, const vector<Range>&)
    {
        assert(false); // unreachable -- this factory is selected only after the parse loop, once every group
                       // has been consumed
        return ICE_NULLPTR;
    }
};

static bool
extractPart(const char* opt, const string& source, string& result)
{
    string::size_type start = source.find(opt);
    if(start == string::npos)
    {
        return false;
    }
    start += strlen(opt);
    string::size_type end = source.find(' ', start);
    if(end != string::npos)
    {
        result = source.substr(start, end - start);
    }
    else
    {
        result = source.substr(start);
    }
    return true;
}

//
// Splits host into its '.'-separated parts.
//
static vector<string>
splitHost(const string& host)
{
    vector<string> parts;
    string::size_type start = 0;
    while(true)
    {
        string::size_type dot = host.find('.', start);
        parts.push_back(host.substr(start, dot == string::npos ? string::npos : dot - start));
        if(dot == string::npos)
        {
            break;
        }
        start = dot + 1;
    }
    return parts;
}

//
// Returns true when s is not empty and every character of s is a decimal digit.
//
static bool
isDigits(const string& s)
{
    if(s.empty())
    {
        return false;
    }
    for(string::const_iterator c = s.begin(); c != s.end(); ++c)
    {
        if(!isdigit(static_cast<unsigned char>(*c)))
        {
            return false;
        }
    }
    return true;
}

//
// Returns true when s is a 0x or 0X prefix followed by zero or more hexadecimal digits.
//
static bool
isHexNumeral(const string& s)
{
    if(s.size() < 2 || s[0] != '0' || (s[1] != 'x' && s[1] != 'X'))
    {
        return false;
    }
    for(string::const_iterator c = s.begin() + 2; c != s.end(); ++c)
    {
        if(!isxdigit(static_cast<unsigned char>(*c)))
        {
            return false;
        }
    }
    return true;
}

//
// Returns true when every '.'-separated part of host is a numeral (digits, or a 0x prefix followed by
// hexadecimal digits): the host spells an IPv4 address in one of the many forms resolvers accept. This is
// a check on the spelling, not the value: a digit-only part covers the decimal and octal (leading zero)
// readings alike, and a bare 0x with no digits reads as 0.
//
static bool
isIPv4Numeral(const string& host)
{
    vector<string> parts = splitHost(host);
    for(vector<string>::const_iterator p = parts.begin(); p != parts.end(); ++p)
    {
        if(!isDigits(*p) && !isHexNumeral(*p))
        {
            return false;
        }
    }
    return true;
}

//
// Returns true when host is the canonical spelling of an IPv4 address: four decimal parts, each between 0
// and 255, with no leading zeros.
//
static bool
isCanonicalIPv4Spelling(const string& host)
{
    vector<string> parts = splitHost(host);
    if(parts.size() != 4)
    {
        return false;
    }
    for(vector<string>::const_iterator p = parts.begin(); p != parts.end(); ++p)
    {
        if(!isDigits(*p) || p->size() > 3 || (p->size() > 1 && (*p)[0] == '0') || atoi(p->c_str()) > 255)
        {
            return false;
        }
    }
    return true;
}

//
// Retrieves the host of the innermost IP endpoint underlying an endpoint. Returns false when no such endpoint
// exists: the transport is not IP based (such as Bluetooth), or it is unknown to this communicator. An unknown
// transport, including an IP-based one whose plug-in is not loaded, is opaque and carries no typed endpoint
// info, so it is indistinguishable from a non-IP transport here.
//
static bool
ipEndpointHost(const EndpointPtr& endpoint, string& host)
{
    for(EndpointInfoPtr info = endpoint->getInfo(); info; info = info->underlying)
    {
        IPEndpointInfoPtr ipInfo = ICE_DYNAMIC_CAST(IPEndpointInfo, info);
        if(ipInfo)
        {
            host = ipInfo->host;
            return true;
        }
    }
    return false;
}

//
// Returns true when an endpoint of the proxy has a host that the address rules cannot match reliably; when an
// address filter is configured, such a proxy is rejected outright:
// - A non-IP endpoint (such as Bluetooth), or an endpoint whose transport is unknown to this communicator.
// - An empty host.
// - A host containing a control character or a space.
// - A host longer than 255 characters. No legal host name or IP address is that long, and the bound keeps
//   the cost of matching the address rules low.
// - A non-canonical spelling of an IPv4 address, such as 0x7f000001, 2130706433, 0177.0.0.1, or 127.1.
//   The resolver accepts many aliases for the same address (with platform-dependent readings), so the
//   spelling would bypass rules written for the dotted quad.
// - An IPv4 address with a trailing dot: the resolver reads it as a DNS name, not as an address.
// A DNS name with a trailing dot is fine: the resolver treats 'name.' as 'name', and the matching strips
// the dot the same way.
// Returns false for an indirect proxy: it has no endpoints, so there is no host to check here (the address
// rules match nothing, and the identity and adapter-id filters constrain it instead).
//
static bool
hasDisallowedHost(const ObjectPrx& proxy)
{
    EndpointSeq endpoints = proxy->ice_getEndpoints();
    for(EndpointSeq::const_iterator p = endpoints.begin(); p != endpoints.end(); ++p)
    {
        string host;
        if(!ipEndpointHost(*p, host))
        {
            return true;
        }

        if(host.empty() || host.size() > 255)
        {
            return true;
        }
        for(string::const_iterator c = host.begin(); c != host.end(); ++c)
        {
            if(*c == ' ' || iscntrl(static_cast<unsigned char>(*c)))
            {
                return true;
            }
        }

        bool trailingDot = host.size() > 1 && host[host.size() - 1] == '.';
        if(trailingDot)
        {
            host.erase(host.size() - 1);
        }
        if(isIPv4Numeral(host) && (trailingDot || !isCanonicalIPv4Spelling(host)))
        {
            return true;
        }
    }
    return false;
}

//
// A proxy validation rule encapsulating an address filter. An accept rule matches a proxy only when every
// endpoint matches it, while a reject rule matches a proxy as soon as one endpoint matches it
// (matchAnyEndpoint == true).
//
class AddressRule : public Glacier2::ProxyRule
{
public:
    AddressRule(const CommunicatorPtr& communicator, const vector<AddressMatcher*>& address, MatchesNumber* port,
                const int traceLevel, bool matchAnyEndpoint) :
        _communicator(communicator),
        _addressRules(address),
        _portMatcher(port),
        _traceLevel(traceLevel),
        _matchAnyEndpoint(matchAnyEndpoint)
    {
    }

    ~AddressRule()
    {
        for(vector<AddressMatcher*>::const_iterator i = _addressRules.begin(); i != _addressRules.end(); ++i)
        {
            delete *i;
        }
        delete _portMatcher;
    }

    virtual bool
    check(const ObjectPrx& prx) const
    {
        EndpointSeq endpoints = prx->ice_getEndpoints();
        if(endpoints.size() == 0)
        {
            return false;
        }

        if(_matchAnyEndpoint)
        {
            for(EndpointSeq::const_iterator i = endpoints.begin(); i != endpoints.end(); ++i)
            {
                if(matchEndpoint(*i))
                {
                    return true;
                }
            }
            return false;
        }
        else
        {
            for(EndpointSeq::const_iterator i = endpoints.begin(); i != endpoints.end(); ++i)
            {
                if(!matchEndpoint(*i))
                {
                    return false;
                }
            }
            return true;
        }
    }

    void
    dump() const
    {
        consoleErr << "address(";
        for(vector<AddressMatcher*>::const_iterator i = _addressRules.begin(); i != _addressRules.end(); ++i)
        {
            consoleErr << (*i)->toString() << " ";
        }
        if(_portMatcher != 0)
        {
            consoleErr << "):port(" << _portMatcher->toString() << " ";
        }
        consoleErr << ")" << endl;
    }

private:

    //
    // Matches the host and port of endpoint against this rule.
    //
    bool
    matchEndpoint(const EndpointPtr& endpoint) const
    {
        string info = endpoint->toString();
        string host;
        if(!extractPart("-h ", info, host))
        {
            return false;
        }

        // Host names are case-insensitive; the rule was folded to lower case when parsed.
        host = IceUtilInternal::toLower(host);

        //
        // The resolver treats the fully-qualified name 'name.' as 'name'; strip the trailing dot so the
        // spelling matches the rules written for 'name'.
        //
        if(host.size() > 1 && host[host.size() - 1] == '.')
        {
            host.erase(host.size() - 1);
        }

        string port;
        if(!extractPart("-p ", info, port))
        {
            return false;
        }

        string::size_type pos = 0;
        if(_portMatcher && !_portMatcher->match(port, pos))
        {
            if(_traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << _portMatcher->toString() << " failed to match " << port << " at pos=" << pos << "\n";
            }
            return false;
        }

        vector<bool> failed(_addressRules.size() * (host.size() + 1), false);
        return matchAddress(host, 0, 0, failed);
    }

    //
    // Matches host against the matchers at position index and up, starting at position pos in host. The
    // matchers must match the remainder of the host in full. When they don't, this function retries the
    // matchers that can match at a later position (the matchers created for the portion of a rule that
    // follows a wildcard) until every later alignment is exhausted. The same (index, pos) state can be
    // reached through many alignments of the preceding wildcards; failed is a matcher-count by
    // (host length + 1) matrix that records failed states so each one is evaluated at most once, keeping
    // the matching cost polynomial in the host length.
    //
    bool
    matchAddress(const string& host, vector<AddressMatcher*>::size_type index, string::size_type pos,
                 vector<bool>& failed) const
    {
        if(index == _addressRules.size())
        {
            // The rule must match the whole host, not just a prefix of it.
            if(pos != host.size())
            {
                if(_traceLevel >= 3)
                {
                    Trace out(_communicator->getLogger(), "Glacier2");
                    out << "matched a prefix of " << host << " only, up to pos=" << pos << "\n";
                }
                return false;
            }
            return true;
        }

        vector<bool>::size_type state = index * (host.size() + 1) + pos;
        if(failed[state])
        {
            return false;
        }

        AddressMatcher* rule = _addressRules[index];
        string::size_type next = pos;
        bool matched = rule->match(host, next);
        while(matched)
        {
            if(_traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << rule->toString() << " matched " << host << " at pos=" << next << "\n";
            }
            if(matchAddress(host, index + 1, next, failed))
            {
                return true;
            }
            matched = rule->retry(host, next);
        }

        if(_traceLevel >= 3)
        {
            Trace out(_communicator->getLogger(), "Glacier2");
            out << rule->toString() << " failed to match " << host << " at pos=" << pos << "\n";
        }
        failed[state] = true;
        return false;
    }

    CommunicatorPtr _communicator;
    vector<AddressMatcher*> _addressRules;
    MatchesNumber* _portMatcher;
    const int _traceLevel;
    const bool _matchAnyEndpoint;
};

static void
parseProperty(const Ice::CommunicatorPtr& communicator, const string& property, vector<ProxyRule*>& rules,
              const int traceLevel, bool matchAnyEndpoint)
{
    StartFactory startsWithFactory;
    WildCardFactory wildCardFactory;
    EndsWithFactory endsWithFactory;
    FollowingFactory followingFactory;
    vector<ProxyRule*> allRules;
    try
    {
        istringstream propertyInput(property);

        while(!propertyInput.eof() && propertyInput.good())
        {
            MatchesNumber* portMatch = 0;
            vector<AddressMatcher*> currentRuleSet;

            string parameter;
            ws(propertyInput);
            propertyInput >> parameter;

            string portInfo;
            string::size_type portPortion = parameter.find(':');

            string addr;
            if(portPortion != string::npos)
            {
                addr = parameter.substr(0, portPortion);
                string port = parameter.substr(portPortion + 1);
                string::size_type openBracket = port.find('[');
                if(openBracket != string::npos)
                {
                    ++openBracket;
                    string::size_type closeBracket = port.find(']', openBracket);
                    if(closeBracket == string::npos)
                    {
                        throw invalid_argument("unclosed group");
                    }
                    port = port.substr(openBracket, closeBracket-openBracket);
                }
                vector<int> ports;
                vector<Range> ranges;
                parseGroup(port, ports, ranges);
                portMatch = new MatchesNumber(ports, ranges);
            }
            else
            {
                addr = parameter;
            }

            //
            // Host names are case-insensitive; fold the rule to lower case and match it against the
            // lower-cased host.
            //
            addr = IceUtilInternal::toLower(addr);

            //
            // The trailing dot of a fully-qualified name is stripped from the host before matching; strip
            // it from the rule the same way.
            //
            if(addr.size() > 1 && addr[addr.size() - 1] == '.')
            {
                addr.erase(addr.size() - 1);
            }

            //
            // The addr portion can contain alphanumerics, * and
            // ranges.
            //
            string::size_type current = 0;

            if(current == addr.size())
            {
                throw invalid_argument("expected address information before ':'");
            }

            //
            // TODO: assuming that there is no leading or trailing whitespace. This
            // should probably be confirmed.
            //
            assert(!isspace(static_cast<unsigned char>(parameter[current])));
            assert(!isspace(static_cast<unsigned char>(addr[addr.size() -1])));

            if(current != 0)
            {
                addr = addr.substr(current);
            }

            string::size_type mark = 0;
            bool inGroup = false;
            AddressMatcherFactory* currentFactory = &startsWithFactory;

            if(addr == "*")
            {
                //
                // Special case. Match everything.
                //
                currentRuleSet.push_back(new MatchesAny);
            }
            else
            {
                for(current = 0; current < addr.size(); ++current)
                {
                    if(addr[current] == '*')
                    {
                        if(inGroup)
                        {
                            throw invalid_argument("wildcards not permitted in groups");
                        }
                        //
                        // current == mark when the wildcard is at the head of a
                        // string or directly after a group.
                        //
                        if(current != mark)
                        {
                            currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current-mark)));
                        }
                        currentFactory = &wildCardFactory;
                        mark = current + 1;
                    }
                    else if(addr[current] == '[')
                    {
                        // ??? what does it mean if current == mark?
                        if(current != mark)
                        {
                            currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current-mark)));
                            currentFactory = &followingFactory;
                        }
                        inGroup = true;
                        mark = current + 1;
                    }
                    else if(addr[current] == ']')
                    {
                        if(!inGroup)
                        {
                            throw invalid_argument("group close without group start");
                        }
                        inGroup = false;
                        if(mark == current)
                        {
                            throw invalid_argument("empty group");
                        }
                        string group = addr.substr(mark, current - mark);
                        vector<int> numbers;
                        vector<Range> ranges;
                        parseGroup(group, numbers, ranges);
                        currentRuleSet.push_back(currentFactory->create(numbers, ranges));
                        currentFactory = &followingFactory;
                        mark = current + 1;
                    }
                }
                if(inGroup)
                {
                    throw invalid_argument("unclosed group");
                }

                if(mark != current)
                {
                    //
                    // A trailing string matches the end of the host only when a wildcard precedes it. Otherwise
                    // it must match at the position the preceding matchers stopped at.
                    //
                    if(currentFactory == &wildCardFactory)
                    {
                        currentFactory = &endsWithFactory;
                    }
                    currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                }
                else if(currentFactory == &wildCardFactory)
                {
                    // A trailing wildcard matches the remainder of the host.
                    currentRuleSet.push_back(new MatchesAny);
                }
            }
            allRules.push_back(new AddressRule(communicator, currentRuleSet, portMatch, traceLevel,
                                               matchAnyEndpoint));
        }
    }
    catch(...)
    {
        for(vector<ProxyRule*>::const_iterator i = allRules.begin(); i != allRules.end(); ++i)
        {
            delete *i;
        }
        throw;
    }
    rules = allRules;
}

//
// Helper function for checking a rule set.
//
static bool
match(const vector<ProxyRule*>& rules, const ObjectPrx& proxy)
{
    for(vector<ProxyRule*>::const_iterator i = rules.begin(); i != rules.end(); ++i)
    {
        if((*i)->check(proxy))
        {
            return true;
        }
    }
    return false;
}

//
// ProxyLengthRule returns 'true' if the string form of the proxy exceeds the configured
// length.
//
class ProxyLengthRule : public ProxyRule
{
public:
    ProxyLengthRule(const CommunicatorPtr communicator, const string& count, int traceLevel) :
        _communicator(communicator),
        _traceLevel(traceLevel)
    {
        istringstream s(count);
        if(!(s >> _count) || !s.eof())
        {
            throw invalid_argument("Error parsing ProxySizeMax property");
        }
        if(_count <= 0)
        {
            throw invalid_argument("ProxySizeMax must be greater than 1");
        }
    }

    bool
    check(const ObjectPrx& p) const
    {
        string s = p->ice_toString();
        bool result = (s.size() > _count);
        if(_traceLevel >= 1)
        {
            Trace out(_communicator->getLogger(), "Glacier2");
            out << _communicator->proxyToString(p) << (result ? " exceeds " : " meets ")
                << "proxy size restriction\n";
        }
        return result;
    }

private:
    const CommunicatorPtr _communicator;
    const int _traceLevel;
    unsigned long _count;
};

} // End proxy rule implementations.

Glacier2::ProxyVerifier::ProxyVerifier(const CommunicatorPtr& communicator):
    _communicator(communicator),
    _traceLevel(communicator->getProperties()->getPropertyAsInt("Glacier2.Client.Trace.Reject")),
    _hasAddressFilters(false)
{
    //
    // Evaluation order is dependant on how the rules are stored to the
    // rules vectors.
    //
    string s = communicator->getProperties()->getProperty("Glacier2.Filter.Address.Accept");
    if(s != "")
    {
        _hasAddressFilters = true;
        try
        {
            // An accept rule matches a proxy only when every endpoint matches it.
            Glacier2::parseProperty(communicator, s, _acceptRules, _traceLevel, false);
        }
        catch(const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.Address.Accept' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    s = communicator->getProperties()->getProperty("Glacier2.Filter.Address.Reject");
    if(s != "")
    {
        _hasAddressFilters = true;
        try
        {
            // A reject rule matches a proxy as soon as one endpoint matches it.
            Glacier2::parseProperty(communicator, s, _rejectRules, _traceLevel, true);
        }
        catch(const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.Address.Reject' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    s = communicator->getProperties()->getProperty("Glacier2.Filter.ProxySizeMax");
    if(s != "")
    {
        try
        {
            _rejectRules.push_back(new ProxyLengthRule(communicator, s, _traceLevel));

        }
        catch(const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.ProxySizeMax' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
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

    bool result = false;

    if(_hasAddressFilters && Glacier2::hasDisallowedHost(proxy))
    {
        // Rejected regardless of the configured rules.
    }
    else if(_rejectRules.size() == 0)
    {
        //
        // If there are no reject rules, we assume "reject all".
        //
        result = match(_acceptRules, proxy);
    }
    else if(_acceptRules.size() == 0)
    {
        //
        // If no accept rules are defined we assume accept all.
        //
        result = !match(_rejectRules, proxy);
    }
    else
    {
        if(match(_acceptRules, proxy))
        {
            result = !match(_rejectRules, proxy);
        }
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
