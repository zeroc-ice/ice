// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using Test;

public class AllTests : TestCommon.TestApp
{
    //
    // There does not appear to be any way to compare collections
    // in either C# or with the .NET framework. Something like
    // C++ STL EqualRange would be nice...
    //
    private static bool Compare(ICollection c1, ICollection c2)
    {
        if (c1 == null)
        {
            return c2 == null;
        }
        if (c2 == null)
        {
            return false;
        }
        if (!c1.GetType().Equals(c2.GetType()))
        {
            return false;
        }

        if (c1.Count != c2.Count)
        {
            return false;
        }

        IEnumerator i1 = c1.GetEnumerator();
        IEnumerator i2 = c2.GetEnumerator();
        while (i1.MoveNext())
        {
            i2.MoveNext();
            if (i1.Current is ICollection)
            {
                Debug.Assert(i2.Current is ICollection);
                if (!Compare((ICollection)i1.Current, (ICollection)i2.Current))
                {
                    return false;
                }
            }
            else if (!i1.Current.Equals(i2.Current))
            {
                return false;
            }
        }
        return true;
    }

    private class TestObjectWriter : Ice.ObjectWriter
    {
        public TestObjectWriter(Test.MyClass obj)
        {
            this.obj = obj;
        }

        public override void write(Ice.OutputStream @out)
        {
            obj.write__(@out);
            called = true;
        }

        internal Test.MyClass obj;
        internal bool called = false;
    }

    private class TestObjectReader : Ice.ObjectReader
    {
        public override void read(Ice.InputStream @in)
        {
            obj = new Test.MyClass();
            obj.read__(@in);
            called = true;
        }

        internal Test.MyClass obj;
        internal bool called = false;
    }

    private class MyInterfaceI : Test.MyInterfaceDisp_
    {
    }

    private static Ice.Object MyInterfaceFactory(string type)
    {
        Debug.Assert(type.Equals(Test.MyInterfaceDisp_.ice_staticId()));
        return new MyInterfaceI();
    }

    private static Ice.Object TestObjectFactory(string type)
    {
        Debug.Assert(type.Equals(Test.MyClass.ice_staticId()));
        return new TestObjectReader();
    }

    private class TestReadObjectCallback
    {
        public void invoke(Ice.Object obj)
        {
            this.obj = obj;
        }

        internal Ice.Object obj;
    }

    public class MyClassFactoryWrapper
    {
        public MyClassFactoryWrapper()
        {
            _factory = null;
        }

        public Ice.Object create(string type)
        {
            if (_factory != null)
            {
                return _factory(type);
            }
            return new Test.MyClass();
        }

        public void setFactory(Ice.ValueFactory factory)
        {
            _factory = factory;
        }

        private Ice.ValueFactory _factory;
    }

