// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import {Ice} from "ice";
import {Test} from "./generated";
import {TestHelper} from "../../../Common/TestHelper";

const test = TestHelper.test;

export class Client extends TestHelper
{
    allTests()
    {
        const communicator = this.communicator();
        const out = this.getWriter();

        class MyInterfaceI extends Ice.InterfaceByValue
        {
            constructor()
            {
                super(Test.MyInterface.ice_staticId());
            }
        }
        communicator.getValueFactoryManager().add(id => new MyInterfaceI(), Test.MyInterface.ice_staticId());

        let inS:Ice.InputStream = null;
        let outS:Ice.OutputStream = null;

        out.write("testing primitive types... ");

        {
            const data = new Uint8Array(0);
            inS = new Ice.InputStream(communicator, data);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.startEncapsulation();
            outS.writeBool(true);
            outS.endEncapsulation();
            const data = outS.finished();

            inS = new Ice.InputStream(communicator, data);
            inS.startEncapsulation();
            test(inS.readBool());
            inS.endEncapsulation();

            inS = new Ice.InputStream(communicator, data);
            inS.startEncapsulation();
            test(inS.readBool());
            inS.endEncapsulation();
        }

        {
            const data = new Uint8Array(0);
            inS = new Ice.InputStream(communicator, data);
            try
            {
                inS.readBool();
                test(false);
            }
            catch(ex)
            {
                test(ex instanceof Ice.UnmarshalOutOfBoundsException);
            }
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeBool(true);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readBool());
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeByte(1);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readByte() == 1);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeShort(2);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readShort() == 2);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeInt(3);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readInt() == 3);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeLong(new Ice.Long(4));
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readLong().toNumber() == 4);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeFloat(5.0);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readFloat() == 5.0);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeDouble(6.0);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readDouble() == 6.0);
        }

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeString("hello world");
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            test(inS.readString() == "hello world");
        }

        out.writeLine("ok");

        out.write("testing constructed types... ");

        {
            outS = new Ice.OutputStream(communicator);
            outS.writeEnum(Test.MyEnum.enum3);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const e:Test.MyEnum = inS.readEnum(Test.MyEnum);
            test(e == Test.MyEnum.enum3);
        }

        {
            outS = new Ice.OutputStream(communicator);
            const s = new Test.SmallStruct();
            s.bo = true;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = new Ice.Long(4);
            s.f = 5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = Test.MyEnum.enum2;
            s.p = Test.MyInterfacePrx.uncheckedCast(communicator.stringToProxy("test:default"));
            Test.SmallStruct.write(outS, s);
            const data = outS.finished();
            const s2 = Test.SmallStruct.read(new Ice.InputStream(communicator, data));
            test(s2.equals(s));
        }

        {
            outS = new Ice.OutputStream(communicator);
            const o = new Test.OptionalClass();
            o.bo = true;
            o.by = 5;
            o.sh = 4;
            o.i = 3;
            outS.writeValue(o);
            outS.writePendingValues();
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            let o2:Test.OptionalClass;
            inS.readValue((obj:Test.OptionalClass) =>
                            {
                                o2 = obj;
                            },
                            Test.OptionalClass);
            inS.readPendingValues();

            test(o2.bo == o.bo);
            test(o2.by == o.by);
            if(communicator.getProperties().getProperty("Ice.Default.EncodingVersion") == "1.0")
            {
                test(o2.sh === undefined);
                test(o2.i === undefined);
            }
            else
            {
                test(o2.sh == o.sh);
                test(o2.i == o.i);
            }
        }

        {
            outS = new Ice.OutputStream(communicator, Ice.Encoding_1_0);
            const o = new Test.OptionalClass();
            o.bo = true;
            o.by = 5;
            o.sh = 4;
            o.i = 3;
            outS.writeValue(o);
            outS.writePendingValues();
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, Ice.Encoding_1_0, data);
            let o2:Test.OptionalClass;
            inS.readValue((obj:Test.OptionalClass) =>
                            {
                                o2 = obj;
                            },
                            Test.OptionalClass);
            inS.readPendingValues();
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            test(o2.sh === undefined);
            test(o2.i === undefined);
        }

        {
            const arr = [true, false, true, false];
            outS = new Ice.OutputStream(communicator);
            Ice.BoolSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.BoolSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr, arr2));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.BoolSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.BoolSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = new Uint8Array([0x01, 0x11, 0x12, 0x22]);
            outS = new Ice.OutputStream(communicator);
            Ice.ByteSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.ByteSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS:Uint8Array[] = [arr, new Uint8Array(0), arr];
            outS = new Ice.OutputStream(communicator);
            Test.ByteSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.ByteSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [0x01, 0x11, 0x12, 0x22];
            outS = new Ice.OutputStream(communicator);
            Ice.ShortSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.ShortSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.ShortSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.ShortSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [0x01, 0x11, 0x12, 0x22];
            outS = new Ice.OutputStream(communicator);
            Ice.IntSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.IntSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.IntSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.IntSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [new Ice.Long(0x01), new Ice.Long(0x11), new Ice.Long(0x12), new Ice.Long(0x22)];
            outS = new Ice.OutputStream(communicator);
            Ice.LongSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.LongSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.LongSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.LongSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [1, 2, 3, 4];
            outS = new Ice.OutputStream(communicator);
            Ice.FloatSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.FloatSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.FloatSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.FloatSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [1, 2, 3, 4];
            outS = new Ice.OutputStream(communicator);
            Ice.DoubleSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.DoubleSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.DoubleSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.DoubleSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = ["string1", "string2", "string3", "string4"];
            outS = new Ice.OutputStream(communicator);
            Ice.StringSeqHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Ice.StringSeqHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.StringSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.StringSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        {
            const arr = [Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1, Test.MyEnum.enum2];
            outS = new Ice.OutputStream(communicator);
            Test.MyEnumSHelper.write(outS, arr);
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Test.MyEnumSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2, arr));

            const arrS = [arr, [], arr];
            outS = new Ice.OutputStream(communicator);
            Test.MyEnumSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.MyEnumSSHelper.read(inS);
            test(Ice.ArrayUtil.equals(arr2S, arrS));
        }

        const smallStructArray = [];
        for(let i = 0; i < 3; ++i)
        {
            const s = new Test.SmallStruct();
            s.bo = true;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = new Ice.Long(4);
            s.f = 5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = Test.MyEnum.enum2;
            s.p = Test.MyInterfacePrx.uncheckedCast(communicator.stringToProxy("test:default"));
            smallStructArray[i] = s;
        }

        const myClassArray = [];
        for(let i = 0; i < 4; ++i)
        {
            const c = new Test.MyClass();
            myClassArray[i] = c;
            c.c = myClassArray[i];
            c.o = myClassArray[i];
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum2;
            c.seq1 = [true, false, true, false];
            c.seq2 = new Uint8Array([1, 2, 3, 4]);
            c.seq3 = [1, 2, 3, 4];
            c.seq4 = [1, 2, 3, 4];
            c.seq5 = [new Ice.Long(1), new Ice.Long(2), new Ice.Long(3), new Ice.Long(4)];
            c.seq6 = [1, 2, 3, 4];
            c.seq7 = [1, 2, 3, 4];
            c.seq8 = ["string1", "string2", "string3", "string4"];
            c.seq9 = [Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1];
            c.seq10 = [null, null, null, null]; // null elements.
            c.d = new Test.StringMyClassD();
            c.d.set("hi", myClassArray[i]);
        }

        const myInterfaceArray = [];
        for(let i = 0; i < 4; ++i)
        {
            myInterfaceArray[i] = new Ice.InterfaceByValue("::Test::MyInterface");
        }

        {
            outS = new Ice.OutputStream(communicator);
            Test.MyClassSHelper.write(outS, myClassArray);
            outS.writePendingValues();
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Test.MyClassSHelper.read(inS);
            inS.readPendingValues();
            test(arr2.length == myClassArray.length);
            for(let i = 0; i < arr2.length; ++i)
            {
                test(arr2[i] !== null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == Test.MyEnum.enum2);
                test(Ice.ArrayUtil.equals(arr2[i].seq1, myClassArray[i].seq1));
                test(Ice.ArrayUtil.equals(arr2[i].seq2, myClassArray[i].seq2));
                test(Ice.ArrayUtil.equals(arr2[i].seq3, myClassArray[i].seq3));
                test(Ice.ArrayUtil.equals(arr2[i].seq4, myClassArray[i].seq4));
                test(Ice.ArrayUtil.equals(arr2[i].seq5, myClassArray[i].seq5));
                test(Ice.ArrayUtil.equals(arr2[i].seq6, myClassArray[i].seq6));
                test(Ice.ArrayUtil.equals(arr2[i].seq7, myClassArray[i].seq7));
                test(Ice.ArrayUtil.equals(arr2[i].seq8, myClassArray[i].seq8));
                test(Ice.ArrayUtil.equals(arr2[i].seq9, myClassArray[i].seq9));
                test(arr2[i].d.get("hi") == arr2[i]);
            }
            const arrS = [myClassArray, [], myClassArray];
            outS = new Ice.OutputStream(communicator);
            Test.MyClassSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.MyClassSSHelper.read(inS);
            test(arr2S.length == arrS.length);
            test(arr2S[0].length == arrS[0].length);
            test(arr2S[1].length == arrS[1].length);
            test(arr2S[2].length == arrS[2].length);
        }

        {
            outS = new Ice.OutputStream(communicator);
            Test.MyInterfaceSHelper.write(outS, myInterfaceArray);
            outS.writePendingValues();
            let data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2 = Test.MyInterfaceSHelper.read(inS);
            inS.readPendingValues();
            test(arr2.length == myInterfaceArray.length);
            const arrS = [myInterfaceArray, [], myInterfaceArray];
            outS = new Ice.OutputStream(communicator);
            Test.MyInterfaceSSHelper.write(outS, arrS);
            data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const arr2S = Test.MyInterfaceSSHelper.read(inS);
            test(arr2S.length == arrS.length);
            test(arr2S[0].length == arrS[0].length);
            test(arr2S[1].length == arrS[1].length);
            test(arr2S[2].length == arrS[2].length);
        }

        {
            outS = new Ice.OutputStream(communicator);
            const ex = new Test.MyException();

            const c = new Test.MyClass();
            c.c = c;
            c.o = c;
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum2;
            c.seq1 = [true, false, true, false];
            c.seq2 = new Uint8Array([1, 2, 3, 4]);
            c.seq3 = [1, 2, 3, 4];
            c.seq4 = [1, 2, 3, 4];
            c.seq5 = [new Ice.Long(1), new Ice.Long(2), new Ice.Long(3), new Ice.Long(4)];
            c.seq6 = [1, 2, 3, 4];
            c.seq7 = [1, 2, 3, 4];
            c.seq8 = ["string1", "string2", "string3", "string4"];
            c.seq9 = [Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1];
            c.seq10 = [null, null, null, null]; // null elements.
            c.d = new Test.StringMyClassD();
            c.d.set("hi", c);

            ex.c = c;

            outS.writeException(ex);
            const data = outS.finished();

            inS = new Ice.InputStream(communicator, data);
            try
            {
                inS.throwException();
                test(false);
            }
            catch(ex1)
            {
                test(ex1 instanceof Test.MyException);
                test(ex1.c.s.e == c.s.e);
                test(Ice.ArrayUtil.equals(ex1.c.seq1, c.seq1));
                test(Ice.ArrayUtil.equals(ex1.c.seq2, c.seq2));
                test(Ice.ArrayUtil.equals(ex1.c.seq3, c.seq3));
                test(Ice.ArrayUtil.equals(ex1.c.seq4, c.seq4));
                test(Ice.ArrayUtil.equals(ex1.c.seq5, c.seq5));
                test(Ice.ArrayUtil.equals(ex1.c.seq6, c.seq6));
                test(Ice.ArrayUtil.equals(ex1.c.seq7, c.seq7));
                test(Ice.ArrayUtil.equals(ex1.c.seq8, c.seq8));
                test(Ice.ArrayUtil.equals(ex1.c.seq9, c.seq9));
            }
        }

        {
            const dict = new Test.ByteBoolD();
            dict.set(4, true);
            dict.set(1, false);
            outS = new Ice.OutputStream(communicator);
            Test.ByteBoolDHelper.write(outS, dict);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const dict2 = Test.ByteBoolDHelper.read(inS);
            test(Ice.MapUtil.equals(dict2, dict));
        }

        {
            const dict = new Test.ShortIntD();
            dict.set(1, 9);
            dict.set(4, 8);
            outS = new Ice.OutputStream(communicator);
            Test.ShortIntDHelper.write(outS, dict);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const dict2 = Test.ShortIntDHelper.read(inS);
            test(Ice.MapUtil.equals(dict2, dict));
        }

        {
            const dict = new Test.LongFloatD();
            dict.set(new Ice.Long(123809828), 0.5);
            dict.set(new Ice.Long(123809829), 0.6);
            outS = new Ice.OutputStream(communicator);
            Test.LongFloatDHelper.write(outS, dict);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const dict2 = Test.LongFloatDHelper.read(inS);
            test(dict2.size == 2);
            test(dict2.get(new Ice.Long(123809828)) == 0.5);
            test(Math.abs(dict2.get(new Ice.Long(123809829)) - 0.6) <= 0.001);
        }

        {
            const dict = new Test.StringStringD();
            dict.set("key1", "value1");
            dict.set("key2", "value2");
            outS = new Ice.OutputStream(communicator);
            Test.StringStringDHelper.write(outS, dict);
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const dict2 = Test.StringStringDHelper.read(inS);
            test(Ice.MapUtil.equals(dict2, dict));
        }

        {
            const dict = new Test.StringMyClassD();
            let c = new Test.MyClass();
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum2;
            dict.set("key1", c);
            c = new Test.MyClass();
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum3;
            dict.set("key2", c);
            outS = new Ice.OutputStream(communicator);
            Test.StringMyClassDHelper.write(outS, dict);
            outS.writePendingValues();
            const data = outS.finished();
            inS = new Ice.InputStream(communicator, data);
            const dict2 = Test.StringMyClassDHelper.read(inS);
            inS.readPendingValues();
            test(dict2.size == dict.size);
            test(dict2.get("key1").s.e == Test.MyEnum.enum2);
            test(dict2.get("key2").s.e == Test.MyEnum.enum3);
        }
        out.writeLine("ok");
    }

    async run(args:string[])
    {
        let communicator:Ice.Communicator;
        try
        {
            [communicator] = this.initialize(args);
            this.allTests();
        }
        finally
        {
            if(communicator)
            {
                await communicator.destroy();
            }
        }
    }
}
