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

    private class TestValueWriter : Ice.ValueWriter
    {
        public TestValueWriter(MyClass obj)
        {
            this.obj = obj;
        }

        public override void write(Ice.OutputStream @out)
        {
            obj.write__(@out);
            called = true;
        }

        internal MyClass obj;
        internal bool called = false;
    }

    private class TestValueReader : Ice.ValueReader
    {
        public override void read(Ice.InputStream @in)
        {
            obj = new MyClass();
            obj.read__(@in);
            called = true;
        }

        internal MyClass obj;
        internal bool called = false;
    }

    private static Ice.Value TestObjectFactory(string type)
    {
        Debug.Assert(type.Equals(MyClass.ice_staticId()));
        return new TestValueReader();
    }

    private class TestReadValueCallback
    {
        public void invoke(Ice.Value obj)
        {
            this.obj = obj;
        }

        internal Ice.Value obj;
    }

    public class MyClassFactoryWrapper
    {
        public MyClassFactoryWrapper()
        {
            _factory = null;
        }

        public Ice.Value create(string type)
        {
            if (_factory != null)
            {
                return _factory(type);
            }
            return new MyClass();
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

        communicator.getValueFactoryManager().add(factoryWrapper.create, MyClass.ice_staticId());

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
            var data = @out.finished();

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
            var data = new byte[0];
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
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readBool());
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeByte(1);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readByte() == 1);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeShort(2);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readShort() == 2);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeInt(3);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readInt() == 3);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeLong(4);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readLong() == 4);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeFloat((float)5.0);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readFloat() == (float)5.0);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeDouble(6.0);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readDouble() == 6.0);
        }

        {
            @out = new Ice.OutputStream(communicator);
            @out.writeString("hello world");
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            test(@in.readString().Equals("hello world"));
        }

        WriteLine("ok");

        Write("testing constructed types... ");
        Flush();

        {
            @out = new Ice.OutputStream(communicator);
            MyEnumHelper.write(@out, MyEnum.enum3);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var e = MyEnumHelper.read(@in);
            test(e == MyEnum.enum3);
        }

        {
            @out = new Ice.OutputStream(communicator);
            var s = new SmallStruct();
            s.bo = true;
            s.by = 1;
            s.sh = 2;
            s.i = 3;
            s.l = 4;
            s.f = 5.0f;
            s.d = 6.0;
            s.str = "7";
            s.e = MyEnum.enum2;
            s.p = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
            s.write__(@out);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var s2 = new SmallStruct();
            s2.read__(@in);
            test(s2.Equals(s));
        }

        {
            @out = new Ice.OutputStream(communicator);
            var o = new OptionalClass();
            o.bo = true;
            o.by = 5;
            o.sh = 4;
            o.i = 3;
            @out.writeValue(o);
            @out.writePendingValues();
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            TestReadValueCallback cb = new TestReadValueCallback();
            @in.readValue(cb.invoke);
            @in.readPendingValues();
            var o2 = (OptionalClass)cb.obj;
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
            var o = new OptionalClass();
            o.bo = true;
            o.by = 5;
            o.sh = 4;
            o.i = 3;
            @out.writeValue(o);
            @out.writePendingValues();
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, Ice.Util.Encoding_1_0, data);
            var cb = new TestReadValueCallback();
            @in.readValue(cb.invoke);
            @in.readPendingValues();
            var o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            test(!o2.sh.HasValue);
            test(!o2.i.HasValue);
        }

        {
            bool[] arr = { true, false, true, false };
            @out = new Ice.OutputStream(communicator);
            Ice.BoolSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.BoolSeqHelper.read(@in);
            test(Compare(arr2, arr));

            bool[][] arrS = { arr, new bool[0], arr };
            @out = new Ice.OutputStream(communicator);
            BoolSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = BoolSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            Ice.ByteSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.ByteSeqHelper.read(@in);
            test(Compare(arr2, arr));

            byte[][] arrS = { arr, new byte[0], arr };
            @out = new Ice.OutputStream(communicator);
            ByteSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = ByteSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            Serialize.Small small = new Serialize.Small();
            small.i = 99;
            @out = new Ice.OutputStream(communicator);
            @out.writeSerializable(small);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var small2 = (Serialize.Small)@in.readSerializable();
            test(small2.i == 99);
        }

        {
            short[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            Ice.ShortSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.ShortSeqHelper.read(@in);
            test(Compare(arr2, arr));

            short[][] arrS = { arr, new short[0], arr };
            @out = new Ice.OutputStream(communicator);
            ShortSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = ShortSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            int[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            Ice.IntSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.IntSeqHelper.read(@in);
            test(Compare(arr2, arr));

            int[][] arrS = { arr, new int[0], arr };
            @out = new Ice.OutputStream(communicator);
            IntSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = IntSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            long[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            Ice.LongSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.LongSeqHelper.read(@in);
            test(Compare(arr2, arr));

            long[][] arrS = { arr, new long[0], arr };
            @out = new Ice.OutputStream(communicator);
            LongSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = LongSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            float[] arr = {1, 2, 3, 4};
            @out = new Ice.OutputStream(communicator);
            Ice.FloatSeqHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            float[] arr2 = Ice.FloatSeqHelper.read(@in);
            test(Compare(arr2, arr));

            float[][] arrS = { arr, new float[0], arr };
            @out = new Ice.OutputStream(communicator);
            FloatSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = FloatSSHelper.read(@in);
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
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.DoubleSeqHelper.read(@in);
            test(Compare(arr2, arr));

            double[][] arrS = { arr, new double[0], arr };
            @out = new Ice.OutputStream(communicator);
            DoubleSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = DoubleSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            string[] arr = { "string1", "string2", "string3", "string4" };
            @out = new Ice.OutputStream(communicator);
            Ice.StringSeqHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = Ice.StringSeqHelper.read(@in);
            test(Compare(arr2, arr));

            string[][] arrS = { arr, new string[0], arr };
            @out = new Ice.OutputStream(communicator);
            StringSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = StringSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        {
            MyEnum[] arr = { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2 };
            @out = new Ice.OutputStream(communicator);
            MyEnumSHelper.write(@out, arr);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = MyEnumSHelper.read(@in);
            test(Compare(arr2, arr));

            MyEnum[][] arrS = { arr, new MyEnum[0], arr };
            @out = new Ice.OutputStream(communicator);
            MyEnumSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = MyEnumSSHelper.read(@in);
            test(Compare(arr2S, arrS));
        }

        var smallStructArray = new SmallStruct[3];
        for (int i = 0; i < smallStructArray.Length; ++i)
        {
            smallStructArray[i] = new SmallStruct();
            smallStructArray[i].bo = true;
            smallStructArray[i].by = 1;
            smallStructArray[i].sh = 2;
            smallStructArray[i].i = 3;
            smallStructArray[i].l = 4;
            smallStructArray[i].f = 5.0f;
            smallStructArray[i].d = 6.0;
            smallStructArray[i].str = "7";
            smallStructArray[i].e = MyEnum.enum2;
            smallStructArray[i].p = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("test:default"));
        }

        var myClassArray = new MyClass[4];
        for (int i = 0; i < myClassArray.Length; ++i)
        {
            myClassArray[i] = new MyClass();
            myClassArray[i].c = myClassArray[i];
            myClassArray[i].o = myClassArray[i];
            myClassArray[i].s = new SmallStruct();
            myClassArray[i].s.e = MyEnum.enum2;
            myClassArray[i].seq1 = new bool[] { true, false, true, false };
            myClassArray[i].seq2 = new byte[] { 1, 2, 3, 4 };
            myClassArray[i].seq3 = new short[] { 1, 2, 3, 4 };
            myClassArray[i].seq4 = new int[] { 1, 2, 3, 4 };
            myClassArray[i].seq5 = new long[] { 1, 2, 3, 4 };
            myClassArray[i].seq6 = new float[] { 1, 2, 3, 4 };
            myClassArray[i].seq7 = new double[] { 1, 2, 3, 4 };
            myClassArray[i].seq8 = new string[] { "string1", "string2", "string3", "string4" };
            myClassArray[i].seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
            myClassArray[i].seq10 = new MyClass[4]; // null elements.
            myClassArray[i].d = new Dictionary<string, Test.MyClass>();
            myClassArray[i].d["hi"] = myClassArray[i];
        }

        {
            @out = new Ice.OutputStream(communicator);
            MyClassSHelper.write(@out, myClassArray);
            @out.writePendingValues();
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2 = MyClassSHelper.read(@in);
            @in.readPendingValues();
            test(arr2.Length == myClassArray.Length);
            for (int i = 0; i < arr2.Length; ++i)
            {
                test(arr2[i] != null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == MyEnum.enum2);
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

            MyClass[][] arrS = { myClassArray, new MyClass[0], myClassArray };
            @out = new Ice.OutputStream(communicator);
            MyClassSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var arr2S = MyClassSSHelper.read(@in);
            test(arr2S.Length == arrS.Length);
            test(arr2S[0].Length == arrS[0].Length);
            test(arr2S[1].Length == arrS[1].Length);
            test(arr2S[2].Length == arrS[2].Length);
        }

        {
            @out = new Ice.OutputStream(communicator);
            var obj = new MyClass();
            obj.s = new SmallStruct();
            obj.s.e = MyEnum.enum2;
            var writer = new TestValueWriter(obj);
            @out.writeValue(writer);
            @out.writePendingValues();
            var data = @out.finished();
            test(writer.called);
            factoryWrapper.setFactory(TestObjectFactory);
            @in = new Ice.InputStream(communicator, data);
            var cb = new TestReadValueCallback();
            @in.readValue(cb.invoke);
            @in.readPendingValues();
            test(cb.obj != null);
            test(cb.obj is TestValueReader);
            var reader = (TestValueReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == MyEnum.enum2);
            factoryWrapper.setFactory(null);
        }

        {
            @out = new Ice.OutputStream(communicator);
            var ex = new MyException();

            var c = new MyClass();
            c.c = c;
            c.o = c;
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum2;
            c.seq1 = new bool[] { true, false, true, false };
            c.seq2 = new byte[] { 1, 2, 3, 4 };
            c.seq3 = new short[] { 1, 2, 3, 4 };
            c.seq4 = new int[] { 1, 2, 3, 4 };
            c.seq5 = new long[] { 1, 2, 3, 4 };
            c.seq6 = new float[] { 1, 2, 3, 4 };
            c.seq7 = new double[] { 1, 2, 3, 4 };
            c.seq8 = new string[] { "string1", "string2", "string3", "string4" };
            c.seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
            c.seq10 = new MyClass[4]; // null elements.
            c.d = new Dictionary<string, MyClass>();
            c.d.Add("hi", c);

            ex.c = c;

            @out.writeException(ex);
            var data = @out.finished();

            @in = new Ice.InputStream(communicator, data);
            try
            {
                @in.throwException();
                test(false);
            }
            catch (MyException ex1)
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
            var dict = new Dictionary<byte, bool>();
            dict.Add(4, true);
            dict.Add(1, false);
            @out = new Ice.OutputStream(communicator);
            ByteBoolDHelper.write(@out, dict);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = ByteBoolDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            var dict = new Dictionary<short, int>();
            dict.Add(1, 9);
            dict.Add(4, 8);
            @out = new Ice.OutputStream(communicator);
            ShortIntDHelper.write(@out, dict);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = ShortIntDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            var dict = new Dictionary<long, float>();
            dict.Add(123809828, 0.51f);
            dict.Add(123809829, 0.56f);
            @out = new Ice.OutputStream(communicator);
            LongFloatDHelper.write(@out, dict);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = LongFloatDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            var dict = new Dictionary<string, string>();
            dict.Add("key1", "value1");
            dict.Add("key2", "value2");
            @out = new Ice.OutputStream(communicator);
            StringStringDHelper.write(@out, dict);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = StringStringDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            var dict = new Dictionary<string, MyClass>();
            var c = new MyClass();
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum2;
            dict.Add("key1", c);
            c = new MyClass();
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum3;
            dict.Add("key2", c);
            @out = new Ice.OutputStream(communicator);
            StringMyClassDHelper.write(@out, dict);
            @out.writePendingValues();
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = StringMyClassDHelper.read(@in);
            @in.readPendingValues();
            test(dict2.Count == dict.Count);
            test(dict2["key1"].s.e == MyEnum.enum2);
            test(dict2["key2"].s.e == MyEnum.enum3);
        }

        {
            bool[] arr = { true, false, true, false };
            @out = new Ice.OutputStream(communicator);
            var l = new List<bool>(arr);
            BoolListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = BoolListHelper.read(@in);
            test(Compare(l, l2));
        }

        {
            byte[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            var l = new List<byte>(arr);
            ByteListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = ByteListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            MyEnum[] arr = { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2 };
            @out = new Ice.OutputStream(communicator);
            var l = new List<MyEnum>(arr);
            MyEnumListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = MyEnumListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            var l = new List<SmallStruct>(smallStructArray);
            SmallStructListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = SmallStructListHelper.read(@in);
            test(l2.Count == l.Count);
            for (int i = 0; i < l2.Count; ++i)
            {
                test(l2[i].Equals(smallStructArray[i]));
            }
        }

        {
            @out = new Ice.OutputStream(communicator);
            var l = new List<MyClass>(myClassArray);
            MyClassListHelper.write(@out, l);
            @out.writePendingValues();
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = MyClassListHelper.read(@in);
            @in.readPendingValues();
            test(l2.Count == l.Count);
            for (int i = 0; i < l2.Count; ++i)
            {
                test(l2[i] != null);
                test(l2[i].c == l2[i]);
                test(l2[i].o == l2[i]);
                test(l2[i].s.e == MyEnum.enum2);
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
            var arr = new MyClassPrx[2];
            arr[0] = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
            arr[1] = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("one"));
            @out = new Ice.OutputStream(communicator);
            var l = new List<MyClassPrx>(arr);
            MyClassProxyListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = MyClassProxyListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            short[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            var l = new LinkedList<short>(arr);
            ShortLinkedListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = ShortLinkedListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            int[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            LinkedList<int> l = new LinkedList<int>(arr);
            Test.IntLinkedListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            LinkedList<int> l2 = Test.IntLinkedListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            MyEnum[] arr = { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2 };
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
            var l = new LinkedList<Test.SmallStruct>(smallStructArray);
            SmallStructLinkedListHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = SmallStructLinkedListHelper.read(@in);
            test(l2.Count == l.Count);
            var e = l.GetEnumerator();
            var e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            long[] arr = { 0x01, 0x11, 0x12, 0x22 };
            @out = new Ice.OutputStream(communicator);
            var l = new Stack<long>(arr);
            LongStackHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = LongStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            float[] arr = { 1, 2, 3, 4 };
            @out = new Ice.OutputStream(communicator);
            var l = new Stack<float>(arr);
            FloatStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = FloatStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            var l = new Stack<SmallStruct>(smallStructArray);
            SmallStructStackHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = SmallStructStackHelper.read(@in);
            test(l2.Count == l.Count);
            var e = l.GetEnumerator();
            var e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            var arr = new MyClassPrx[2];
            arr[0] = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("zero"));
            arr[1] = MyClassPrxHelper.uncheckedCast(communicator.stringToProxy("one"));
            @out = new Ice.OutputStream(communicator);
            var l = new Stack<MyClassPrx>(arr);
            MyClassProxyStackHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = MyClassProxyStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            double[] arr = { 1, 2, 3, 4 };
            @out = new Ice.OutputStream(communicator);
            var l = new Queue<double>(arr);
            DoubleQueueHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = DoubleQueueHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            string[] arr = { "string1", "string2", "string3", "string4" };
            @out = new Ice.OutputStream(communicator);
            var l = new Queue<string>(arr);
            StringQueueHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = StringQueueHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            @out = new Ice.OutputStream(communicator);
            var l = new Queue<SmallStruct>(smallStructArray);
            SmallStructQueueHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = SmallStructQueueHelper.read(@in);
            test(l2.Count == l.Count);
            var e = l.GetEnumerator();
            var e2 = l2.GetEnumerator();
            while (e.MoveNext() && e2.MoveNext())
            {
                test(e.Current.Equals(e2.Current));
            }
        }

        {
            string[] arr = { "string1", "string2", "string3", "string4" };
            string[][] arrS = { arr, new string[0], arr };
            @out = new Ice.OutputStream(communicator);
            var l = new List<string[]>(arrS);
            StringSListHelper.write(@out, l);
            byte[] data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = StringSListHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            string[] arr = { "string1", "string2", "string3", "string4" };
            string[][] arrS = { arr, new string[0], arr };
            @out = new Ice.OutputStream(communicator);
            var l = new Stack<string[]>(arrS);
            StringSStackHelper.write(@out, l);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var l2 = StringSStackHelper.read(@in);
            test(Compare(l2, l));
        }

        {
            var dict = new SortedDictionary<string, string>();
            dict.Add("key1", "value1");
            dict.Add("key2", "value2");
            @out = new Ice.OutputStream(communicator);
            SortedStringStringDHelper.write(@out, dict);
            var data = @out.finished();
            @in = new Ice.InputStream(communicator, data);
            var dict2 = SortedStringStringDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        WriteLine("ok");
        return 0;
    }
}
