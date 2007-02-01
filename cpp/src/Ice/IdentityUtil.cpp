// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IdentityUtil.h>
#include <Ice/LocalException.h>
#include <IceUtil/StringUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

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
        if(!IceUtil::unescapeString(s, 0, s.size(), ident.name))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
    }
    else
    {
        if(!IceUtil::unescapeString(s, 0, slash, ident.category))
        {
            IdentityParseException ex(__FILE__, __LINE__);
            ex.str = s;
            throw ex;
        }
        if(slash + 1 < s.size())
        {
            if(!IceUtil::unescapeString(s, slash + 1, s.size(), ident.name))
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
        return IceUtil::escapeString(ident.name, "/");
    }
    else
    {
        return IceUtil::escapeString(ident.category, "/") + '/' + IceUtil::escapeString(ident.name, "/");
    }
}
