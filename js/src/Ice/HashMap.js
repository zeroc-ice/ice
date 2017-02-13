// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
var __M = Ice.__M;
__M.require(module, ["../Ice/Class", "../Ice/StringUtil", "../Ice/UUID"]);
var StringUtil = Ice.StringUtil;

function setInternal(map, key, value, hash, index)
{
    //
    // Search for an entry with the same key.
    //
    for(var e = map._table[index]; e !== null; e = e._nextInBucket)
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

var HashMap = Ice.Class({
    __init__: function(arg1, arg2)
    {
        //
        // The first argument can be a HashMap or the keyComparator, the second
        // argument if present is always the value comparator.
        // 
        var args = arguments;

        var h, keyComparator, valueComparator;

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

        var i, length;
        if(h instanceof HashMap && h._size > 0)
        {
            this._threshold = h._threshold;
            length = h._table.length;
            this._table.length = length;
            for(i = 0; i < length; i++)
            {
                this._table[i] = null;
            }
            this.merge(h);
        }
        else
        {
            this._threshold = this._initialCapacity * this._loadFactor;
            for(i = 0; i < this._initialCapacity; i++)
            {
                this._table[i] = null;
            }
        }
    },
    set: function(key, value)
    {
        var r = this.computeHash(key); // Returns an object with key,hash members.

        var index = this.hashIndex(r.hash, this._table.length);

        return setInternal(this, r.key, value, r.hash, index);
    },
    get: function(key)
    {
        var r = this.computeHash(key); // Returns an object with key,hash members.
        var e = this.findEntry(r.key, r.hash);
        return e !== undefined ? e._value : undefined;
    },
    has: function(key)
    {
        var r = this.computeHash(key); // Returns an object with key,hash members.
        return this.findEntry(r.key, r.hash) !== undefined;
    },
    delete: function(key)
    {
        var r = this.computeHash(key); // Returns an object with key,hash members.

        var index = this.hashIndex(r.hash, this._table.length);

        //
        // Search for an entry with the same key.
        //
        var prev = null;
        for(var e = this._table[index]; e !== null; e = e._nextInBucket)
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
    },
    clear: function()
    {
        for(var i = 0; i < this._table.length; ++i)
        {
            this._table[i] = null;
        }
        this._head = null;
        this._size = 0;
    },
    forEach: function(fn, obj)
    {
        obj = obj === undefined ? fn : obj;
        for(var e = this._head; e !== null; e = e._next)
        {
            fn.call(obj, e._key, e._value);
        }
    },
    keys: function()
    {
        var k = [];
        var i = 0;
        for(var e = this._head; e !== null; e = e._next)
        {
            k[i++] = e._key;
        }
        return k;
    },
    values: function()
    {
        var v = [];
        var i = 0;
        for(var e = this._head; e !== null; e = e._next)
        {
            v[i++] = e._value;
        }
        return v;
    },
    equals: function(other, valuesEqual)
    {
        if(other === null || !(other instanceof HashMap) || this._size !== other._size)
        {
            return false;
        }

        var self = this;
        var eq = valuesEqual || function(v1, v2)
            {
                return self._valueComparator.call(self._valueComparator, v1, v2);
            };
        
        for(var e = this._head; e !== null; e = e._next)
        {
            var oe = other.findEntry(e._key, e._hash);
            if(oe === undefined || !eq(e._value, oe._value))
            {
                return false;
            }
        }

        return true;
    },
    clone: function()
    {
        return new HashMap(this);
    },
    merge: function(from)
    {
        for(var e = from._head; e !== null; e = e._next)
        {
            setInternal(this, e._key, e._value, e._hash, this.hashIndex(e._hash, this._table.length));
        }
    },
    add: function(key, value, hash, index)
    {
        //
        // Create a new table entry.
        //
        var e = Object.create(null, {
            "key": {
                enumerable: true,
                get: function() { return this._key; }
            },
            "value": {
                enumerable: true,
                get: function() { return this._value; }
            },
            "next": {
                enumerable: true,
                get: function() { return this._next; }
            },
            "_key": {
                enumerable: false,
                writable: true,
                value: key
            },
            "_value": {
                enumerable: false,
                writable: true,
                value: value
            },
            "_prev": {
                enumerable: false,
                writable: true,
                value: null
            },
            "_next": {
                enumerable: false,
                writable: true,
                value: null
            },
            "_nextInBucket": {
                enumerable: false,
                writable: true,
                value: null
            },
            "_hash": {
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
    },
    resize: function(capacity)
    {
        var oldTable = this._table;

        var newTable = [];
        for(var i = 0; i < capacity; i++)
        {
            newTable[i] = null;
        }

        //
        // Re-assign all entries to buckets.
        //
        for(var e = this._head; e !== null; e = e._next)
        {
            var index = this.hashIndex(e._hash, capacity);
            e._nextInBucket = newTable[index];
            newTable[index] = e;
        }

        this._table = newTable;
        this._threshold = (capacity * this._loadFactor);
    },
    findEntry: function(key, hash)
    {
        var index = this.hashIndex(hash, this._table.length);
        //
        // Search for an entry with the same key.
        //
        for(var e = this._table[index]; e !== null; e = e._nextInBucket)
        {
            if(e._hash === hash && this.keysEqual(key, e._key))
            {
                return e;
            }
        }

        return undefined;
    },
    hashIndex: function(hash, len)
    {
        return hash & (len - 1);
    },
    computeHash: function(v)
    {
        var uuid;
        if(v === 0 || v === -0)
        {
            return {key:0, hash:0};
        }

        if(v === null)
        {
            if(HashMap._null === null)
            {
                uuid = Ice.generateUUID();
                HashMap._null = {key:uuid, hash:StringUtil.hashCode(uuid)};
            }
            return HashMap._null;
        }

        if(v === undefined)
        {
            throw new Error("cannot compute hash for undefined value");
        }

        if(typeof(v.hashCode) === "function")
        {
            return {key:v, hash:v.hashCode()};
        }

        var type = typeof(v);
        if(type === "string" || v instanceof String)
        {
            return {key:v, hash:StringUtil.hashCode(v)};
        }
        else if(type === "number" || v instanceof Number)
        {
            if(isNaN(v))
            {
                if(HashMap._nan === null)
                {
                    uuid = Ice.generateUUID();
                    HashMap._nan = {key:uuid, hash:StringUtil.hashCode(uuid)};
                }
                return HashMap._nan;
            }
            return {key:v, hash:v.toFixed(0)};
        }
        else if(type === "boolean" || v instanceof Boolean)
        {
            return {key:v, hash:v ? 1 : 0};
        }

        throw new Error("cannot compute hash for value of type " + type);
    },
    keysEqual: function(k1, k2)
    {
        return this._keyComparator.call(this._keyComparator, k1, k2);
    }
});
Ice.HashMap = HashMap;

HashMap.compareEquals = compareEquals;
HashMap.compareIdentity = compareIdentity;
HashMap._null = null;
HashMap._nan = null;

var prototype = HashMap.prototype;

Object.defineProperty(prototype, "size", {
    get: function() { return this._size; }
});

Object.defineProperty(prototype, "entries", {
    get: function() { return this._head; }
});

var Slice = Ice.Slice;
Slice.defineDictionary = function(module, name, helperName, keyHelper, valueHelper, fixed, keysEqual, valueType, valuesEqual)
{
    if(keysEqual !== undefined || valuesEqual !== undefined)
    {
        //
        // Define a constructor function for a dictionary whose key type requires
        // comparison using an equals() method instead of the native comparison
        // operators.
        //
        module[name] = function(h)
        {
            return new HashMap(h || keysEqual, valuesEqual);
        };
    }
    else
    {
        module[name] = HashMap;
    }

    var helper = null;
    Object.defineProperty(module, helperName,
    {
        get: function()
            {
                if(helper === null)
                {
                    /*jshint -W061 */
                    helper = Ice.StreamHelpers.generateDictHelper(__M.type(keyHelper), __M.type(valueHelper), fixed, 
                                                                  __M.type(valueType), module[name]);
                    /*jshint +W061 */
                }
                return helper;
            }
    });
};
module.exports.Ice = Ice;
