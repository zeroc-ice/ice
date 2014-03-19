// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    require("Ice/Class");
    require("Ice/StringUtil");
    
    var Slice = global.Slice || {};
    var Ice = global.Ice || {};

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
    
    var HashMap = Ice.Class({
        __init__: function(h)
        {
            this._size = 0;
            this._head = null;
            this._initialCapacity = 32;
            this._loadFactor = 0.75;
            this._table = [];
            this._keyComparator = function(k1, k2) { return k1 === k2; };
            this._valueComparator = function(k1, k2) { return k1 === k2; };

            var i, length;
            if(h === undefined || h === null || h._size === 0)
            {
                this._threshold = this._initialCapacity * this._loadFactor;
                for(i = 0; i < this._initialCapacity; i++)
                {
                    this._table[i] = null;
                }
            }
            else
            {
                this._threshold = h._threshold;
                this._keyComparator = h._keyComparator;
                this._valueComparator = h._valueComparator;
                length = h._table.length;
                this._table.length = length;
                for(i = 0; i < length; i++)
                {
                    this._table[i] = null;
                }
                this.merge(h);
            }
        },
        set: function(key, value)
        {
            var hash = this.computeHash(key);

            var index = this.hashIndex(hash, this._table.length);

            return setInternal(this, key, value, hash, index);
        },
        get: function(key)
        {
            var e = this.findEntry(key, this.computeHash(key));
            return e !== undefined ? e._value : undefined;
        },
        has: function(key)
        {
            return this.findEntry(key, this.computeHash(key)) !== undefined;
        },
        delete: function(key)
        {
            var hash = this.computeHash(key);

            var index = this.hashIndex(hash, this._table.length);

            //
            // Search for an entry with the same key.
            //
            var prev = null;
            for(var e = this._table[index]; e !== null; e = e._nextInBucket)
            {
                if(e._hash === hash && this.keysEqual(key, e._key))
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
        equals: function(other)
        {
            if(other === null || !(other instanceof HashMap) || this._size !== other._size)
            {
                return false;
            }

            for(var e = this._head; e !== null; e = e._next)
            {
                var oe = other.findEntry(e._key, e._hash);
                if(oe === undefined || !this.valuesEqual(e._value, oe._value))
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
            /*
            var e =
            {
                key: key,
                value: value,
                prev: null,
                next: null,
                _hash: hash
            }
            */
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
            if(typeof(v.hashCode) === "function")
            {
                return v.hashCode();
            }

            var hash = 0;
            var type = typeof(v);
            if(type === "string" || v instanceof String)
            {
                hash = StringUtil.hashCode(v);
            }
            else if(type === "number" || v instanceof Number)
            {
                hash = v.toFixed(0);
            }
            else if(type === "boolean" || v instanceof Boolean)
            {
                hash = v ? 1 : 0;
            }
            else if(v !== null)
            {
                throw "cannot compute hash for value of type " + type;
            }
            return hash;
        },
        keysEqual: function(k1, k2)
        {
            return this._keyComparator.call(this._keyComparator, k1, k2);
        },
        valuesEqual: function(v1, v2)
        {
            return this._valueComparator.call(this._valueComparator, v1, v2);
        }
    });
    
    var prototype = HashMap.prototype;
    
    Object.defineProperty(prototype, "size", {
        get: function() { return this._size; }
    });

    Object.defineProperty(prototype, "entries", {
        get: function() { return this._head; }
    });

    Object.defineProperty(prototype, "keyComparator", {
        get: function() { return this._keyComparator; },
        set: function(fn) { this._keyComparator = fn; }
    });

    Object.defineProperty(prototype, "valueComparator", {
        get: function() { return this._valueComparator; },
        set: function(fn) { this._valueComparator = fn; }
    });

    Object.defineProperty(HashMap, "compareEquals", {
        get: function() { return function(o1, o2) { return o1.equals(o2); }; }
    });
    
    Slice.defineDictionary = function(module, name, helperName, keyHelper, valueHelper, fixed, useEquals, valueType)
    {
        if(useEquals)
        {
            //
            // Define a constructor function for a dictionary whose key type requires
            // comparison using an equals() method instead of the native comparison
            // operators.
            //
            module[name] = function(h)
            {
                var r = new HashMap(h);
                r.keyComparator = HashMap.compareEquals;
                return r;
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
                        helper = Ice.StreamHelpers.generateDictHelper(eval(keyHelper), eval(valueHelper), fixed, 
                                                                      eval(valueType), module[name]);
                        /*jshint +W061 */
                    }
                    return helper;
                }
        });
    };

    Ice.HashMap = HashMap;
    global.Slice = Slice;
    global.Ice = Ice;
}(typeof (global) === "undefined" ? window : global));
