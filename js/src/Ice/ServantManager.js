// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Debug",
        "../Ice/LocalException",
        "../Ice/StringUtil",
        "../Ice/IdentityUtil",
        "../Ice/HashMap"
    ]);

const Debug = Ice.Debug;
const StringUtil = Ice.StringUtil;
const HashMap = Ice.HashMap;

//
// Only for use by Ice.ObjectAdatperI.
//
class ServantManager
{
    constructor(instance, adapterName)
    {
        this._instance = instance;
        this._adapterName = adapterName;
        this._servantMapMap = new HashMap(HashMap.compareEquals);       // Map<Ice.Identity, Map<String, Ice.Object> >
        this._defaultServantMap = new Map();                            // Map<String, Ice.Object>
        this._locatorMap = new Map();                                   // Map<String, Ice.ServantLocator>
    }

    addServant(servant, ident, facet)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        if(facet === null)
        {
            facet = "";
        }

        let m = this._servantMapMap.get(ident);
        if(m === undefined)
        {
            m = new Map();
            this._servantMapMap.set(ident, m);
        }
        else
        {
            if(m.has(facet))
            {
                const ex = new Ice.AlreadyRegisteredException();
                ex.id = Ice.identityToString(ident, this._instance.toStringMode());
                ex.kindOfObject = "servant";
                if(facet.length > 0)
                {
                    ex.id += " -f " + StringUtil.escapeString(facet, "", this._instance.toStringMode());
                }
                throw ex;
            }
        }

        m.set(facet, servant);
    }

    addDefaultServant(servant, category)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction

        if(this._defaultServantMap.has(category))
        {
            const ex = new Ice.AlreadyRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        this._defaultServantMap.set(category, servant);
    }

    removeServant(ident, facet)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        if(facet === null)
        {
            facet = "";
        }

        const m = this._servantMapMap.get(ident);
        if(m === undefined || !m.has(facet))
        {
            const ex = new Ice.NotRegisteredException();
            ex.id = Ice.identityToString(ident, this._instance.toStringMode());
            ex.kindOfObject = "servant";
            if(facet.length > 0)
            {
                ex.id += " -f " + StringUtil.escapeString(facet, "", this._instance.toStringMode());
            }
            throw ex;
        }

        const obj = m.get(facet);
        m.delete(facet);

        if(m.size === 0)
        {
            this._servantMapMap.delete(ident);
        }

        return obj;
    }

    removeDefaultServant(category)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        const obj = this._defaultServantMap.get(category);
        if(obj === undefined)
        {
            const ex = new Ice.NotRegisteredException();
            ex.kindOfObject = "default servant";
            ex.id = category;
            throw ex;
        }

        this._defaultServantMap.delete(category);
        return obj;
    }

    removeAllFacets(ident)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        const m = this._servantMapMap.get(ident);
        if(m === undefined)
        {
            const ex = new Ice.NotRegisteredException();
            ex.id = Ice.identityToString(ident, this._instance.toStringMode());
            ex.kindOfObject = "servant";
            throw ex;
        }

        this._servantMapMap.delete(ident);

        return m;
    }

    findServant(ident, facet)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //Debug.assert(this._instance !== null); // Must not be called after destruction.

        if(facet === null)
        {
            facet = "";
        }

        const m = this._servantMapMap.get(ident);
        let obj = null;
        if(m === undefined)
        {
            obj = this._defaultServantMap.get(ident.category);
            if(obj === undefined)
            {
                obj = this._defaultServantMap.get("");
            }
        }
        else
        {
            obj = m.get(facet);
        }

        return obj === undefined ? null : obj;
    }

    findDefaultServant(category)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        const ds = this._defaultServantMap.get(category);
        return ds === undefined ? null : ds;
    }

    findAllFacets(ident)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        const m = this._servantMapMap.get(ident);
        if(m !== undefined)
        {
            return new Map(m);
        }

        return new Map();
    }

    hasServant(ident)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //Debug.assert(this._instance !== null); // Must not be called after destruction.

        const m = this._servantMapMap.get(ident);
        if(m === undefined)
        {
            return false;
        }
        else
        {
            Debug.assert(m.size > 0);
            return true;
        }
    }

    addServantLocator(locator, category)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        if(this._locatorMap.has(category))
        {
            const ex = new Ice.AlreadyRegisteredException();
            ex.id = StringUtil.escapeString(category, "", this._instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }

        this._locatorMap.set(category, locator);
    }

    removeServantLocator(category)
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.

        const l = this._locatorMap.get(category);
        if(l === undefined)
        {
            const ex = new Ice.NotRegisteredException();
            ex.id = StringUtil.escapeString(category, "", this._instance.toStringMode());
            ex.kindOfObject = "servant locator";
            throw ex;
        }
        this._locatorMap.delete(category);
        return l;
    }

    findServantLocator(category)
    {
        //
        // This assert is not valid if the adapter dispatch incoming
        // requests from bidir connections. This method might be called if
        // requests are received over the bidir connection after the
        // adapter was deactivated.
        //
        //Debug.assert(this._instance !== null); // Must not be called after destruction.

        const l = this._locatorMap.get(category);
        return l === undefined ? null : l;
    }

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    destroy()
    {
        Debug.assert(this._instance !== null); // Must not be called after destruction.
        const logger = this._instance.initializationData().logger;
        this._servantMapMap.clear();

        this._defaultServantMap.clear();

        const locatorMap = new Map(this._locatorMap);
        this._locatorMap.clear();
        this._instance = null;

        for(let [key, locator] of locatorMap)
        {
            try
            {
                locator.deactivate(key);
            }
            catch(ex)
            {
                logger.error("exception during locator deactivation:\nobject adapter: `" +
                             this._adapterName + "'\nlocator category: `" + key + "'\n" +
                             ex.toString());
            }
        }
    }
}

Ice.ServantManager = ServantManager;
module.exports.Ice = Ice;
