// Copyright (c) ZeroC, Inc.

#include "ProxyVerifier.h"
#include "../Ice/ConsoleUtil.h"

#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
using namespace Ice;
using namespace IceInternal;

// TODO: Some of the address matching helper classes can probably be refactored out. It's a question of whether to keep
// really simple differences refactored into separate classes or go to fewer but slightly smarter classes.

// Proxy rule implementations.
namespace Glacier2
{
    // A numeric range that can be encoded in a filter.
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

    // Base class for address matching operations.
    class AddressMatcher
    {
    public:
        virtual ~AddressMatcher() = default;

        virtual bool match(const string&, string::size_type& pos) = 0;
        [[nodiscard]] virtual const char* toString() const = 0;
    };

    class MatchesAny final : public AddressMatcher
    {
    public:
        bool match(const string&, string::size_type&) override { return true; }

        [[nodiscard]] const char* toString() const override { return "(ANY)"; }
    };

    // Match the start of a string (i.e. position == 0). Occurs when filter string starts with a set of characters
    // followed by a wildcard or numeric range.
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

    // Match the end portion of a string. Occurs when a filter string starts with a wildcard or numeric range, but ends
    // with a string.
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
            if (strncmp(space.c_str(), _criteria.c_str(), _criteria.size()) == 0)
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

    // Match against somewhere within a string. Occurs when a filter contains a string bounded by wildcards, or numeric
    // ranges. e.g. *bar*.com.
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
            pos = offset + _criteria.size() + 1;
            return true;
        }

        [[nodiscard]] const char* toString() const override { return _description.c_str(); }

    private:
        string _criteria;
        string _description;
    };

    // Match a number against a range of values. This occurs when the filter contains a numeric range or group of
    // numeric values. e.g. foo[1-3, 10].bar.com. Also used to match port numbers and ranges.
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
            ostr << ends;
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
            pos += static_cast<string::size_type>(istr.tellg());
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

    // Occurs when a numeric range is preceded by a wildcard.
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
            }
            return false;
        }
    };

    class EndsWithNumber final : public MatchesNumber
    {
    public:
        EndsWithNumber(const vector<int>& values, const vector<Range>& ranges)
            : MatchesNumber(values, ranges, "ends with ")
        {
        }

        bool match(const string& space, string::size_type& pos) override
        {
            pos = space.find_last_not_of("0123456789", pos);
            if (pos == space.size() - 1)
            {
                return false;
            }

            return MatchesNumber::match(space, pos);
        }
    };

    // AddressMatcher factories abstract away the logic of which matching objects need to be created depending on the
    // state of the filter string parsing. Similar to changing a tool that produces the right result depending on how
    // far along you are in the job, the factories are selected according to what transition has occurred while parsing
    // the filter string.
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

        AddressMatcher* create(const vector<int>& ports, const vector<Range>& ranges) override
        {
            return new EndsWithNumber(ports, ranges);
        }
    };

    // A proxy validation rule encapsulating an address filter.
    class AddressRule final : public Glacier2::ProxyRule
    {
    public:
        AddressRule(
            CommunicatorPtr communicator,
            const vector<AddressMatcher*>& address,
            MatchesNumber* port,
            const int traceLevel)
            : _communicator(std::move(communicator)),
              _addressRules(address),
              _portMatcher(port),
              _traceLevel(traceLevel)
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
            for (const auto& endpoint : endpoints)
            {
                string info = endpoint->toString();
                string host;
                if (!extractPart("-h ", info, host))
                {
                    return false;
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

                pos = 0;
                for (const auto& rule : _addressRules)
                {
                    if (!rule->match(host, pos))
                    {
                        if (_traceLevel >= 3)
                        {
                            Trace out(_communicator->getLogger(), "Glacier2");
                            out << rule->toString() << " failed to match " << host << " at pos=" << pos << "\n";
                        }
                        return false;
                    }
                    if (_traceLevel >= 3)
                    {
                        Trace out(_communicator->getLogger(), "Glacier2");
                        out << rule->toString() << " matched " << host << " at pos=" << pos << "\n";
                    }
                }
            }

            return true;
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
        bool extractPart(const char* opt, const string& source, string& result) const
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

        const CommunicatorPtr _communicator;
        vector<AddressMatcher*> _addressRules;
        MatchesNumber* _portMatcher;
        const int _traceLevel;
    };

    static void parseProperty(
        const shared_ptr<Ice::Communicator>& communicator,
        const string& property,
        vector<ProxyRule*>& rules,
        const int traceLevel)
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

                // The addr portion can contain alphanumerics, * and ranges.
                string::size_type current = 0;

                if (current == addr.size())
                {
                    throw invalid_argument("expected address information before ':'");
                }

                // TODO: assuming that there is no leading or trailing whitespace. This should probably be confirmed.
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
                    // Special case. Match everything.
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
                            // current == mark when the wildcard is at the head of a string or directly after a group.
                            if (current != mark)
                            {
                                currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                            }
                            currentFactory = &wildCardFactory;
                            mark = current + 1;
                        }
                        else if (addr[current] == '[')
                        {
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
                    currentFactory = &endsWithFactory;

                    if (inGroup)
                    {
                        throw invalid_argument("unclosed group");
                    }
                    if (mark != current)
                    {
                        currentRuleSet.push_back(currentFactory->create(addr.substr(mark, current - mark)));
                    }
                }
                allRules.push_back(new AddressRule(communicator, currentRuleSet, portMatch, traceLevel));
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

    // Helper function for checking a rule set.
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

    // ProxyLengthRule returns 'true' if the string form of the proxy exceeds the configured length.
    class ProxyLengthRule : public ProxyRule
    {
    public:
        ProxyLengthRule(CommunicatorPtr communicator, const string& count, int traceLevel)
            : _communicator(std::move(communicator)),
              _traceLevel(traceLevel)
        {
            istringstream s(count);
            if (!(s >> _count) || !s.eof())
            {
                throw invalid_argument("Error parsing ProxySizeMax property");
            }
            if (_count <= 0)
            {
                throw invalid_argument("ProxySizeMax must be greater than 1");
            }
        }

        [[nodiscard]] bool check(const ObjectPrx& p) const override
        {
            string s = p->ice_toString();
            bool result = (s.size() > _count);
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
        unsigned long _count;
    };

} // End proxy rule implementations.

