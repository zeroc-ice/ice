// Copyright (c) ZeroC, Inc.

#include "DistinguishedName.h"
#include "RFC2253.h"

#include <sstream>

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

DistinguishedName::DistinguishedName(const string& dn) : _rdns(RFC2253::parseStrict(dn)) { unescape(); }

DistinguishedName::DistinguishedName(const list<pair<string, string>>& rdns) : _rdns(rdns) { unescape(); }

namespace Ice::SSL
{
    bool operator==(const DistinguishedName& lhs, const DistinguishedName& rhs)
    {
        return lhs._unescaped == rhs._unescaped;
    }

    bool operator<(const DistinguishedName& lhs, const DistinguishedName& rhs)
    {
        return lhs._unescaped < rhs._unescaped;
    }
}

bool
DistinguishedName::match(const DistinguishedName& other) const
{
    for (const auto& p : other._unescaped)
    {
        bool found = false;
        for (const auto& q : _unescaped)
        {
            if (p.first == q.first)
            {
                found = true;
                if (p.second != q.second)
                {
                    return false;
                }
            }
        }
        if (!found)
        {
            return false;
        }
    }
    return true;
}

bool
DistinguishedName::match(const string& other) const
{
    return match(DistinguishedName(other));
}

//
// This always produces the same output as the input DN -- the type of
// escaping is not changed.
//
std::string
DistinguishedName::toString() const
{
    ostringstream os;
    bool first = true;
    for (const auto& rdn : _rdns)
    {
        if (!first)
        {
            os << ",";
        }
        first = false;
        os << rdn.first << "=" << rdn.second;
    }
    return os.str();
}

void
DistinguishedName::unescape()
{
    for (auto rdn : _rdns)
    {
        rdn.second = RFC2253::unescape(rdn.second);
        _unescaped.push_back(rdn);
    }
}
