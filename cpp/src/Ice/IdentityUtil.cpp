// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/IdentityUtil.h>
#include <Ice/StringUtil.h>
#include <Ice/LocalException.h>

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

    //
    // Find unescaped separator
    //
    string::size_type slash = 0;
    while((slash = s.find('/', slash)) != string::npos)
    {
        if(slash == 0 || s[slash - 1] != '\\')
        {
            break;
        }
        slash++;
    }

    if(slash == string::npos)
    {
        if(!decodeString(s, 0, 0, ident.name))
        {
            throw SystemException(__FILE__, __LINE__);
        }
    }
    else
    {
        if(!decodeString(s, 0, slash, ident.category))
        {
            throw SystemException(__FILE__, __LINE__);
        }
        if(slash + 1 < s.size())
        {
            if(!decodeString(s, slash + 1, 0, ident.name))
            {
                throw SystemException(__FILE__, __LINE__);
            }
        }
    }

    return ident;
}

string
Ice::identityToString(const Identity& ident)
{
    if(ident.category.empty())
    {
	return encodeString(ident.name, "/");
    }
    else
    {
	return encodeString(ident.category, "/") + '/' + encodeString(ident.name, "/");
    }
}
