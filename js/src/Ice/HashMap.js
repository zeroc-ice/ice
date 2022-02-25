//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/StringUtil");
require("../Ice/UUID");

const _ModuleRegistry = Ice._ModuleRegistry;
const StringUtil = Ice.StringUtil;

function setInternal(map, key, value, hash, index)
{
    //
    // Search for an entry with the same key.
    //
    for(let e = map._table[index]; e !== null; e = e._nextInBucket)
    {
        if(e._hash === hash && map.keysEqual(key, e._key))
        {
            //
            // Found a match, update the value.
            //
            e._value = value;
            return undefined;
        }
    }

    //
    // No match found, add a new entry.
    //
    map.add(key, value, hash, index);
    return undefined;
}

function compareEquals(v1, v2)
{
    if(v1 === v2)
    {
        return true;
    }
    if(v1 === undefined || v1 === null || v2 === undefined || v2 === null)
    {
        return false;
    }
    return v1.equals(v2);
}

function compareIdentity(v1, v2)
{
    return v1 === v2;
}

class HashMap
{
    constructor(arg1, arg2)
    {
        //
        // The first argument can be a HashMap or the keyComparator, the second
        // argument if present is always the value comparator.
        //
        let h, keyComparator, valueComparator;

        if(typeof arg1 == "function")
        {
            keyComparator = arg1;
            valueComparator = arg2;
        }
        else if(arg1 instanceof HashMap)
        {
            h = arg1;
            keyComparator = h.keyComparator;
            valueComparator = h.valueComparator;
        }

        this._size = 0;
        this._head = null;
        this._initialCapacity = 32;
        this._loadFactor = 0.75;
        this._table = [];

        this._keyComparator = (typeof keyComparator == "function") ? keyComparator : compareIdentity;
        this._valueComparator = (typeof valueComparator == "function") ? valueComparator : compareIdentity;

        if(h instanceof HashMap && h._size > 0)
        {
            this._threshold = h._threshold;
            this._table.length = h._table.length;
            for(let i = 0; i < h._table.length; i++)
            {
                this._table[i] = null;
            }
            this.merge(h);
        }
        else
        {
            this._threshold = this._initialCapacity * this._loadFactor;
            for(let i = 0; i < this._initialCapacity; i++)
            {
                this._table[i] = null;
            }
        }
    }

    set(key, value)
    {
        const r = this.computeHash(key); // Returns an object with key,hash members.

        const index = this.hashIndex(r.hash, this._table.length);

        return setInternal(this, r.key, value, r.hash, index);
    }

    get(key)
    {
        const r = this.computeHash(key); // Returns an object with key,hash members.
        const e = this.findEntry(r.key, r.hash);
        return e !== undefined ? e._value : undefined;
    }

    has(key)
    {
        const r = this.computeHash(key); // Returns an object with key,hash members.
        return this.findEntry(r.key, r.hash) !== undefined;
    }

    delete(key)
    {
        const r = this.computeHash(key); // Returns an object with key,hash members.

        const index = this.hashIndex(r.hash, this._table.length);

        //
        // Search for an entry with the same key.
        //
        let prev = null;
        for(let e = this._table[index]; e !== null; e = e._nextInBucket)
        {
            if(e._hash === r.hash && this.keysEqual(r.key, e._key))
            {
                //
                // Found a match.
                //
                this._size--;

                //
                // Remove from bucket.
                //
                if(prev !== null)
                {
                    prev._nextInBucket = e._nextInBucket;
                }
                else
                {
                    this._table[index] = e._nextInBucket;
                }

                //
                // Unlink the entry.
                //
                if(e._prev !== null)
                {
                    e._prev._next = e._next;
                }
                if(e._next !== null)
                {
                    e._next._prev = e._prev;
                }

                if(this._head === e)
                {
                    this._head = e._next;
                }

                return e._value;
            }

            prev = e;
        }

        return undefined;
    }

    clear()
    {
        for(let i = 0; i < this._table.length; ++i)
        {
            this._table[i] = null;
        }
        this._head = null;
        this._size = 0;
    }

    forEach(fn, obj)
    {
        obj = obj === undefined ? fn : obj;
        for(let e = this._head; e !== null; e = e._next)
        {
            fn.call(obj, e._value, e._key);
        }
    }

    *entries()
    {
        for(let e = this._head; e !== null; e = e._next)
        {
            yield [e._key, e._value];
        }
    }

    *keys()
    {
        for(let e = this._head; e !== null; e = e._next)
        {
            yield e._key;
        }
    }

    *values()
    {
        for(let e = this._head; e !== null; e = e._next)
        {
            yield e._value;
        }
    }

    equals(other, valuesEqual)
    {
        if(other === null || !(other instanceof HashMap) || this._size !== other._size)
        {
            return false;
        }

        let eq;
        if(valuesEqual)
        {
            eq = valuesEqual;
        }
        else
        {
            eq = (v1, v2) => this._valueComparator.call(this._valueComparator, v1, v2);
        }

        for(let e = this._head; e !== null; e = e._next)
        {
            const oe = other.findEntry(e._key, e._hash);
            if(oe === undefined || !eq(e._value, oe._value))
            {
                return false;
            }
        }
        return true;
    }

