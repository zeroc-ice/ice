// Copyright (c) ZeroC, Inc.

#include "ProxyVerifier.h"
#include "../Ice/ConsoleUtil.h"
#include "Ice/StringUtil.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

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

    static void parseGroup(const string& parameter, vector<int>& validPorts, vector<Range>& ranges)
    {
        istringstream istr(parameter);
        while (!istr.eof())
        {
            ws(istr);
            int value;
            if (!(istr >> value))
            {
                throw invalid_argument("expected number");
            }
            ws(istr);
            if (!istr.eof())
            {
                char c;
                if (istr >> c)
                {
                    if (c == ',')
                    {
                        validPorts.push_back(value);
                    }
                    else if (c == '-')
                    {
                        Range r;
                        r.start = value;
                        ws(istr);
                        if (istr.eof())
                        {
                            throw invalid_argument("Unterminated range");
                        }
                        if (!(istr >> value))
                        {
                            throw invalid_argument("expected number");
                        }
                        r.end = value;
                        if (r.end < r.start)
                        {
                            throw invalid_argument("range lower bound is greater than its upper bound");
                        }
                        ws(istr);
                        if (!istr.eof())
                        {
                            istr >> c;
                            if (c != ',')
                            {
                                throw invalid_argument("expected comma separator");
                            }
                        }
                        ranges.push_back(r);
                    }
                    else if (!istr.eof())
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
        virtual ~AddressMatcher() = default;

        // Matches space, the string to search, starting at position pos. Returns true when the matcher finds a
        // match, and false otherwise. When this function returns true, it sets pos to the position immediately
        // after the matched portion of space.
        virtual bool match(const string& space, string::size_type& pos) = 0;

        // Searches for the next match in space after a previous match that ended at position pos. Returns true
        // when the matcher finds another match, and sets pos to the position immediately after it. The default
        // implementation returns false: most matchers can match at a single position only. Matchers created for
        // the portion of a rule that follows a wildcard can match at multiple positions and override this
        // function.
        virtual bool retry(const string&, string::size_type&) { return false; }

        [[nodiscard]] virtual const char* toString() const = 0;
    };

    class MatchesAny final : public AddressMatcher
    {
    public:
        bool match(const string& space, string::size_type& pos) override
        {
            pos = space.size();
            return true;
        }

        [[nodiscard]] const char* toString() const override { return "(ANY)"; }
    };

    //
    // Match the start of a string (i.e. position == 0). Occurs when filter
    // string starts with a set of characters followed by a wildcard or
    // numeric range.
    //
    class StartsWithString final : public AddressMatcher
    {
    public:
        StartsWithString(const string& criteria) : _criteria(criteria), _description("starts with " + criteria) {}

        bool match(const string& space, string::size_type& pos) override
        {
            assert(pos == 0);
            bool result = strncmp(space.c_str(), _criteria.c_str(), _criteria.size()) == 0;
            if (result)
            {
                pos += _criteria.size();
            }
            return result;
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

    private:
        string _criteria;
        string _description;
    };

    //
    // Match the end portion of a string. Occurs when a filter string starts
    // with a wildcard or numeric range, but ends with a string.
    //
    class EndsWithString final : public AddressMatcher
    {
    public:
        EndsWithString(const string& criteria) : _criteria(criteria), _description("ends with " + criteria) {}

        bool match(const string& space, string::size_type& pos) override
        {
            if (space.size() - pos < _criteria.size())
            {
                return false;
            }

            string::size_type spaceEnd = space.size();
            for (string::size_type i = _criteria.size(); i > 0; --i)
            {
                if (space[spaceEnd - 1] != _criteria[i - 1])
                {
                    return false;
                }
                --spaceEnd;
            }
            pos = space.size();
            return true;
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

    private:
        string _criteria;
        string _description;
    };

    class MatchesString final : public AddressMatcher
    {
    public:
        MatchesString(const string& criteria) : _criteria(criteria), _description("matches " + criteria) {}

        bool match(const string& space, string::size_type& pos) override
        {
            if (pos + _criteria.size() <= space.size() &&
                strncmp(space.c_str() + pos, _criteria.c_str(), _criteria.size()) == 0)
            {
                pos += _criteria.size();
                return true;
            }
            return false;
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

    private:
        string _criteria;
        string _description;
    };

    //
    // Match against somewhere within a string. Occurs when a filter
    // contains a string bounded by wildcards, or numeric ranges. e.g. *bar*.com.
    //
    class ContainsString final : public AddressMatcher
    {
    public:
        ContainsString(const string& criteria) : _criteria(criteria), _description("contains " + criteria) {}

        bool match(const string& space, string::size_type& pos) override
        {
            string::size_type offset = space.find(_criteria, pos);
            if (offset == string::npos)
            {
                return false;
            }
            pos = offset + _criteria.size();
            return true;
        }

        bool retry(const string& space, string::size_type& pos) override
        {
            // Resume the search one character past the start of the previous match.
            pos -= _criteria.size() - 1;
            return match(space, pos);
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

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
        MatchesNumber(
            const vector<int>& values,
            const vector<Range>& ranges,
            const char* descriptionPrefix = "matches ")
            : _values(values),
              _ranges(ranges)
        {
            ostringstream ostr;
            ostr << descriptionPrefix;
            {
                bool start = true;
                for (auto i = values.begin(); i != values.end(); ++i)
                {
                    if (start)
                    {
                        if (values.size() > 1)
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
            if (values.size() > 0 && ranges.size() > 0)
            {
                ostr << " or ";
            }
            {
                bool start = true;
                for (const auto& range : ranges)
                {
                    if (start)
                    {
                        start = false;
                    }
                    else
                    {
                        ostr << ", or";
                    }
                    ostr << range.start << " up to " << range.end;
                }
            }
            _description = ostr.str();
        }

        bool match(const string& space, string::size_type& pos) override
        {
            istringstream istr(space.substr(pos));
            int val;
            if (!(istr >> val))
            {
                return false;
            }

            // tellg() returns -1 once the extraction reached the end of the string.
            pos = istr.eof() ? space.size() : pos + static_cast<string::size_type>(istr.tellg());
            {
                for (int value : _values)
                {
                    if (val == value)
                    {
                        return true;
                    }
                }
            }
            {
                for (const auto& range : _ranges)
                {
                    if ((val >= range.start) && (val <= range.end))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

    private:
        const vector<int> _values;
        const vector<Range> _ranges;
        string _description;
    };

    //
    // Occurs when a numeric range is preceded by a wildcard.
    //
    class ContainsNumberMatch final : public MatchesNumber
    {
    public:
        ContainsNumberMatch(const vector<int>& values, const vector<Range>& ranges)
            : MatchesNumber(values, ranges, "contains ")
        {
        }

        bool match(const string& space, string::size_type& pos) override
        {
            while (true)
            {
                pos = space.find_first_of("0123456789", pos);
                if (pos == string::npos)
                {
                    return false;
                }

                if (MatchesNumber::match(space, pos))
                {
                    return true;
                }

                // MatchesNumber::match does not advance pos when the extraction fails (the digit run is too
                // long to fit in an int); skip the digit run so that the scan makes progress.
                pos = space.find_first_not_of("0123456789", pos);
                if (pos == string::npos)
                {
                    return false;
                }
            }
            return false;
        }

        bool retry(const string& space, string::size_type& pos) override
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
        virtual AddressMatcher* create(const string& criteria) = 0;

        virtual AddressMatcher* create(const vector<int>& ports, const vector<Range>& ranges) = 0;
    };

    class StartFactory final : public AddressMatcherFactory
    {
    public:
        AddressMatcher* create(const string& criteria) override { return new StartsWithString(criteria); }

        AddressMatcher* create(const vector<int>& ports, const vector<Range>& ranges) override
        {
            return new MatchesNumber(ports, ranges);
        }
    };

    class WildCardFactory final : public AddressMatcherFactory
    {
    public:
        AddressMatcher* create(const string& criteria) override { return new ContainsString(criteria); }

        AddressMatcher* create(const vector<int>& ports, const vector<Range>& ranges) override
        {
            return new ContainsNumberMatch(ports, ranges);
        }
    };

    class FollowingFactory final : public AddressMatcherFactory
    {
    public:
        AddressMatcher* create(const string& criteria) override { return new MatchesString(criteria); }

        AddressMatcher* create(const vector<int>& ports, const vector<Range>& ranges) override
        {
            return new MatchesNumber(ports, ranges);
        }
    };

    class EndsWithFactory final : public AddressMatcherFactory
    {
    public:
        AddressMatcher* create(const string& criteria) override { return new EndsWithString(criteria); }

        AddressMatcher* create(const vector<int>&, const vector<Range>&) override
        {
            assert(false); // unreachable — this factory is selected only after the parse loop, once every group
                           // has been consumed
            return nullptr;
        }
    };

    static bool extractPart(const char* opt, const string& source, string& result)
    {
        string::size_type start = source.find(opt);
        if (start == string::npos)
        {
            return false;
        }
        start += strlen(opt);
        string::size_type end = source.find(' ', start);
        if (end != string::npos)
        {
            result = source.substr(start, end - start);
        }
        else
        {
            result = source.substr(start);
        }
        return true;
    }

    // Splits host into its '.'-separated parts.
    static vector<string> splitHost(const string& host)
    {
        vector<string> parts;
        string::size_type start = 0;
        while (true)
        {
            string::size_type dot = host.find('.', start);
            parts.push_back(host.substr(start, dot == string::npos ? string::npos : dot - start));
            if (dot == string::npos)
            {
                break;
            }
            start = dot + 1;
        }
        return parts;
    }

    // Returns true when every '.'-separated part of host is a numeral (digits, or a 0x prefix followed by
    // hexadecimal digits): the host spells an IPv4 address in one of the many forms resolvers accept. This is
    // a check on the spelling, not the value: a digit-only part covers the decimal and octal (leading zero)
    // readings alike, and a bare 0x with no digits reads as 0.
    static bool isIPv4Numeral(const string& host)
    {
        for (const string& part : splitHost(host))
        {
            bool digitsOnly =
                !part.empty() && all_of(part.begin(), part.end(), [](unsigned char c) { return isdigit(c) != 0; });
            bool hex = part.size() >= 2 && part[0] == '0' && (part[1] == 'x' || part[1] == 'X') &&
                       all_of(part.begin() + 2, part.end(), [](unsigned char c) { return isxdigit(c) != 0; });
            if (!digitsOnly && !hex)
            {
                return false;
            }
        }
        return true;
    }

    // Returns true when host is the canonical spelling of an IPv4 address: four decimal parts, each between 0
    // and 255, with no leading zeros.
    static bool isCanonicalIPv4Spelling(const string& host)
    {
        vector<string> parts = splitHost(host);
        if (parts.size() != 4)
        {
            return false;
        }
        for (const string& part : parts)
        {
            bool digitsOnly =
                !part.empty() && all_of(part.begin(), part.end(), [](unsigned char c) { return isdigit(c) != 0; });
            if (!digitsOnly || part.size() > 3 || (part.size() > 1 && part[0] == '0') || stoi(part) > 255)
            {
                return false;
            }
        }
        return true;
    }

    // Returns the host of the innermost IP endpoint underlying an endpoint, or nullopt when no such endpoint
    // exists: the transport is not IP based (such as Bluetooth), or it is unknown to this communicator. An
    // unknown transport, including an IP-based one whose plug-in is not loaded, is opaque and carries no typed
    // endpoint info, so it is indistinguishable from a non-IP transport here.
    static optional<string> ipEndpointHost(const EndpointPtr& endpoint)
    {
        for (EndpointInfoPtr info = endpoint->getInfo(); info; info = info->underlying)
        {
            if (auto ipInfo = dynamic_pointer_cast<IPEndpointInfo>(info))
            {
                return ipInfo->host;
            }
        }
        return nullopt;
    }

    // Returns true when an endpoint of the proxy has a host that the address rules cannot match reliably:
    // - A non-IP endpoint (such as Bluetooth), or an endpoint whose transport is unknown to this communicator.
    // - An empty host.
    // - A host containing a control character or a space.
    // - A host longer than 255 characters. No legal host name or IP address is that long.
    // - A non-canonical spelling of an IPv4 address, such as 0x7f000001, 2130706433, 0177.0.0.1, or 127.1.
    // - An IPv4 address with a trailing dot: the resolver reads it as a DNS name, not as an address.
    // A DNS name with a trailing dot is fine: the resolver treats 'name.' as 'name', and the matching strips
    // the dot the same way.
    // Returns false for an indirect proxy: it has no endpoints, so there is no host to check here (the address
    // rules match nothing, and the identity and adapter-id filters constrain it instead).
    static bool hasDisallowedHost(const ObjectPrx& proxy)
    {
        for (const auto& endpoint : proxy->ice_getEndpoints())
        {
            optional<string> optHost = ipEndpointHost(endpoint);
            if (!optHost)
            {
                return true;
            }
            string host = std::move(*optHost);

            if (host.empty())
            {
                return true;
            }
            if (any_of(host.begin(), host.end(), [](unsigned char c) { return c == ' ' || iscntrl(c) != 0; }))
            {
                return true;
            }
            if (host.size() > 255)
            {
                return true;
            }
            bool trailingDot = host.size() > 1 && host.back() == '.';
            if (trailingDot)
            {
                host.pop_back();
            }
            if (isIPv4Numeral(host) && (trailingDot || !isCanonicalIPv4Spelling(host)))
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
    class AddressRule final : public Glacier2::ProxyRule
    {
    public:
        AddressRule(
            CommunicatorPtr communicator,
            const vector<AddressMatcher*>& address,
            MatchesNumber* port,
            const int traceLevel,
            bool matchAnyEndpoint)
            : _communicator(std::move(communicator)),
              _addressRules(address),
              _portMatcher(port),
              _traceLevel(traceLevel),
              _matchAnyEndpoint(matchAnyEndpoint)
        {
        }

        ~AddressRule() override
        {
            for (const auto& addressRule : _addressRules)
            {
                delete addressRule;
            }
            delete _portMatcher;
        }

        [[nodiscard]] bool check(const ObjectPrx& prx) const override
        {
            EndpointSeq endpoints = prx->ice_getEndpoints();
            if (endpoints.size() == 0)
            {
                return false;
            }
            if (_matchAnyEndpoint)
            {
                for (const auto& endpoint : endpoints)
                {
                    if (matchEndpoint(endpoint))
                    {
                        return true;
                    }
                }
                return false;
            }
            else
            {
                for (const auto& endpoint : endpoints)
                {
                    if (!matchEndpoint(endpoint))
                    {
                        return false;
                    }
                }
                return true;
            }
        }

        void dump() const
        {
            consoleErr << "address(";
            for (const auto& rule : _addressRules)
            {
                consoleErr << rule->toString() << " ";
            }
            if (_portMatcher != nullptr)
            {
                consoleErr << "):port(" << _portMatcher->toString() << " ";
            }
            consoleErr << ")" << endl;
        }

    private:
        // Matches the host and port of endpoint against this rule.
        [[nodiscard]] bool matchEndpoint(const EndpointPtr& endpoint) const
        {
            string info = endpoint->toString();
            string host;
            if (!extractPart("-h ", info, host))
            {
                return false;
            }
            // Host names are case-insensitive; the rule was folded to lower case when parsed.
            host = toLower(host);

            // The resolver treats the fully-qualified name 'name.' as 'name'; strip the trailing dot so the
            // spelling matches the rules written for 'name'.
            if (host.size() > 1 && host.back() == '.')
            {
                host.pop_back();
            }

            string port;
            if (!extractPart("-p ", info, port))
            {
                return false;
            }

            string::size_type pos = 0;
            if (_portMatcher && !_portMatcher->match(port, pos))
            {
                if (_traceLevel >= 3)
                {
                    Trace out(_communicator->getLogger(), "Glacier2");
                    out << _portMatcher->toString() << " failed to match " << port << " at pos=" << pos << "\n";
                }
                return false;
            }

            vector<bool> failed(_addressRules.size() * (host.size() + 1), false);
            return matchAddress(host, 0, 0, failed);
        }

        // Matches host against the matchers at position index and up, starting at position pos in host. The
        // matchers must match the remainder of the host in full. When they don't, this function retries the
        // matchers that can match at a later position (the matchers created for the portion of a rule that
        // follows a wildcard) until every later alignment is exhausted. The same (index, pos) state can be
        // reached through many alignments of the preceding wildcards; failed is a matcher-count by
        // (host length + 1) matrix that records failed states so each one is evaluated at most once, keeping
        // the matching cost polynomial in the host length.
        [[nodiscard]] bool matchAddress(
            const string& host,
            vector<AddressMatcher*>::size_type index,
            string::size_type pos,
            vector<bool>& failed) const
        {
            if (index == _addressRules.size())
            {
                // The rule must match the whole host, not just a prefix of it.
                if (pos != host.size())
                {
                    if (_traceLevel >= 3)
                    {
                        Trace out(_communicator->getLogger(), "Glacier2");
                        out << "matched a prefix of " << host << " only, up to pos=" << pos << "\n";
                    }
                    return false;
                }
                return true;
            }

            vector<bool>::size_type state = index * (host.size() + 1) + pos;
            if (failed[state])
            {
                return false;
            }

            AddressMatcher* rule = _addressRules[index];
            string::size_type next = pos;
            bool matched = rule->match(host, next);
            while (matched)
            {
                if (_traceLevel >= 3)
                {
                    Trace out(_communicator->getLogger(), "Glacier2");
                    out << rule->toString() << " matched " << host << " at pos=" << next << "\n";
                }
                if (matchAddress(host, index + 1, next, failed))
                {
                    return true;
                }
                matched = rule->retry(host, next);
            }

            if (_traceLevel >= 3)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << rule->toString() << " failed to match " << host << " at pos=" << pos << "\n";
            }
            failed[state] = true;
            return false;
        }

        const CommunicatorPtr _communicator;
        vector<AddressMatcher*> _addressRules;
        MatchesNumber* _portMatcher;
        const int _traceLevel;
        const bool _matchAnyEndpoint;
    };

    static void parseProperty(
        const shared_ptr<Ice::Communicator>& communicator,
        const string& property,
        vector<ProxyRule*>& rules,
        const int traceLevel,
        bool matchAnyEndpoint)
    {
        StartFactory startsWithFactory;
        WildCardFactory wildCardFactory;
        EndsWithFactory endsWithFactory;
        FollowingFactory followingFactory;
        vector<ProxyRule*> allRules;
        try
        {
            istringstream propertyInput(property);

            while (!propertyInput.eof() && propertyInput.good())
            {
                MatchesNumber* portMatch = nullptr;
                vector<AddressMatcher*> currentRuleSet;

                string parameter;
                ws(propertyInput);
                propertyInput >> parameter;

                string portInfo;
                string::size_type portPortion = parameter.find(':');

                string addr;
                if (portPortion != string::npos)
                {
                    addr = parameter.substr(0, portPortion);
                    string port = parameter.substr(portPortion + 1);
                    string::size_type openBracket = port.find('[');
                    if (openBracket != string::npos)
                    {
                        ++openBracket;
                        string::size_type closeBracket = port.find(']', openBracket);
                        if (closeBracket == string::npos)
                        {
                            throw invalid_argument("unclosed group");
                        }
                        port = port.substr(openBracket, closeBracket - openBracket);
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

                // Host names are case-insensitive; fold the rule to lower case and match it against the
                // lower-cased host.
                addr = toLower(addr);

                // The trailing dot of a fully-qualified name is stripped from the host before matching; strip
                // it from the rule the same way.
                if (addr.size() > 1 && addr.back() == '.')
                {
                    addr.pop_back();
                }

                //
                // The addr portion can contain alphanumerics, * and
                // ranges.
                //
                string::size_type current = 0;

                if (current == addr.size())
                {
                    throw invalid_argument("expected address information before ':'");
                }

                //
                // TODO: assuming that there is no leading or trailing whitespace. This
                // should probably be confirmed.
                //
                assert(!isspace(static_cast<unsigned char>(parameter[current])));
                assert(!isspace(static_cast<unsigned char>(addr[addr.size() - 1])));

                if (current != 0)
                {
                    addr = addr.substr(current);
                }

                string::size_type mark = 0;
                bool inGroup = false;
                AddressMatcherFactory* currentFactory = &startsWithFactory;

                if (addr == "*")
                {
                    //
                    // Special case. Match everything.
                    //
                    currentRuleSet.push_back(new MatchesAny);
                }
                else
                {
                    for (current = 0; current < addr.size(); ++current)
                    {
                        if (addr[current] == '*')
                        {
                            if (inGroup)
                            {
                                throw invalid_argument("wildcards not permitted in groups");
                            }
                            //
                            // current == mark when the wildcard is at the head of a
                            // string or directly after a group.
                            //
                            if (current != mark)
                            {
                                currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                            }
                            currentFactory = &wildCardFactory;
                            mark = current + 1;
                        }
                        else if (addr[current] == '[')
                        {
                            if (inGroup)
                            {
                                throw invalid_argument("unexpected '[' in group");
                            }
                            // ??? what does it mean if current == mark?
                            if (current != mark)
                            {
                                currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                                currentFactory = &followingFactory;
                            }
                            inGroup = true;
                            mark = current + 1;
                        }
                        else if (addr[current] == ']')
                        {
                            if (!inGroup)
                            {
                                throw invalid_argument("group close without group start");
                            }
                            inGroup = false;
                            if (mark == current)
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
                    if (inGroup)
                    {
                        throw invalid_argument("unclosed group");
                    }

                    if (mark != current)
                    {
                        // A trailing string matches the end of the host only when a wildcard precedes it. Otherwise
                        // it must match at the position the preceding matchers stopped at.
                        if (currentFactory == &wildCardFactory)
                        {
                            currentFactory = &endsWithFactory;
                        }
                        currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                    }
                    else if (currentFactory == &wildCardFactory)
                    {
                        // A trailing wildcard matches the remainder of the host.
                        currentRuleSet.push_back(new MatchesAny);
                    }
                }
                allRules.push_back(
                    new AddressRule(communicator, currentRuleSet, portMatch, traceLevel, matchAnyEndpoint));
            }
        }
        catch (...)
        {
            for (const auto& allRule : allRules)
            {
                delete allRule;
            }
            throw;
        }
        rules = allRules;
    }

    //
    // Helper function for checking a rule set.
    //
    static bool match(const vector<ProxyRule*>& rules, const ObjectPrx& proxy)
    {
        for (auto rule : rules)
        {
            if (rule->check(proxy))
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
        ProxyLengthRule(CommunicatorPtr communicator, int count, int traceLevel)
            : _communicator(std::move(communicator)),
              _traceLevel(traceLevel),
              _count(count)
        {
        }

        [[nodiscard]] bool check(const ObjectPrx& p) const override
        {
            string s = p->ice_toString();
            bool result = (s.size() > static_cast<size_t>(_count));
            if (_traceLevel >= 1)
            {
                Trace out(_communicator->getLogger(), "Glacier2");
                out << p << (result ? " exceeds " : " meets ") << "proxy size restriction\n";
            }
            return result;
        }

    private:
        const CommunicatorPtr _communicator;
        const int _traceLevel;
        const int _count;
    };

} // End proxy rule implementations.

Glacier2::ProxyVerifier::ProxyVerifier(CommunicatorPtr communicator, int traceLevel)
    : _communicator(std::move(communicator)),
      _traceLevel(traceLevel)
{
    //
    // Evaluation order is dependant on how the rules are stored to the
    // rules vectors.
    //
    string s = _communicator->getProperties()->getIceProperty("Glacier2.Filter.Address.Accept");
    if (s != "")
    {
        _hasAddressFilters = true;
        try
        {
            // An accept rule matches a proxy only when every endpoint matches it.
            Glacier2::parseProperty(_communicator, s, _acceptRules, _traceLevel, false);
        }
        catch (const exception& ex)
        {
            ostringstream os;
            os << "invalid 'Glacier2.Filter.Address.Accept' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    s = _communicator->getProperties()->getIceProperty("Glacier2.Filter.Address.Reject");
    if (s != "")
    {
        _hasAddressFilters = true;
        try
        {
            // A reject rule matches a proxy as soon as one endpoint matches it.
            Glacier2::parseProperty(_communicator, s, _rejectRules, _traceLevel, true);
        }
        catch (const exception& ex)
        {
            ostringstream os;
            os << "invalid 'Glacier2.Filter.Address.Reject' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    int proxySizeMax = _communicator->getProperties()->getIcePropertyAsInt("Glacier2.Filter.ProxySizeMax");
    if (proxySizeMax < 0)
    {
        throw InitializationException(
            __FILE__,
            __LINE__,
            "invalid value for Glacier2.Filter.ProxySizeMax: " + to_string(proxySizeMax));
    }
    if (proxySizeMax > 0)
    {
        _rejectRules.push_back(new ProxyLengthRule(_communicator, proxySizeMax, _traceLevel));
    }
}

Glacier2::ProxyVerifier::~ProxyVerifier()
{
    for (const auto& acceptRule : _acceptRules)
    {
        delete acceptRule;
    }
    for (const auto& rejectRule : _rejectRules)
    {
        delete rejectRule;
    }
}

bool
Glacier2::ProxyVerifier::verify(const ObjectPrx& proxy)
{
    //
    // No rules have been defined so we accept all.
    //
    if (_acceptRules.size() == 0 && _rejectRules.size() == 0)
    {
        return true;
    }

    bool result = false;

    if (_hasAddressFilters && Glacier2::hasDisallowedHost(proxy))
    {
        // Rejected regardless of the configured rules.
    }
    else if (_rejectRules.size() == 0)
    {
        //
        // If there are no reject rules, we assume "reject all".
        //
        result = match(_acceptRules, proxy);
    }
    else if (_acceptRules.size() == 0)
    {
        //
        // If no accept rules are defined we assume accept all.
        //
        result = !match(_rejectRules, proxy);
    }
    else
    {
        if (match(_acceptRules, proxy))
        {
            result = !match(_rejectRules, proxy);
        }
    }

    //
    // The proxy rules take care of the tracing for higher trace levels.
    //
    if (_traceLevel > 0)
    {
        Trace out(_communicator->getLogger(), "Glacier2");
        if (result)
        {
            out << "accepted proxy " << proxy << '\n';
        }
        else
        {
            out << "rejected proxy " << proxy << '\n';
        }
    }
    return result;
}
