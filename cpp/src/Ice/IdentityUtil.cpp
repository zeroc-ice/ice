// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/IdentityUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

ostream&
Ice::operator<<(ostream& out, const Identity& ident)
{
    return out << identityToString(ident);
}

Identity
Ice::stringToIdentity(const string& s)
{
    Identity ident;
    string::size_type pos = s.find_first_of("/");
    if (pos != string::npos)
    {
	ident.category = s.substr(0, pos);
	ident.name = s.substr(pos + 1);
    }
    else
    {
	ident.name = s;
    }
    return ident;
}

string
Ice::identityToString(const Identity& ident)
{
    if (ident.category.empty())
    {
	return ident.name;
    }
    else
    {
	return ident.category + '/' + ident.name;
    }
}
