// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/ExUtil");
    require("Ice/StringUtil");
    require("Ice/Identity");
    require("Ice/LocalException");
    
    var Ice = global.Ice || {};
    
    var ExUtil = Ice.ExUtil;
    var StringUtil = Ice.StringUtil;
    var Identity = Ice.Identity;
    var IdentityParseException = Ice.IdentityParseException;

    Ice = global.Ice || {};
    /**
    * Converts a string to an object identity.
    *
    * @param s The string to convert.
    *
    * @return The converted object identity.
    **/
    Ice.stringToIdentity = function(s)
    {
        var ident = new Identity();

        //
        // Find unescaped separator.
        //
        var slash = -1;
        var pos = 0;
        while((pos = s.indexOf('/', pos)) !== -1)
        {
            if(pos === 0 || s.charAt(pos - 1) != '\\')
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
                    var ex = new IdentityParseException();
                    ex.str = "unescaped backslash in identity `" + s + "'";
                    throw ex;
                }
            }
            pos++;
        }

        if(slash == -1)
        {
            ident.category = "";
            try
            {
                ident.name = StringUtil.unescapeString(s);
            }
            catch(e)
            {
                var ex = new IdentityParseException();
                ex.str = "invalid identity name `" + s + "': " + ExUtil.toString(e);
                throw ex;
            }
        }
        else
        {
            try
            {
                ident.category = StringUtil.unescapeString(s, 0, slash);
            }
            catch(e)
            {
                var ex = new IdentityParseException();
                ex.str = "invalid category in identity `" + s + "': " + ExUtil.toString(e);
                throw ex;
            }
            if(slash + 1 < s.length)
            {
                try
                {
                    ident.name = StringUtil.unescapeString(s, slash + 1, s.length);
                }
                catch(e)
                {
                    var ex = new IdentityParseException();
                    ex.str = "invalid name in identity `" + s + "': " + ExUtil.toString(e);
                    throw ex;
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
    * @return The string representation of the object identity.
    **/
    Ice.identityToString = function(ident)
    {
        if(ident.category === null || ident.category.length === 0)
        {
            return StringUtil.escapeString(ident.name, "/");
        }
        else
        {
            return StringUtil.escapeString(ident.category, "/") + '/' + StringUtil.escapeString(ident.name, "/");
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
            var lhsIdentity = lhs.ice_getIdentity();
            var rhsIdentity = rhs.ice_getIdentity();
            var n;
            if((n = lhsIdentity.name.localeCompare(rhsIdentity.name)) !== 0)
            {
                return n;
            }
            return lhsIdentity.category.localeCompare(rhsIdentity.category);
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
            var lhsIdentity = lhs.ice_getIdentity();
            var rhsIdentity = rhs.ice_getIdentity();
            var n;
            if((n = lhsIdentity.name.localeCompare(rhsIdentity.name)) !== 0)
            {
                return n;
            }
            if((n = lhsIdentity.category.localeCompare(rhsIdentity.category)) !== 0)
            {
                return n;
            }

            var lhsFacet = lhs.ice_getFacet();
            var rhsFacet = rhs.ice_getFacet();
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
    
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
