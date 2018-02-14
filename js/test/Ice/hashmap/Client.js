// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;

    var Promise = Ice.Promise;

    var fill = function(arr, val)
    {
        for(var i = 0; i < arr.length; ++i)
        {
            arr[i] = val;
        }
    };

    var testKeys = function(keys, keyComparator, valueComparator)
    {
        var test = function(b)
        {
            if(!b)
            {
                throw new Error("test failed");
            }
        };

        var h = new Ice.HashMap(keyComparator, valueComparator);
        var i;
        for(i = 0; i < keys.length; ++i)
        {
            h.set(keys[i], i);
        }

        //
        // Test the keys.
        //
        for(i = 0; i < keys.length; ++i)
        {
            test(h.has(keys[i]));
            test(h.get(keys[i]) === i);
            test(h.delete(keys[i]) === i);
            test(!h.has(keys[i]));
            h.set(keys[i], i);
            test(h.has(keys[i]));
        }

        //
        // Use an array to keep track of whether we've encountered all entries.
        //
        var a = [];
        a.length = keys.length;

        //
        // Test the keys() method.
        //
        fill(a, false);
        var k = h.keys();
        test(k.length === keys.length);
        for(i = 0; i < k.length; ++i)
        {
            var p = keys.indexOf(k[i]);
            test(p != -1);
            a[p] = true;
        }
        test(a.indexOf(false) === -1);

        //
        // Test the values() method.
        //
        fill(a, false);
        var v = h.values();
        test(v.length === keys.length);
        for(i = 0; i < v.length; ++i)
        {
            a[v[i]] = true;
        }
        test(a.indexOf(false) === -1);

        //
        // Test the forEach() method.
        //
        fill(a, false);
        h.forEach(function(key, val)
            {
                test(keys[val] === key);
                a[val] = true;
            });
        test(a.indexOf(false) === -1);

        //
        // Test the clone() & equals() methods.
        //
        var h2 = h.clone();
        test(h2.equals(h));

        //
        // Test the clear() method.
        //
        h.clear();
        test(h.size === 0);
    };

    var allTests = function(out)
    {
        var p = new Ice.Promise();
        var test = function(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.fail(err);
                    throw err;
                }
            }
        };

        Promise.try(
            function()
            {
                out.write("testing Ice.HashMap... ");

                var h = new Ice.HashMap();

                //
                // Test null key.
                //
                h.set(null, 1);
                test(h.has(null));
                test(h.get(null) === 1);
                h.set(null, 2);
                test(h.get(null) === 2);
                test(h.delete(null) === 2);
                test(!h.has(null));

                //
                // Test NaN key.
                //
                h.set(NaN, 1);
                test(h.has(NaN));
                test(h.has(-NaN));
                test(h.get(NaN) === 1);
                test(h.get(-NaN) === 1);
                h.set(NaN, 2);
                test(h.get(NaN) === 2);
                test(h.delete(NaN) === 2);
                test(!h.has(NaN));

                //
                // Test zero key.
                //
                h.set(0, 1);
                test(h.has(0));
                test(h.has(-0));
                test(h.get(0) === 1);
                test(h.get(-0) === 1);
                h.set(0, 2);
                test(h.get(0) === 2);
                test(h.delete(0) === 2);
                test(!h.has(0));

                //
                // Test integer keys.
                //
                var k = [];
                k.length = 1000;
                var i;
                for(i = 0; i < k.length; ++i)
                {
                    k[i] = i;
                }
                testKeys(k);

                //
                // Test string keys.
                //
                k.length = 100;
                for(i = 0; i < k.length; ++i)
                {
                    k[i] = Ice.generateUUID();
                }
                testKeys(k);

                //
                // Test boolean keys.
                //
                testKeys([true, false]);

                //
                // Test object keys (with hashCode methods).
                //
                testKeys([Ice.OperationMode.Normal, Ice.OperationMode.Nonmutating, Ice.OperationMode.Idempotent]);

                //
                // Test object keys (with hashCode methods and custom key comparator).
                //
                function createObject(i)
                {
                    var obj =
                        {
                            key:i, 
                            hashCode:function()
                            { 
                                return i; 
                            }
                        };
                    return obj;
                }
                k.length = 10;
                for(i = 0; i < k.length; ++i)
                {
                    k[i] = createObject(i);
                }
                var eq = function(a, b) { return a.key === b.key; };
                testKeys(k, eq, eq);

                out.writeLine("ok");
            }
        ).exception(
            function(ex)
            {
                p.fail(ex);
            }
        );
        return p;
    };

    var run = function(out, id)
    {
        return Promise.try(
            function()
            {
                return allTests(out);
            }
        );
    };
    exports.__test__ = run;
    exports.__runServer__ = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
