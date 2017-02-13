// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

(function(module, require, exports)
{
    var Ice = require("ice").Ice;
    var Test = require("Test").Test;
    var Promise = Ice.Promise;

    var run = function(out, id)
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
                out.write("testing default values... ");
                var v = new Test.Struct1();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b === 254);
                test(v.s === 16000);
                test(v.i === 3);
                test(v.l.equals(new Ice.Long(0, 4)));
                test(v.f === 5.1);
                test(v.d === 6.2);
                test(v.str === "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b?");
                test(v.c1 === Test.Color.red);
                test(v.c2 === Test.Color.green);
                test(v.c3 === Test.Color.blue);
                test(v.nc1 === Test.Nested.Color.red);
                test(v.nc2 === Test.Nested.Color.green);
                test(v.nc3 === Test.Nested.Color.blue);
                test(v.noDefault === "");
                test(v.zeroI === 0);
                test(v.zeroL.equals(new Ice.Long(0, 0)));
                test(v.zeroF === 0);
                test(v.zeroDotF === 0);
                test(v.zeroD === 0);
                test(v.zeroDotD === 0);

                v = new Test.Struct2();
                test(v.boolTrue === Test.ConstBool);
                test(v.b === Test.ConstByte);
                test(v.s === Test.ConstShort);
                test(v.i === Test.ConstInt);
                test(v.l.equals(Test.ConstLong));
                test(v.f === Test.ConstFloat);
                test(v.d === Test.ConstDouble);
                test(v.str ==  Test.ConstString);
                test(v.c1 === Test.ConstColor1);
                test(v.c2 === Test.ConstColor2);
                test(v.c3 === Test.ConstColor3);
                test(v.nc1 === Test.ConstNestedColor1);
                test(v.nc2 === Test.ConstNestedColor2);
                test(v.nc3 === Test.ConstNestedColor3);

                v = new Test.Base();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b === 1);
                test(v.s === 2);
                test(v.i === 3);
                test(v.l.equals(new Ice.Long(0, 4)));
                test(v.f === 5.1);
                test(v.d === 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b?");
                test(v.noDefault === "");
                test(v.zeroI === 0);
                test(v.zeroL.equals(new Ice.Long(0, 0)));
                test(v.zeroF === 0);
                test(v.zeroDotF === 0);
                test(v.zeroD === 0);
                test(v.zeroDotD === 0);

                v = new Test.Derived();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b === 1);
                test(v.s === 2);
                test(v.i === 3);
                test(v.l.equals(new Ice.Long(0, 4)));
                test(v.f === 5.1);
                test(v.d === 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b?");
                test(v.c1 === Test.Color.red);
                test(v.c2 === Test.Color.green);
                test(v.c3 === Test.Color.blue);
                test(v.nc1 === Test.Nested.Color.red);
                test(v.nc2 === Test.Nested.Color.green);
                test(v.nc3 === Test.Nested.Color.blue);
                test(v.noDefault === "");
                test(v.zeroI === 0);
                test(v.zeroL.equals(new Ice.Long(0, 0)));
                test(v.zeroF === 0);
                test(v.zeroDotF === 0);
                test(v.zeroD === 0);
                test(v.zeroDotD === 0);

                v = new Test.BaseEx();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b === 1);
                test(v.s === 2);
                test(v.i === 3);
                test(v.l.equals(new Ice.Long(0, 4)));
                test(v.f === 5.1);
                test(v.d === 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b?");
                test(v.noDefault === "");
                test(v.zeroI === 0);
                test(v.zeroL.equals(new Ice.Long(0, 0)));
                test(v.zeroF === 0);
                test(v.zeroDotF === 0);
                test(v.zeroD === 0);
                test(v.zeroDotD === 0);

                v = new Test.DerivedEx();
                test(!v.boolFalse);
                test(v.boolTrue);
                test(v.b === 1);
                test(v.s === 2);
                test(v.i === 3);
                test(v.l.equals(new Ice.Long(0, 4)));
                test(v.f === 5.1);
                test(v.d === 6.2);
                test(v.str == "foo \\ \"bar\n \r\n\t\u000b\f\u0007\b?");
                test(v.noDefault === "");
                test(v.c1 === Test.Color.red);
                test(v.c2 === Test.Color.green);
                test(v.c3 === Test.Color.blue);
                test(v.nc1 === Test.Nested.Color.red);
                test(v.nc2 === Test.Nested.Color.green);
                test(v.nc3 === Test.Nested.Color.blue);
                test(v.zeroI === 0);
                test(v.zeroL.equals(new Ice.Long(0, 0)));
                test(v.zeroF === 0);
                test(v.zeroDotF === 0);
                test(v.zeroD === 0);
                test(v.zeroDotD === 0);

                out.writeLine("ok");


                out.write("testing default constructor... ");

                var v2 = new Test.StructNoDefaults();
                test(v2.bo === false);
                test(v2.b === 0);
                test(v2.s === 0);
                test(v2.i === 0);
                test(v2.l === 0);
                test(v2.f === 0.0);
                test(v2.d === 0.0);
                test(v2.str === "");
                test(v2.c1 == Test.Color.red);
                test(v2.bs === null);
                test(v2.is === null);
                test(v2.st !== null && v2.st instanceof Test.InnerStruct);
                test(v2.dict === null);

                var e = new Test.ExceptionNoDefaults();
                test(e.str === "");
                test(e.c1 == Test.Color.red);
                test(e.bs === null);
                test(e.st !== null && e.st instanceof Test.InnerStruct);
                test(e.dict === null);

                var cl = new Test.ClassNoDefaults();
                test(cl.str === "");
                test(cl.c1 == Test.Color.red);
                test(cl.bs === null);
                test(cl.st !== null && cl.st instanceof Test.InnerStruct);
                test(cl.dict === null);

                out.writeLine("ok");
            }
        ).then(
            function()
            {
                p.succeed();
            },
            function(ex)
            {
                p.fail(ex);
            }
        );
        return p;
    };
    exports.__test__ = run;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice.__require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
