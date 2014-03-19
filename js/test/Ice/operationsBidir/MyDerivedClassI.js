// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(global){
    var Ice = global.Ice;
    var Test = global.Test;

    var Class = Ice.Class;

    var test = function(b)
    {
        if(!b)
        {
            throw new Error("test failed");
        }
    };

    var MyDerivedClassI = Class(Test.MyDerivedClass, {
        //
        // Override the Object "pseudo" operations to verify the operation mode.
        //

        ice_isA: function(id, current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_isA.call(this, id, current);
        },

        ice_ping: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            Ice.Object.prototype.ice_ping.call(this, current);
        },

        ice_ids: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_ids.call(this, current);
        },

        ice_id: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
            return Ice.Object.prototype.ice_id.call(this, current);
        },

        shutdown: function(current)
        {
            current.adapter.getCommunicator().shutdown();
        },

        delay: function(cb, ms, current)
        {
            setTimeout(
                function()
                {
                    cb.ice_response();
                }, ms);
        },

        opVoid: function(current)
        {
            test(current.mode === Ice.OperationMode.Normal);
        },

        opBool: function(p1, p2, current)
        {
            return [p2, p1];
        },

        opBoolS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        },

        opBoolSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            return [p1.reverse(), p3];
        },

        opByte: function(p1, p2, current)
        {
            return [p1, (p1 ^ p2) & 0xff];
        },

        opByteBoolD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opByteS: function(p1, p2, current)
        {
            var p3 = Ice.Buffer.createNative(p1.length);
            for(var i = 0; i < p1.length; i++)
            {
                p3[i] = p1[p1.length - (i + 1)];
            }

            var r = Ice.Buffer.createNative(p1.length + p2.length);
            var i;
            for(i = 0; i < p1.length; ++i)
            {
                r[i] = p1[i];
            }
            for(i = 0; i < p2.length; ++i)
            {
                r[i + p1.length] = p2[i];
            }
            return [r, p3];
        },

        opByteSS: function(p1, p2, current)
        {
            var r = p1.concat(p2);
            return [r, p1.reverse()];
        },

        opFloatDouble: function(p1, p2, current)
        {
            return [p2, p1, p2];
        },

        opFloatDoubleS: function(p1, p2, current)
        {
            var r = p2.concat(p1);
            var p4 = p2.reverse();
            return [r, p1, p4];
        },

        opFloatDoubleSS: function(p1, p2, current)
        {
            var r = p2.concat(p2);
            var p4 = p2.reverse();
            return [r, p1, p4];
        },

        opLongFloatD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opMyClass: function(p1, current)
        {
            var p2 = p1;
            var p3 = Test.MyClassPrx.uncheckedCast(
                current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
            var r = Test.MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
            return [r, p2, p3];
        },

        opMyEnum: function(p1, current)
        {
            return [Test.MyEnum.enum3, p1];
        },

        opShortIntD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opShortIntLong: function(p1, p2, p3, current)
        {
            return [p3, p1, p2, p3];
        },

        opShortIntLongS: function(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        },

        opShortIntLongSS: function(p1, p2, p3, current)
        {
            return [p3, p1, p2.reverse(), p3.concat(p3)];
        },

        opString: function(p1, p2, current)
        {
            return [p1 + " " + p2, p2 + " " + p1];
        },

        opStringMyEnumD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opMyEnumStringD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opMyStructMyEnumD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opIntS: function(s, current)
        {
            return s.map(function(v, i, arr) { return -v; });
        },

        opByteSOneway: function(s, current)
        {
        },

        opContext: function(current)
        {
            return current.ctx;
        },

        opDoubleMarshaling: function(p1, p2, current)
        {
            var d = 1278312346.0 / 13.0;
            test(p1 === d);
            for(var i = 0; i < p2.length; ++i)
            {
                test(p2[i] === d);
            }
        },

        opStringS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p1.reverse();
            return [r, p3];
        },

        opStringSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p2.reverse();
            return [r, p3];
        },

        opStringSSS: function(p1, p2, current)
        {
            var p3 = p1.concat(p2);
            var r = p2.reverse();
            return [r, p3];
        },

        opStringStringD: function(p1, p2, current)
        {
            var r = p1.clone();
            r.merge(p2);
            return [r, p1];
        },

        opStruct: function(p1, p2, current)
        {
            p1.s.s = "a new string";
            return [p2, p1];
        },

        opIdempotent: function(current)
        {
            test(current.mode === Ice.OperationMode.Idempotent);
        },

        opNonmutating: function(current)
        {
            test(current.mode === Ice.OperationMode.Nonmutating);
        },

        opDerived: function(current)
        {
        }
    });

    global.MyDerivedClassI = MyDerivedClassI;
}(typeof (global) === "undefined" ? window : global));
