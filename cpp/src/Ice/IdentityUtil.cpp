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
    // Find unescaped separator.
    //
    string::size_type slash = string::npos, pos = 0;
    while((pos = s.find('/', pos)) != string::npos)
    {
        if(pos == 0 || s[pos - 1] != '\\')
        {
            if(slash == string::npos)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
            }
        }
        pos++;
    }

    if(slash == string::npos)
    {
        if(!decodeString(s, 0, s.size(), ident.name))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
    }
    else
    {
        if(!decodeString(s, 0, slash, ident.category))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            if(!decodeString(s, slash + 1, s.size(), ident.name))
            {
                IdentityParseException ex(__FILE__, __LINE__);
                ex.str = s;
                throw ex;
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
