// Copyright (c) ZeroC, Inc.

#include "Ice/ProxyFunctions.h"
#include "Ice/Communicator.h"
#include "Ice/Current.h"
#include "Ice/ObjectAdapter.h"
#include "Reference.h"
#include "TargetCompare.h"

#include <sstream>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::throwNullProxyMarshalException(const char* file, int line, const Current& current)
{
    ostringstream os;
    os << "null proxy passed to " << current.operation << " on object "
       << current.adapter->getCommunicator()->identityToString(current.id);
    throw MarshalException{file, line, os.str()};
}

bool
Ice::operator<(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept
{
    return targetLess(lhs._getReference(), rhs._getReference());
}

bool
Ice::operator==(const ObjectPrx& lhs, const ObjectPrx& rhs) noexcept
{
    return targetEqualTo(lhs._getReference(), rhs._getReference());
}

bool
Ice::proxyIdentityLess(const optional<ObjectPrx>& lhs, const optional<ObjectPrx>& rhs) noexcept
{
    return lhs && rhs ? lhs->ice_getIdentity() < rhs->ice_getIdentity()
                      : std::less<>()(lhs.has_value(), rhs.has_value());
}

bool
Ice::proxyIdentityEqual(const optional<ObjectPrx>& lhs, const optional<ObjectPrx>& rhs) noexcept
{
    return lhs && rhs ? lhs->ice_getIdentity() == rhs->ice_getIdentity()
                      : std::equal_to<>()(static_cast<bool>(lhs), static_cast<bool>(rhs));
}

bool
Ice::proxyIdentityAndFacetLess(const optional<ObjectPrx>& lhs, const optional<ObjectPrx>& rhs) noexcept
{
    if (lhs && rhs)
    {
        Identity lhsIdentity = lhs->ice_getIdentity();
        Identity rhsIdentity = rhs->ice_getIdentity();

        if (lhsIdentity < rhsIdentity)
        {
            return true;
        }
        else if (rhsIdentity < lhsIdentity)
        {
            return false;
        }

        string lhsFacet = lhs->ice_getFacet();
        string rhsFacet = rhs->ice_getFacet();

        return lhsFacet < rhsFacet;
    }
    else
    {
        return std::less<>()(static_cast<bool>(lhs), static_cast<bool>(rhs));
    }
}

bool
Ice::proxyIdentityAndFacetEqual(const optional<ObjectPrx>& lhs, const optional<ObjectPrx>& rhs) noexcept
{
    if (lhs && rhs)
    {
        return lhs->ice_getIdentity() == rhs->ice_getIdentity() && lhs->ice_getFacet() == rhs->ice_getFacet();
    }
    else
    {
        return std::equal_to<>()(static_cast<bool>(lhs), static_cast<bool>(rhs));
    }
}