Glacier2::ProxyVerifier::ProxyVerifier(CommunicatorPtr communicator)
    : _communicator(std::move(communicator)),
      _traceLevel(_communicator->getProperties()->getIcePropertyAsInt("Glacier2.Client.Trace.Reject"))
{
    // Evaluation order is dependant on how the rules are stored to the rules vectors.
    string s = _communicator->getProperties()->getIceProperty("Glacier2.Filter.Address.Accept");
    if (s != "")
    {
        try
        {
            Glacier2::parseProperty(_communicator, s, _acceptRules, _traceLevel);
        }
        catch (const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.Address.Accept' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    s = _communicator->getProperties()->getIceProperty("Glacier2.Filter.Address.Reject");
    if (s != "")
    {
        try
        {
            Glacier2::parseProperty(_communicator, s, _rejectRules, _traceLevel);
        }
        catch (const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.Address.Reject' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
    }

    s = _communicator->getProperties()->getIceProperty("Glacier2.Filter.ProxySizeMax");
    if (s != "")
    {
        try
        {
            _rejectRules.push_back(new ProxyLengthRule(_communicator, s, _traceLevel));
        }
        catch (const exception& ex)
        {
            ostringstream os;
            os << "invalid `Glacier2.Filter.ProxySizeMax' property:\n" << ex.what();
            throw InitializationException(__FILE__, __LINE__, os.str());
        }
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
    // No rules have been defined so we accept all.
    if (_acceptRules.size() == 0 && _rejectRules.size() == 0)
    {
        return true;
    }

    bool result = false;

    if (_rejectRules.size() == 0)
    {
        // If there are no reject rules, we assume "reject all".
        result = match(_acceptRules, proxy);
    }
    else if (_acceptRules.size() == 0)
    {
        // If no accept rules are defined we assume accept all.
        result = !match(_rejectRules, proxy);
    }
    else
    {
        if (match(_acceptRules, proxy))
        {
            result = !match(_rejectRules, proxy);
        }
    }

    // The proxy rules take care of the tracing for higher trace levels.
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