    merge(from)
    {
        for(let e = from._head; e !== null; e = e._next)
        {
            setInternal(this, e._key, e._value, e._hash, this.hashIndex(e._hash, this._table.length));
        }
    }

    add(key, value, hash, index)
    {
        //
        // Create a new table entry.
        //
        const e = Object.create(null, {
            key:
            {
                enumerable: true,
                get: function() { return this._key; }
            },
            value:
            {
                enumerable: true,
                get: function() { return this._value; }
            },
            next:
            {
                enumerable: true,
                get: function() { return this._next; }
            },
            _key:
            {
                enumerable: false,
                writable: true,
                value: key
            },
            _value:
            {
                enumerable: false,
                writable: true,
                value: value
            },
            _prev:
            {
                enumerable: false,
                writable: true,
                value: null
            },
            _next:
            {
                enumerable: false,
                writable: true,
                value: null
            },
            _nextInBucket:
            {
                enumerable: false,
                writable: true,
                value: null
            },
            _hash:
            {
                enumerable: false,
                writable: true,
                value: hash
            }
        });
        e._nextInBucket = this._table[index];
        this._table[index] = e;

        e._next = this._head;
        if(this._head !== null)
        {
            this._head._prev = e;
        }
        this._head = e;

        this._size++;
        if(this._size >= this._threshold)
        {
            this.resize(this._table.length * 2);
        }
    }

    resize(capacity)
    {
        const newTable = new Array(capacity).fill(null);

        //
        // Re-assign all entries to buckets.
        //
        for(let e = this._head; e !== null; e = e._next)
        {
            const index = this.hashIndex(e._hash, capacity);
            e._nextInBucket = newTable[index];
            newTable[index] = e;
        }

        this._table = newTable;
        this._threshold = (capacity * this._loadFactor);
    }

    findEntry(key, hash)
    {
        const index = this.hashIndex(hash, this._table.length);
        //
        // Search for an entry with the same key.
        //
        for(let e = this._table[index]; e !== null; e = e._nextInBucket)
        {
            if(e._hash === hash && this.keysEqual(key, e._key))
            {
                return e;
            }
        }

        return undefined;
    }

    hashIndex(hash, len)
    {
        return hash & (len - 1);
    }

    computeHash(v)
    {
        if(v === 0)
        {
            return {key: 0, hash: 0};
        }

        if(v === null)
        {
            if(HashMap._null === null)
            {
                const uuid = Ice.generateUUID();
                HashMap._null = {key: uuid, hash: StringUtil.hashCode(uuid)};
            }
            return HashMap._null;
        }

        if(v === undefined)
        {
            throw new RangeError("cannot compute hash for undefined value");
        }

        if(typeof v.hashCode === "function")
        {
            return {key: v, hash: v.hashCode()};
        }

        const type = typeof v;
        if(type === "string" || v instanceof String)
        {
            return {key: v, hash: StringUtil.hashCode(v)};
        }
        else if(type === "number" || v instanceof Number)
        {
            if(isNaN(v))
            {
                if(HashMap._nan === null)
                {
                    const uuid = Ice.generateUUID();
                    HashMap._nan = {key: uuid, hash: StringUtil.hashCode(uuid)};
                }
                return HashMap._nan;
            }
            return {key: v, hash: v.toFixed(0)};
        }
        else if(type === "boolean" || v instanceof Boolean)
        {
            return {key: v, hash: v ? 1 : 0};
        }

        throw new RangeError("cannot compute hash for value of type " + type);
    }

    keysEqual(k1, k2)
    {
        return this._keyComparator.call(this._keyComparator, k1, k2);
    }

    get size()
    {
        return this._size;
    }
}

HashMap.prototype[Symbol.iterator] = HashMap.prototype.entries;

Ice.HashMap = HashMap;

HashMap.compareEquals = compareEquals;
HashMap.compareIdentity = compareIdentity;
HashMap._null = null;
HashMap._nan = null;

const Slice = Ice.Slice;

Slice.defineDictionary = function(module, name, helperName, keyHelper, valueHelper, fixed, keysEqual, valueType)
{
    if(keysEqual === undefined)
    {
        module[name] = Map;
    }
    else
    {
        //
        // Define a constructor function for a dictionary whose key type requires
        // comparison using an equals() method instead of the native comparison
        // operators.
        //
        module[name] = function(h)
        {
            return new HashMap(h || keysEqual);
        };
    }

    let helper = null;
    Object.defineProperty(module, helperName,
    {
        get: function()
        {
            if(helper === null)
            {
                helper = Ice.StreamHelpers.generateDictHelper(_ModuleRegistry.type(keyHelper),
                                                              _ModuleRegistry.type(valueHelper),
                                                              fixed,
                                                              _ModuleRegistry.type(valueType),
                                                              module[name]);
            }
            return helper;
        }
    });
};
module.exports.Ice = Ice;
