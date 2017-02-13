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
    const Ice = require("ice").Ice;
    const Test = require("Test").Test;

    const Promise = Ice.Promise;

    function allTests(out, communicator)
    {
        const p = new Ice.Promise();
        function test(b)
        {
            if(!b)
            {
                try
                {
                    throw new Error("test failed");
                }
                catch(err)
                {
                    p.reject(err);
                    throw err;
                }
            }
        };

        let base, proxy;
        Promise.try(() =>
            {
                out.write("testing stringToProxy... ");
                const ref = "test:default -p 12010";
                base = communicator.stringToProxy(ref);
                test(base !== null);
                out.writeLine("ok");
                out.write("testing checked cast... ");
                return Test.TestIntfPrx.checkedCast(base);
            }
        ).then(obj =>
            {
                proxy = obj;
                test(proxy !== null);
                test(proxy.equals(base));
                out.writeLine("ok");
                out.write("testing enum values... ");
                test(Test.ByteEnum.benum1.value === 0);
                test(Test.ByteEnum.benum2.value === 1);
                test(Test.ByteEnum.benum3.value === Test.ByteConst1);
                test(Test.ByteEnum.benum4.value === Test.ByteConst1 + 1);
                test(Test.ByteEnum.benum5.value === Test.ShortConst1);
                test(Test.ByteEnum.benum6.value === Test.ShortConst1 + 1);
                test(Test.ByteEnum.benum7.value === Test.IntConst1);
                test(Test.ByteEnum.benum8.value === Test.IntConst1 + 1);
                test(Test.ByteEnum.benum9.value === Test.LongConst1.low);
                test(Test.ByteEnum.benum10.value === Test.LongConst1.low + 1);
                test(Test.ByteEnum.benum11.value === Test.ByteConst2);

                test(Test.ByteEnum.valueOf(0) === Test.ByteEnum.benum1);
                test(Test.ByteEnum.valueOf(1) === Test.ByteEnum.benum2);
                test(Test.ByteEnum.valueOf(Test.ByteConst1) === Test.ByteEnum.benum3);
                test(Test.ByteEnum.valueOf(Test.ByteConst1+ 1) === Test.ByteEnum.benum4);
                test(Test.ByteEnum.valueOf(Test.ShortConst1) === Test.ByteEnum.benum5);
                test(Test.ByteEnum.valueOf(Test.ShortConst1 + 1) === Test.ByteEnum.benum6);
                test(Test.ByteEnum.valueOf(Test.IntConst1) === Test.ByteEnum.benum7);
                test(Test.ByteEnum.valueOf(Test.IntConst1 + 1) === Test.ByteEnum.benum8);
                test(Test.ByteEnum.valueOf(Test.LongConst1.low) === Test.ByteEnum.benum9);
                test(Test.ByteEnum.valueOf(Test.LongConst1.low + 1) === Test.ByteEnum.benum10);
                test(Test.ByteEnum.valueOf(Test.ByteConst2) === Test.ByteEnum.benum11);

                test(Test.ShortEnum.senum1.value === 3);
                test(Test.ShortEnum.senum2.value === 4);
                test(Test.ShortEnum.senum3.value === Test.ByteConst1);
                test(Test.ShortEnum.senum4.value === Test.ByteConst1 + 1);
                test(Test.ShortEnum.senum5.value === Test.ShortConst1);
                test(Test.ShortEnum.senum6.value === Test.ShortConst1 + 1);
                test(Test.ShortEnum.senum7.value === Test.IntConst1);
                test(Test.ShortEnum.senum8.value === Test.IntConst1 + 1);
                test(Test.ShortEnum.senum9.value === Test.LongConst1.low);
                test(Test.ShortEnum.senum10.value === Test.LongConst1.low + 1);
                test(Test.ShortEnum.senum11.value === Test.ShortConst2);

                test(Test.ShortEnum.valueOf(3) === Test.ShortEnum.senum1);
                test(Test.ShortEnum.valueOf(4) === Test.ShortEnum.senum2);
                test(Test.ShortEnum.valueOf(Test.ByteConst1) === Test.ShortEnum.senum3);
                test(Test.ShortEnum.valueOf(Test.ByteConst1 + 1) === Test.ShortEnum.senum4);
                test(Test.ShortEnum.valueOf(Test.ShortConst1) === Test.ShortEnum.senum5);
                test(Test.ShortEnum.valueOf(Test.ShortConst1 + 1) === Test.ShortEnum.senum6);
                test(Test.ShortEnum.valueOf(Test.IntConst1) === Test.ShortEnum.senum7);
                test(Test.ShortEnum.valueOf(Test.IntConst1 + 1) === Test.ShortEnum.senum8);
                test(Test.ShortEnum.valueOf(Test.LongConst1.low) === Test.ShortEnum.senum9);
                test(Test.ShortEnum.valueOf(Test.LongConst1.low + 1) === Test.ShortEnum.senum10);
                test(Test.ShortEnum.valueOf(Test.ShortConst2) === Test.ShortEnum.senum11);

                test(Test.IntEnum.ienum1.value === 0);
                test(Test.IntEnum.ienum2.value === 1);
                test(Test.IntEnum.ienum3.value === Test.ByteConst1);
                test(Test.IntEnum.ienum4.value === Test.ByteConst1 + 1);
                test(Test.IntEnum.ienum5.value === Test.ShortConst1);
                test(Test.IntEnum.ienum6.value === Test.ShortConst1 + 1);
                test(Test.IntEnum.ienum7.value === Test.IntConst1);
                test(Test.IntEnum.ienum8.value === Test.IntConst1 + 1);
                test(Test.IntEnum.ienum9.value === Test.LongConst1.low);
                test(Test.IntEnum.ienum10.value === Test.LongConst1.low + 1);
                test(Test.IntEnum.ienum11.value === Test.IntConst2);
                test(Test.IntEnum.ienum12.value === Test.LongConst2.low);

                test(Test.IntEnum.valueOf(0) === Test.IntEnum.ienum1);
                test(Test.IntEnum.valueOf(1) === Test.IntEnum.ienum2);
                test(Test.IntEnum.valueOf(Test.ByteConst1) === Test.IntEnum.ienum3);
                test(Test.IntEnum.valueOf(Test.ByteConst1 + 1) === Test.IntEnum.ienum4);
                test(Test.IntEnum.valueOf(Test.ShortConst1) === Test.IntEnum.ienum5);
                test(Test.IntEnum.valueOf(Test.ShortConst1 + 1) === Test.IntEnum.ienum6);
                test(Test.IntEnum.valueOf(Test.IntConst1) === Test.IntEnum.ienum7);
                test(Test.IntEnum.valueOf(Test.IntConst1 + 1) === Test.IntEnum.ienum8);
                test(Test.IntEnum.valueOf(Test.LongConst1.low) === Test.IntEnum.ienum9);
                test(Test.IntEnum.valueOf(Test.LongConst1.low + 1) === Test.IntEnum.ienum10);
                test(Test.IntEnum.valueOf(Test.IntConst2) === Test.IntEnum.ienum11);
                test(Test.IntEnum.valueOf(Test.LongConst2.low) === Test.IntEnum.ienum12);

                test(Test.SimpleEnum.red.value === 0);
                test(Test.SimpleEnum.green.value === 1);
                test(Test.SimpleEnum.blue.value === 2);

                test(Test.SimpleEnum.valueOf(0) === Test.SimpleEnum.red);
                test(Test.SimpleEnum.valueOf(1) === Test.SimpleEnum.green);
                test(Test.SimpleEnum.valueOf(2) === Test.SimpleEnum.blue);

                out.writeLine("ok");
            }
        ).then(() =>
            {
                out.write("testing enum operations... ");
                return proxy.opByte(Test.ByteEnum.benum1);
            }
        ).then(r =>
            {
                const [ret, b1] = r;
                test(ret === b1);
                test(ret === Test.ByteEnum.benum1);
                return proxy.opByte(Test.ByteEnum.benum11);
            }
        ).then(r =>
            {
                const [ret, b11] = r;
                test(ret === b11);
                test(ret === Test.ByteEnum.benum11);
                return proxy.opShort(Test.ShortEnum.senum1);
            }
        ).then(r =>
            {
                const [ret, s1] = r;
                test(ret === s1);
                test(ret === Test.ShortEnum.senum1);
                return proxy.opShort(Test.ShortEnum.senum11);
            }
        ).then(r =>
            {
                const [ret, s11] = r;
                test(ret === s11);
                test(ret === Test.ShortEnum.senum11);
                return proxy.opInt(Test.IntEnum.ienum1);
            }
        ).then(r =>
            {
                const [ret, i1] = r;
                test(ret === i1);
                test(ret === Test.IntEnum.ienum1);
                return proxy.opInt(Test.IntEnum.ienum11);
            }
        ).then(r =>
            {
                const [ret, i11] = r;
                test(ret === i11);
                test(ret === Test.IntEnum.ienum11);
                return proxy.opInt(Test.IntEnum.ienum12);
            }
        ).then(r =>
            {
                const [ret, i12] = r;
                test(ret === i12);
                test(ret === Test.IntEnum.ienum12);
                return proxy.opSimple(Test.SimpleEnum.green);
            }
        ).then(r =>
            {
                const [ret, g] = r;
                test(ret === g);
                test(ret === Test.SimpleEnum.green);

                out.writeLine("ok");
                return proxy.shutdown();
            }
        ).then(p.resolve, p.reject);
        
        return p;
    };

    function run(out, id)
    {
        var c = Ice.initialize(id);
        return Promise.try(() => allTests(out, c)).finally(() => c.destroy());
    };
    exports._test = run;
    exports._runServer = true;
}
(typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? module : undefined,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? require : this.Ice._require,
 typeof(global) !== "undefined" && typeof(global.process) !== "undefined" ? exports : this));
