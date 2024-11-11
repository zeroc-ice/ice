//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import { HashMap } from "./HashMap.js";
import { StringUtil } from "./StringUtil.js";
import {
    AlreadyRegisteredException,
    NotRegisteredException,
    ObjectNotExistException,
    FacetNotExistException,
} from "./LocalExceptions.js";
import { identityToString } from "./IdentityToString.js";
import { Debug } from "./Debug.js";

//
// Only for use by Ice.ObjectAdapter.
//
export class ServantManager {
    constructor(instance, adapterName) {
        this._instance = instance;
        this._adapterName = adapterName;
        // Map<Ice.Identity, Map<String, Ice.Object> >
        this._servantMapMap = new HashMap(HashMap.compareEquals);
        // Map<String, Ice.Object>
        this._defaultServantMap = new Map();
        // Map<String, Ice.ServantLocator>
        this._locatorMap = new Map();
    }

    async dispatch(request) {
        const current = request.current;
        let servant = this.findServant(current.id, current.facet);

        if (servant !== null) {
            // the simple, common path
            return await servant.dispatch(request);
        }

        // Else, check servant locators
        let locator = this.findServantLocator(current.id.category);
        if (locator === null && current.id.category.length > 0) {
            locator = this.findServantLocator("");
        }

        if (locator !== null) {
            const cookie = {};
            try {
                servant = locator.locate(current, cookie);
            } catch (ex) {
                // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
                request.inputStream.skipEncapsulation();
                throw ex;
            }

            if (servant !== null) {
                try {
                    return await servant.dispatch(request);
                } finally {
                    locator.finished(current, servant, cookie.value);
                }
            }
        }

        DEV: console.assert(servant === null);

        // Skip the encapsulation. This allows the next batch requests in the same InputStream to proceed.
        request.inputStream.skipEncapsulation();
        if (this.hasServant(current.id)) {
            throw new FacetNotExistException();
        } else {
            throw new ObjectNotExistException();
        }
    }

    addServant(servant, ident, facet) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        if (facet === null) {
            facet = "";
        }

        let m = this._servantMapMap.get(ident);
        if (m === undefined) {
            m = new Map();
            this._servantMapMap.set(ident, m);
        } else if (m.has(facet)) {
            let id = identityToString(ident, this._instance.toStringMode());
            if (facet.length > 0) {
                id += " -f " + StringUtil.escapeString(facet, "", this._instance.toStringMode());
            }
            throw new AlreadyRegisteredException("servant", id);
        }

        m.set(facet, servant);
    }

    addDefaultServant(servant, category) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction

        if (this._defaultServantMap.has(category)) {
            throw new AlreadyRegisteredException("default servant", category);
        }

        this._defaultServantMap.set(category, servant);
    }

    removeServant(ident, facet) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        if (facet === null) {
            facet = "";
        }

        const m = this._servantMapMap.get(ident);
        if (m === undefined || !m.has(facet)) {
            let id = identityToString(ident, this._instance.toStringMode());
            if (facet.length > 0) {
                id += " -f " + StringUtil.escapeString(facet, "", this._instance.toStringMode());
            }
            throw new NotRegisteredException("servant", id);
        }

        const obj = m.get(facet);
        m.delete(facet);

        if (m.size === 0) {
            this._servantMapMap.delete(ident);
        }

        return obj;
    }

    removeDefaultServant(category) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        const obj = this._defaultServantMap.get(category);
        if (obj === undefined) {
            throw new NotRegisteredException("default servant", category);
        }

        this._defaultServantMap.delete(category);
        return obj;
    }

    removeAllFacets(ident) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        const m = this._servantMapMap.get(ident);
        if (m === undefined) {
            throw new NotRegisteredException("servant", identityToString(ident, this._instance.toStringMode()));
        }

        this._servantMapMap.delete(ident);

        return m;
    }

    findServant(ident, facet) {
        if (facet === null) {
            facet = "";
        }

        const m = this._servantMapMap.get(ident);
        let obj = null;
        if (m === undefined) {
            obj = this._defaultServantMap.get(ident.category);
            if (obj === undefined) {
                obj = this._defaultServantMap.get("");
            }
        } else {
            obj = m.get(facet);
        }

        return obj === undefined ? null : obj;
    }

    findDefaultServant(category) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        const ds = this._defaultServantMap.get(category);
        return ds === undefined ? null : ds;
    }

    findAllFacets(ident) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        const m = this._servantMapMap.get(ident);
        if (m !== undefined) {
            return new Map(m);
        }

        return new Map();
    }

    hasServant(ident) {
        const m = this._servantMapMap.get(ident);
        if (m === undefined) {
            return false;
        } else {
            DEV: console.assert(m.size > 0);
            return true;
        }
    }

    addServantLocator(locator, category) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        if (this._locatorMap.has(category)) {
            throw new AlreadyRegisteredException(
                "servant locator",
                StringUtil.escapeString(category, "", this._instance.toStringMode()),
            );
        }

        this._locatorMap.set(category, locator);
    }

    removeServantLocator(category) {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.

        const l = this._locatorMap.get(category);
        if (l === undefined) {
            throw new NotRegisteredException(
                "servant locator",
                StringUtil.escapeString(category, "", this._instance.toStringMode()),
            );
        }
        this._locatorMap.delete(category);
        return l;
    }

    findServantLocator(category) {
        const l = this._locatorMap.get(category);
        return l === undefined ? null : l;
    }

    //
    // Only for use by Ice.ObjectAdapterI.
    //
    destroy() {
        DEV: console.assert(this._instance !== null); // Must not be called after destruction.
        const logger = this._instance.initializationData().logger;
        this._servantMapMap.clear();

        this._defaultServantMap.clear();

        const locatorMap = new Map(this._locatorMap);
        this._locatorMap.clear();
        this._instance = null;

        for (const [key, locator] of locatorMap) {
            try {
                locator.deactivate(key);
            } catch (ex) {
                logger.error(
                    "exception during locator deactivation:\nobject adapter: `" +
                        this._adapterName +
                        "'\nlocator category: `" +
                        key +
                        "'\n" +
                        ex.toString(),
                );
            }
        }
    }
}