    static public int run(Ice.Communicator communicator)
    {
        MyClassFactoryWrapper factoryWrapper = new MyClassFactoryWrapper();

        communicator.getValueFactoryManager().add(factoryWrapper.create, Test.MyClass.ice_staticId());
        communicator.getValueFactoryManager().add(MyInterfaceFactory, Test.MyInterfaceDisp_.ice_staticId());

        Ice.InputStream @in;
        Ice.OutputStream @out;

        Write("testing primitive types... ");
        Flush();

        {
            byte[] data = new byte[0];
            @in = new Ice.InputStream(communicator, data);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.startEncapsulation();
            @out.writeBool(true);
            @out.endEncapsulation();
            byte[] data = @out.finished();

            @in = new Ice.InputStream(communicator, data);
            @in.startEncapsulation();
            test(@in.readBool());
            @in.endEncapsulation();

            @in = new Ice.InputStream(communicator, data);
            @in.startEncapsulation();
            test(@in.readBool());
            @in.endEncapsulation();
        }

        {
            byte[] data = new byte[0];
            @in = new Ice.InputStream(communicator, data);
            try
            {
                @in.readBool();
                test(false);
            }
            catch (Ice.UnmarshalOutOfBoundsException)
            {
            }
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeBool(true);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readBool());
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeByte((byte)1);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readByte() == (byte)1);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeShort((short)2);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readShort() == (short)2);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeInt(3);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readInt() == 3);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeLong(4);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readLong() == 4);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeFloat((float)5.0);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readFloat() == (float)5.0);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeDouble(6.0);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readDouble() == 6.0);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeString("hello world");
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readString().Equals("hello world"));
        }

        WriteLine("ok");

        Write("testing constructed types... ");
        Flush();

        {
            int max = 2;
            @out = new Ice.OutputStream(communicator);
            @out.writeEnum((int)Test.MyEnum.enum3, max);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.MyEnum e = (Test.MyEnum)@in.readEnum(max);
            test(e == Test.MyEnum.enum3);
        }

        {
            @out = new Ice.OutputStream(communicator);
            Test.SmallStruct s = new Test.SmallStruct();
            s.bo = true;
            s.by = (byte)1;
            s.sh = (short)2;
            s.i = 3;
            s.l = 4;
            s.f = (float)5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = Test.MyEnum.enum2;
            s.p = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
            s.write__(@out);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.SmallStruct s2 = new Test.SmallStruct();
            s2.read__(@in);
            test(s2.Equals(s));
        }

        {
            @out = new Ice.OutputStream(communicator);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = (byte)5;
            o.sh = 4;
            o.i = 3;
            @out.writeObject(o);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            @in.readObject(cb.invoke);
            @in.readPendingObjects();
            OptionalClass o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            if(communicator.getProperties().getProperty("Ice.Default.EncodingVersion").Equals("1.0"))
            {
                test(!o2.sh.HasValue);
                test(!o2.i.HasValue);
            }
            else
            {
                test(o2.sh.Value == o.sh.Value);
                test(o2.i.Value == o.i.Value);
            }
        }

        {
            @out = new Ice.OutputStream(communicator, Ice.Util.Encoding_1_0);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = 5;
            o.sh = 4;
            o.i = 3;
            @out.writeObject(o);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, Ice.Util.Encoding_1_0, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            @in.readObject(cb.invoke);
            @in.readPendingObjects();
            OptionalClass o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            test(!o2.sh.HasValue);
            test(!o2.i.HasValue);
        }

        {
            bool[] arr =
            {
                true,
                false,
                true,
                false
            };
            @out = new Ice.OutputStream(communicator);
            Ice.BoolSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            bool[] arr2 = Ice.BoolSeqHelper.read(@in);
            test(Compare(arr2, arr));

            bool[][] arrS =
            {
                arr,
                new bool[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.BoolSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            bool[][] arr2S = Test.BoolSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            byte[] arr =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            @out = new Ice.OutputStream(communicator);
            Ice.ByteSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            byte[] arr2 = Ice.ByteSeqHelper.read(@in);
            test(Compare(arr2, arr));

            byte[][] arrS =
            {
                arr,
                new byte[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.ByteSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            byte[][] arr2S = Test.ByteSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            Serialize.Small small = new Serialize.Small();
            small.i = 99;
            @out = new Ice.OutputStream(communicator);
            @out.writeSerializable(small);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Serialize.Small small2 = (Serialize.Small)@in.readSerializable();
            test(small2.i == 99);
        }

        {
            short[] arr =
            {
                (short)0x01,
                (short)0x11,
                (short)0x12,
                (short)0x22
            };
            @out = new Ice.OutputStream(communicator);
            Ice.ShortSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            short[] arr2 = Ice.ShortSeqHelper.read(@in);
            test(Compare(arr2, arr));

            short[][] arrS =
            {
                arr,
                new short[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.ShortSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            short[][] arr2S = Test.ShortSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            int[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = new Ice.OutputStream(communicator);
            Ice.IntSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            int[] arr2 = Ice.IntSeqHelper.read(@in);
            test(Compare(arr2, arr));

            int[][] arrS =
            {
                arr,
                new int[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.IntSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            int[][] arr2S = Test.IntSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            long[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = new Ice.OutputStream(communicator);
            Ice.LongSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            long[] arr2 = Ice.LongSeqHelper.read(@in);
            test(Compare(arr2, arr));

            long[][] arrS =
            {
                arr,
                new long[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.LongSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            long[][] arr2S = Test.LongSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            float[] arr =
            {
                (float)1,
                (float)2,
                (float)3,
                (float)4
            };
            @out = new Ice.OutputStream(communicator);
            Ice.FloatSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            float[] arr2 = Ice.FloatSeqHelper.read(@in);
            test(Compare(arr2, arr));

            float[][] arrS =
            {
                arr,
                new float[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.FloatSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            float[][] arr2S = Test.FloatSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            double[] arr =
            {
                (double)1,
                (double)2,
                (double)3,
                (double)4
            };
            @out = new Ice.OutputStream(communicator);
            Ice.DoubleSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            double[] arr2 = Ice.DoubleSeqHelper.read(@in);
            test(Compare(arr2, arr));

            double[][] arrS =
            {
                arr,
                new double[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.DoubleSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            double[][] arr2S = Test.DoubleSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            string[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            @out = new Ice.OutputStream(communicator);
            Ice.StringSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            string[] arr2 = Ice.StringSeqHelper.read(@in);
            test(Compare(arr2, arr));

            string[][] arrS =
            {
                arr,
                new string[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.StringSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            string[][] arr2S = Test.StringSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            Test.MyEnum[] arr =
            {
                Test.MyEnum.enum3,
                Test.MyEnum.enum2,
                Test.MyEnum.enum1,
                Test.MyEnum.enum2
            };
            @out = new Ice.OutputStream(communicator);
            Test.MyEnumSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.MyEnum[] arr2 = Test.MyEnumSHelper.read(@in);
            test(Compare(arr2, arr));

            Test.MyEnum[][] arrS =
            {
                arr,
                new Test.MyEnum[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Test.MyEnumSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.MyEnum[][] arr2S = Test.MyEnumSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        Test.SmallStruct[] smallStructArray = new Test.SmallStruct[3];
        for (int i = 0; i < smallStructArray.Length; ++i)
        {
            smallStructArray[i] = new Test.SmallStruct();
            smallStructArray[i].bo = true;
            smallStructArray[i].by = (byte)1;
            smallStructArray[i].sh = (short)2;
            smallStructArray[i].i = 3;
            smallStructArray[i].l = 4;
            smallStructArray[i].f = (float)5.0;
            smallStructArray[i].d = 6.0;
            smallStructArray[i].str = "7";
            smallStructArray[i].e = Test.MyEnum.enum2;
            smallStructArray[i].p = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
        }

        Test.MyClass[] myClassArray = new Test.MyClass[4];
        for (int i = 0; i < myClassArray.Length; ++i)
        {
            myClassArray[i] = new Test.MyClass();
            myClassArray[i].c = myClassArray[i];
            myClassArray[i].o = myClassArray[i];
            myClassArray[i].s = new Test.SmallStruct();
            myClassArray[i].s.e = Test.MyEnum.enum2;
            myClassArray[i].seq1 = new bool[] { true, false, true, false };
            myClassArray[i].seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
            myClassArray[i].seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
            myClassArray[i].seq4 = new int[] { 1, 2, 3, 4 };
            myClassArray[i].seq5 = new long[] { 1, 2, 3, 4 };
            myClassArray[i].seq6 = new float[] { (float)1, (float)2, (float)3, (float)4 };
            myClassArray[i].seq7 = new double[] { (double)1, (double)2, (double)3, (double)4 };
            myClassArray[i].seq8 = new string[] { "string1", "string2", "string3", "string4" };
            myClassArray[i].seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
            myClassArray[i].seq10 = new Test.MyClass[4]; // null elements.
            myClassArray[i].d = new System.Collections.Generic.Dictionary<string, Test.MyClass>();
            myClassArray[i].d["hi"] = myClassArray[i];
        }

        {
            @out = new Ice.OutputStream(communicator);
            Test.MyClassSHelper.write(@out, myClassArray);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.MyClass[] arr2 = Test.MyClassSHelper.read(@in);
            @in.readPendingObjects();
            test(arr2.Length == myClassArray.Length);
            for (int i = 0; i < arr2.Length; ++i)
            {
                test(arr2[i] != null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == Test.MyEnum.enum2);
                test(Compare(arr2[i].seq1, myClassArray[i].seq1));
                test(Compare(arr2[i].seq2, myClassArray[i].seq2));
                test(Compare(arr2[i].seq3, myClassArray[i].seq3));
                test(Compare(arr2[i].seq4, myClassArray[i].seq4));
                test(Compare(arr2[i].seq5, myClassArray[i].seq5));
                test(Compare(arr2[i].seq6, myClassArray[i].seq6));
                test(Compare(arr2[i].seq7, myClassArray[i].seq7));
                test(Compare(arr2[i].seq8, myClassArray[i].seq8));
                test(Compare(arr2[i].seq9, myClassArray[i].seq9));
                test(arr2[i].d["hi"].Equals(arr2[i]));
            }

            Test.MyClass[][] arrS =
            {
                myClassArray,
                new Test.MyClass[0],
                myClassArray
            };
            @out = new Ice.OutputStream(communicator);
            Test.MyClassSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Test.MyClass[][] arr2S = Test.MyClassSSHelper.read(@in);
            test(arr2S.Length == arrS.Length);
            test(arr2S[0].Length == arrS[0].Length);
            test(arr2S[1].Length == arrS[1].Length);
            test(arr2S[2].Length == arrS[2].Length);
        }

        {
            Test.MyInterface i = new MyInterfaceI();
            @out = new Ice.OutputStream(communicator);
            @out.writeObject(i);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            @in.readObject(cb.invoke);
            @in.readPendingObjects();
            test(cb.obj != null);
        }

        {
            @out = new Ice.OutputStream(communicator);
            Test.MyClass obj = new Test.MyClass();
            obj.s = new Test.SmallStruct();
            obj.s.e = Test.MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            @out.writeObject(writer);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            test(writer.called);
            factoryWrapper.setFactory(TestObjectFactory);
            @in = new Ice.InputStream(communicator, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            @in.readObject(cb.invoke);
            @in.readPendingObjects();
            test(cb.obj != null);
            test(cb.obj is TestObjectReader);
            TestObjectReader reader = (TestObjectReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == Test.MyEnum.enum2);
            factoryWrapper.setFactory(null);
        }

        {
            @out = new Ice.OutputStream(communicator);
            Test.MyException ex = new Test.MyException();

            Test.MyClass c = new Test.MyClass();
            c.c = c;
            c.o = c;
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum2;
            c.seq1 = new bool[] { true, false, true, false };
            c.seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
            c.seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
            c.seq4 = new int[] { 1, 2, 3, 4 };
            c.seq5 = new long[] { 1, 2, 3, 4 };
            c.seq6 = new float[] { (float)1, (float)2, (float)3, (float)4 };
            c.seq7 = new double[] { (double)1, (double)2, (double)3, (double)4 };
            c.seq8 = new string[] { "string1", "string2", "string3", "string4" };
            c.seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
            c.seq10 = new Test.MyClass[4]; // null elements.
            c.d = new Dictionary<string, Test.MyClass>();
            c.d.Add("hi", c);

            ex.c = c;

            @out.writeException(ex);
            byte[] data = @out.finished();

            @in = new Ice.InputStream(communicator, data);
            try
            {
                @in.throwException();
                test(false);
            }
            catch (Test.MyException ex1)
            {
                test(ex1.c.s.e == c.s.e);
                test(Compare(ex1.c.seq1, c.seq1));
                test(Compare(ex1.c.seq2, c.seq2));
                test(Compare(ex1.c.seq3, c.seq3));
                test(Compare(ex1.c.seq4, c.seq4));
                test(Compare(ex1.c.seq5, c.seq5));
                test(Compare(ex1.c.seq6, c.seq6));
                test(Compare(ex1.c.seq7, c.seq7));
                test(Compare(ex1.c.seq8, c.seq8));
                test(Compare(ex1.c.seq9, c.seq9));
            }
            catch (Ice.UserException)
            {
                test(false);
            }
        }

        {
            Dictionary<byte, bool> dict = new Dictionary<byte, bool>();
            dict.Add((byte)4, true);
            dict.Add((byte)1, false);
            @out = new Ice.OutputStream(communicator);
            Test.ByteBoolDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Dictionary<byte, bool> dict2 = Test.ByteBoolDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<short, int> dict = new Dictionary<short, int>();
            dict.Add((short)1, 9);
            dict.Add((short)4, 8);
            @out = new Ice.OutputStream(communicator);
            Test.ShortIntDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Dictionary<short, int> dict2 = Test.ShortIntDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<long, float> dict = new Dictionary<long, float>();
            dict.Add((long)123809828, (float)0.51f);
            dict.Add((long)123809829, (float)0.56f);
            @out = new Ice.OutputStream(communicator);
            Test.LongFloatDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Dictionary<long, float> dict2 = Test.LongFloatDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<string, string> dict = new Dictionary<string, string>();
            dict.Add("key1", "value1");
            dict.Add("key2", "value2");
            @out = new Ice.OutputStream(communicator);
            Test.StringStringDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Dictionary<string, string> dict2 = Test.StringStringDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<string, Test.MyClass> dict = new Dictionary<string, Test.MyClass>();
            Test.MyClass c;
            c = new Test.MyClass();
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum2;
            dict.Add("key1", c);
            c = new Test.MyClass();
            c.s = new Test.SmallStruct();
            c.s.e = Test.MyEnum.enum3;
            dict.Add("key2", c);
            @out = new Ice.OutputStream(communicator);
            Test.StringMyClassDHelper.write(@out, dict);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Dictionary<string, Test.MyClass> dict2 = Test.StringMyClassDHelper.read(@in);
            @in.readPendingObjects();
            test(dict2.Count == dict.Count);
            test(dict2["key1"].s.e == Test.MyEnum.enum2);
            test(dict2["key2"].s.e == Test.MyEnum.enum3);
        }

        {
            bool[] arr =
            {
                true,
                false,
                true,
                false
            };
            @out = new Ice.OutputStream(communicator);
            List<bool> l = new List<bool>(arr);
            Test.BoolListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<bool> l2 = Test.BoolListHelper.read(@in);
            test(Compare(l, l2));
        }

        {
            byte[] arr =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            @out = new Ice.OutputStream(communicator);
            List<byte> l = new List<byte>(arr);
            Test.ByteListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<byte> l2 = Test.ByteListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            Test.MyEnum[] arr =
            {
                Test.MyEnum.enum3,
                Test.MyEnum.enum2,
                Test.MyEnum.enum1,
                Test.MyEnum.enum2
            };
            @out = new Ice.OutputStream(communicator);
            List<Test.MyEnum> l = new List<Test.MyEnum>(arr);
            Test.MyEnumListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<Test.MyEnum> l2 = Test.MyEnumListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            List<Test.SmallStruct> l = new List<Test.SmallStruct>(smallStructArray);
            Test.SmallStructListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<Test.SmallStruct> l2 = Test.SmallStructListHelper.read(@in);
            test(l2.Count == l.Count);
            for (int i = 0; i < l2.Count; ++i)
            {
                test(l2[i].Equals(smallStructArray[i]));
            }
        }

        {
            @out = new Ice.OutputStream(communicator);
            List<Test.MyClass> l = new List<Test.MyClass>(myClassArray);
            Test.MyClassListHelper.write(@out, l);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<Test.MyClass> l2 = Test.MyClassListHelper.read(@in);
            @in.readPendingObjects();
            test(l2.Count == l.Count);
            for (int i = 0; i < l2.Count; ++i)
            {
                test(l2[i] != null);
                test(l2[i].c == l2[i]);
                test(l2[i].o == l2[i]);
                test(l2[i].s.e == Test.MyEnum.enum2);
                test(Compare(l2[i].seq1, l[i].seq1));
                test(Compare(l2[i].seq2, l[i].seq2));
                test(Compare(l2[i].seq3, l[i].seq3));
                test(Compare(l2[i].seq4, l[i].seq4));
                test(Compare(l2[i].seq5, l[i].seq5));
                test(Compare(l2[i].seq6, l[i].seq6));
                test(Compare(l2[i].seq7, l[i].seq7));
                test(Compare(l2[i].seq8, l[i].seq8));
                test(Compare(l2[i].seq9, l[i].seq9));
                test(l2[i].d["hi"].Equals(l2[i]));
            }
        }

        {
            Test.MyClassPrx[] arr = new Test.MyClassPrx[2];
            arr[0] = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
            arr[1] = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("one"));
            @out = new Ice.OutputStream(communicator);
            List<Test.MyClassPrx> l = new List<Test.MyClassPrx>(arr);
            Test.MyClassProxyListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<Test.MyClassPrx> l2 = Test.MyClassProxyListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            short[] arr =
            {
                (short)0x01,
                (short)0x11,
                (short)0x12,
                (short)0x22
            };
            @out = new Ice.OutputStream(communicator);
            LinkedList<short> l = new LinkedList<short>(arr);
            Test.ShortLinkedListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            LinkedList<short> l2 = Test.ShortLinkedListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            int[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = new Ice.OutputStream(communicator);
            LinkedList<int> l = new LinkedList<int>(arr);
            Test.IntLinkedListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            LinkedList<int> l2 = Test.IntLinkedListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            Test.MyEnum[] arr =
            {
                Test.MyEnum.enum3,
                Test.MyEnum.enum2,
                Test.MyEnum.enum1,
                Test.MyEnum.enum2
            };
            @out = new Ice.OutputStream(communicator);
            LinkedList<Test.MyEnum> l = new LinkedList<Test.MyEnum>(arr);
            Test.MyEnumLinkedListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            LinkedList<Test.MyEnum> l2 = Test.MyEnumLinkedListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            LinkedList<Test.SmallStruct> l = new LinkedList<Test.SmallStruct>(smallStructArray);
            Test.SmallStructLinkedListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            LinkedList<Test.SmallStruct> l2 = Test.SmallStructLinkedListHelper.read(@in);
            test(l2.Count == l.Count);
            IEnumerator<Test.SmallStruct> e = l.GetEnumerator();
            IEnumerator<Test.SmallStruct> e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            long[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = new Ice.OutputStream(communicator);
            Stack<long> l = new Stack<long>(arr);
            Test.LongStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Stack<long> l2 = Test.LongStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            float[] arr =
            {
                (float)1,
                (float)2,
                (float)3,
                (float)4
            };
            @out = new Ice.OutputStream(communicator);
            Stack<float> l = new Stack<float>(arr);
            Test.FloatStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Stack<float> l2 = Test.FloatStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            Stack<Test.SmallStruct> l = new Stack<Test.SmallStruct>(smallStructArray);
            Test.SmallStructStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Stack<Test.SmallStruct> l2 = Test.SmallStructStackHelper.read(@in);
            test(l2.Count == l.Count);
            IEnumerator<Test.SmallStruct> e = l.GetEnumerator();
            IEnumerator<Test.SmallStruct> e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            Test.MyClassPrx[] arr = new Test.MyClassPrx[2];
            arr[0] = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
            arr[1] = Test.MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("one"));
            @out = new Ice.OutputStream(communicator);
            Stack<Test.MyClassPrx> l = new Stack<Test.MyClassPrx>(arr);
            Test.MyClassProxyStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Stack<Test.MyClassPrx> l2 = Test.MyClassProxyStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            double[] arr =
            {
                (double)1,
                (double)2,
                (double)3,
                (double)4
            };
            @out = new Ice.OutputStream(communicator);
            Queue<double> l = new Queue<double>(arr);
            Test.DoubleQueueHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Queue<double> l2 = Test.DoubleQueueHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            string[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            @out = new Ice.OutputStream(communicator);
            Queue<string> l = new Queue<string>(arr);
            Test.StringQueueHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Queue<string> l2 = Test.StringQueueHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            Queue<Test.SmallStruct> l = new Queue<Test.SmallStruct>(smallStructArray);
            Test.SmallStructQueueHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Queue<Test.SmallStruct> l2 = Test.SmallStructQueueHelper.read(@in);
            test(l2.Count == l.Count);
            IEnumerator<Test.SmallStruct> e = l.GetEnumerator();
            IEnumerator<Test.SmallStruct> e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            string[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            string[][] arrS =
            {
                arr,
                new string[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            List<string[]> l = new List<string[]>(arrS);
            Test.StringSListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            List<string[]> l2 = Test.StringSListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            string[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            string[][] arrS =
            {
                arr,
                new string[0],
                arr
            };
            @out = new Ice.OutputStream(communicator);
            Stack<string[]> l = new Stack<string[]>(arrS);
            Test.StringSStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            Stack<string[]> l2 = Test.StringSStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            SortedDictionary<string, string> dict = new SortedDictionary<string, string>();
            dict.Add("key1", "value1");
            dict.Add("key2", "value2");
            @out = new Ice.OutputStream(communicator);
            Test.SortedStringStringDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            IDictionary<string, string> dict2 = Test.SortedStringStringDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        WriteLine("ok");
        return 0;
    }
}
