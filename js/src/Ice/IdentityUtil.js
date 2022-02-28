//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Identity");
require("../Ice/LocalException");
require("../Ice/StringUtil");

const Identity = Ice.Identity;
const IdentityParseException = Ice.IdentityParseException;
const StringUtil = Ice.StringUtil;

/**
* Converts a string to an object identity.
*
* @param s The string to convert.
*
* @return The converted object identity.
**/
Ice.stringToIdentity = function(s)
{
    const ident = new Identity();

    //
    // Find unescaped separator; note that the string may contain an escaped
    // backslash before the separator.
    //
    let slash = -1;
    let pos = 0;
    while((pos = s.indexOf('/', pos)) !== -1)
    {
        let escapes = 0;
        while(pos - escapes > 0 && s.charAt(pos - escapes - 1) == '\\')
        {
            escapes++;
        }

        //
        // We ignore escaped escapes
        //
        if(escapes % 2 === 0)
        {
            if(slash == -1)
            {
                slash = pos;
            }
            else
            {
                //
                // Extra unescaped slash found.
                //
                throw new IdentityParseException(`unescaped backslash in identity \`${s}'`);
            }
        }
        pos++;
    }

    if(slash == -1)
    {
        ident.category = "";
        try
        {
            ident.name = StringUtil.unescapeString(s, 0, s.length, "/");
        }
        catch(e)
        {
            throw new IdentityParseException(`invalid identity name \`${s}': ${e.toString()}`);
        }
    }
    else
    {
        try
        {
            ident.category = StringUtil.unescapeString(s, 0, slash, "/");
        }
        catch(e)
        {
            throw new IdentityParseException(`invalid category in identity \`${s}': ${e.toString()}`);
        }
        if(slash + 1 < s.length)
        {
            try
            {
                ident.name = StringUtil.unescapeString(s, slash + 1, s.length, "/");
            }
            catch(e)
            {
                throw new IdentityParseException(`invalid name in identity \`${s}': ${e.toString()}`);
            }
        }
        else
        {
            ident.name = "";
        }
    }

    return ident;
};

/**
* Converts an object identity to a string.
*
* @param ident The object identity to convert.
*
* @param toStringMode Specifies if and how non-printable ASCII characters are escaped in the result.
*
* @return The string representation of the object identity.
**/
Ice.identityToString = function(ident, toStringMode = Ice.ToStringMode.Unicode)
{
    if(ident.category === null || ident.category.length === 0)
    {
        return StringUtil.escapeString(ident.name, "/", toStringMode);
    }
    else
    {
        return StringUtil.escapeString(ident.category, "/", toStringMode) + '/' + StringUtil.escapeString(ident.name, "/", toStringMode);
    }
};

/**
* Compares the object identities of two proxies.
*
* @param lhs A proxy.
* @param rhs A proxy.
* @return -1 if the identity in <code>lhs</code> compares
* less than the identity in <code>rhs</code>; 0 if the identities
* compare equal; 1, otherwise.
*
* @see ProxyIdentityKey
* @see ProxyIdentityAndFacetKey
* @see ProxyIdentityAndFacetCompare
**/
Ice.proxyIdentityCompare = function(lhs, rhs)
{
    if(lhs === rhs)
    {
        return 0;
    }
    else if(lhs === null && rhs !== null)
    {
        return -1;
    }
    else if(lhs !== null && rhs === null)
    {
        return 1;
    }
    else
    {
        const lhsIdentity = lhs.ice_getIdentity();
        const rhsIdentity = rhs.ice_getIdentity();
        const n = lhsIdentity.name.localeCompare(rhsIdentity.name);
        return (n !== 0) ? n : lhsIdentity.category.localeCompare(rhsIdentity.category);
    }
};

/**
* Compares the object identities and facets of two proxies.
*
* @param lhs A proxy.
* @param rhs A proxy.
* @return -1 if the identity and facet in <code>lhs</code> compare
* less than the identity and facet in <code>rhs</code>; 0 if the identities
* and facets compare equal; 1, otherwise.
*
* @see ProxyIdentityAndFacetKey
* @see ProxyIdentityKey
* @see ProxyIdentityCompare
**/
Ice.proxyIdentityAndFacetCompare = function(lhs, rhs)
{
    if(lhs === rhs)
    {
        return 0;
    }
    else if(lhs === null && rhs !== null)
    {
        return -1;
    }
    else if(lhs !== null && rhs === null)
    {
        return 1;
    }
    else
    {
        const lhsIdentity = lhs.ice_getIdentity();
        const rhsIdentity = rhs.ice_getIdentity();
        let n = lhsIdentity.name.localeCompare(rhsIdentity.name);
        if(n !== 0)
        {
            return n;
        }
        n = lhsIdentity.category.localeCompare(rhsIdentity.category);
        if(n !== 0)
        {
            return n;
        }

        const lhsFacet = lhs.ice_getFacet();
        const rhsFacet = rhs.ice_getFacet();
        if(lhsFacet === null && rhsFacet === null)
        {
            return 0;
        }
        else if(lhsFacet === null)
        {
            return -1;
        }
        else if(rhsFacet === null)
        {
            return 1;
        }
        return lhsFacet.localeCompare(rhsFacet);
    }
};

module.exports.Ice = Ice;
