//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/HashMap");
require("../Ice/IdentityUtil");
require("../Ice/LocalException");
require("../Ice/StringUtil");

const Debug = Ice.Debug;
const HashMap = Ice.HashMap;
const StringUtil = Ice.StringUtil;

//
// Only for use by Ice.ObjectAdatperI.
//
class ServantManager
{
    constructor(instance, adapterName)
    {
        this._instance = instance;
        this._adapterName = adapterName;
        // Map<Ice.Identity, Map<String, Ice.Object> >
        this._servantMapMap = new HashMap(HashMap.compareEquals);
        // Map<String, Ice.Object>
        this._defaultServantMap = new Map();
        // Map<String, Ice.ServantLocator>
        this._locatorMap = new Map();
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
        else if(m.has(facet))
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

        for(const [key, locator] of locatorMap)
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
