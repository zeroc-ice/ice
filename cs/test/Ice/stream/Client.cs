// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    //
    // There does not appear to be any way to compare collections
    // in either C# or with the .NET framework. Something like
    // C++ STL EqualRange would be nice...
    //
    private static bool Compare(ICollection c1, ICollection c2)
    {
        if(c1 == null)
        {
            return c2 == null;
        }
        if(c2 == null)
        {
            return false;
        }
        if(!c1.GetType().Equals(c2.GetType()))
        {
            return false;
        }
        
        if(c1.Count != c2.Count)
        {
            return false;
        }

        IEnumerator i1 = c1.GetEnumerator();
        IEnumerator i2 = c2.GetEnumerator();
        while(i1.MoveNext())
        {
            i2.MoveNext();
            if(i1.Current is ICollection)
            {
                Debug.Assert(i2.Current is ICollection);
                if(!Compare((ICollection)i1.Current, (ICollection)i2.Current))
                {
                    return false;
                }
            }
            else if(!i1.Current.Equals(i2.Current))
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
        public override void read(Ice.InputStream @in, bool rid)
        {
            obj = new Test.MyClass();
            obj.read__(@in, rid);
            called = true;
        }

        internal Test.MyClass obj;
        internal bool called = false;
    }

    private class MyInterfaceI : Test.MyInterfaceDisp_
    {
    }

    private class MyInterfaceFactory : Ice.ObjectFactory
    {
        public Ice.Object create(string type)
        {
            Debug.Assert(type.Equals(Test.MyInterfaceDisp_.ice_staticId()));
            return new MyInterfaceI();
        }

        public void destroy()
        {
        }
    }

    private class TestObjectFactory : Ice.ObjectFactory
    {
        public Ice.Object create(string type)
        {
            Debug.Assert(type.Equals(Test.MyClass.ice_staticId()));
            return new TestObjectReader();
        }

        public void destroy()
        {
        }
    }

    private class TestReadObjectCallback : Ice.ReadObjectCallback
    {
        public void invoke(Ice.Object obj)
        {
            this.obj = obj;
        }

        internal Ice.Object obj;
    }

    public class MyClassFactoryWrapper : Ice.ObjectFactory
    {
        public MyClassFactoryWrapper()
        {
            _factory = null;
        }

        public Ice.Object create(string type)
        {
            if(_factory != null)
            {
                return _factory.create(type);
            }
            return new Test.MyClass();
        }

        public void destroy()
        {
        }

        public void setFactory(Ice.ObjectFactory factory)
        {
            _factory = factory;
        }

        private Ice.ObjectFactory _factory;
    }

    private static int run(string[] args, Ice.Communicator communicator)
    {
        MyClassFactoryWrapper factoryWrapper = new MyClassFactoryWrapper();
        communicator.addObjectFactory(factoryWrapper, Test.MyClass.ice_staticId());
        communicator.addObjectFactory(new MyInterfaceFactory(), Test.MyInterfaceDisp_.ice_staticId());

        Ice.InputStream @in;
        Ice.OutputStream @out;

        Console.Write("testing primitive types... ");
        Console.Out.Flush();

        {
            byte[] data = new byte[0];
            @in = Ice.Util.createInputStream(communicator, data);
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.startEncapsulation();
            @out.writeBool(true);
            @out.endEncapsulation();
            byte[] data = @out.finished();
            @out.destroy();

            @in = Ice.Util.createInputStream(communicator, data);
            @in.startEncapsulation();
            test(@in.readBool());
            @in.endEncapsulation();
            @in.destroy();
        }

        {
            byte[] data = new byte[0];
            @in = Ice.Util.createInputStream(communicator, data);
            try
            {
                @in.readBool();
                test(false);
            }
            catch(Ice.UnmarshalOutOfBoundsException)
            {
            }
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeBool(true);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readBool());
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeByte((byte)1);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readByte() == (byte)1);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeShort((short)2);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readShort() == (short)2);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeInt(3);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readInt() == 3);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeLong(4);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readLong() == 4);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeFloat((float)5.0);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readFloat() == (float)5.0);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeDouble(6.0);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readDouble() == 6.0);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeString("hello world");
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(@in.readString().Equals("hello world"));
            @out.destroy();
            @in.destroy();
        }

        Console.WriteLine("ok");

        Console.Write("testing constructed types... ");
        Console.Out.Flush();

        {
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyEnumHelper.write(@out, Test.MyEnum.enum3);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            test(Test.MyEnumHelper.read(@in) == Test.MyEnum.enum3);
            @out.destroy();
            @in.destroy();
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
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
            s.ice_write(@out);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.SmallStruct s2 = new Test.SmallStruct();
            s2.ice_read(@in);
            test(s2.Equals(s));
            @out.destroy();
            @in.destroy();
        }

        {
            bool[] arr =
            {
                true,
                false,
                true,
                false
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.BoolSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            bool[] arr2 = Test.BoolSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            bool[][] arrS =
            {
                arr,
                new bool[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.BoolSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            bool[][] arr2S = Test.BoolSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            byte[] arr =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.ByteSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            byte[] arr2 = Test.ByteSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            byte[][] arrS =
            {
                arr,
                new byte[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.ByteSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            byte[][] arr2S = Test.ByteSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            Serialize.Small small = new Serialize.Small();
            small.i = 99;
            @out = Ice.Util.createOutputStream(communicator);
            @out.writeSerializable(small);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Serialize.Small small2 = (Serialize.Small)@in.readSerializable();
            test(small2.i == 99);
            @out.destroy();
            @in.destroy();
        }

        {
            short[] arr =
            {
                (short)0x01,
                (short)0x11,
                (short)0x12,
                (short)0x22
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.ShortSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            short[] arr2 = Test.ShortSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            short[][] arrS =
            {
                arr,
                new short[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.ShortSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            short[][] arr2S = Test.ShortSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            int[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.IntSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            int[] arr2 = Test.IntSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            int[][] arrS =
            {
                arr,
                new int[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.IntSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            int[][] arr2S = Test.IntSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            long[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.LongSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            long[] arr2 = Test.LongSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            long[][] arrS =
            {
                arr,
                new long[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.LongSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            long[][] arr2S = Test.LongSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            float[] arr =
            {
                (float)1,
                (float)2,
                (float)3,
                (float)4
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.FloatSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            float[] arr2 = Test.FloatSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            float[][] arrS =
            {
                arr,
                new float[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.FloatSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            float[][] arr2S = Test.FloatSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            double[] arr =
            {
                (double)1,
                (double)2,
                (double)3,
                (double)4
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.DoubleSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            double[] arr2 = Test.DoubleSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            double[][] arrS =
            {
                arr,
                new double[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.DoubleSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            double[][] arr2S = Test.DoubleSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            string[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.StringSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            string[] arr2 = Test.StringSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            string[][] arrS =
            {
                arr,
                new string[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.StringSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            string[][] arr2S = Test.StringSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            Test.MyEnum[] arr =
            {
                Test.MyEnum.enum3,
                Test.MyEnum.enum2,
                Test.MyEnum.enum1,
                Test.MyEnum.enum2
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyEnumSHelper.write(@out, arr);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.MyEnum[] arr2 = Test.MyEnumSHelper.read(@in);
            test(Compare(arr2, arr));
            @out.destroy();
            @in.destroy();

            Test.MyEnum[][] arrS =
            {
                arr,
                new Test.MyEnum[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyEnumSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.MyEnum[][] arr2S = Test.MyEnumSSHelper.read(@in);
            test(Compare(arr2S, arrS));
            @out.destroy();
            @in.destroy();
        }

        {
            Test.MyClass[] arr = new Test.MyClass[4];
            for(int i = 0; i < arr.Length; ++i)
            {
                arr[i] = new Test.MyClass();
                arr[i].c = arr[i];
                arr[i].o = arr[i];
                arr[i].s = new Test.SmallStruct();
                arr[i].s.e = Test.MyEnum.enum2;
                arr[i].seq1 = new bool[] { true, false, true, false };
                arr[i].seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
                arr[i].seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
                arr[i].seq4 = new int[] { 1, 2, 3, 4 };
                arr[i].seq5 = new long[] { 1, 2, 3, 4 };
                arr[i].seq6 = new float[] { (float)1, (float)2, (float)3, (float)4 };
                arr[i].seq7 = new double[] { (double)1, (double)2, (double)3, (double)4 };
                arr[i].seq8 = new string[] { "string1", "string2", "string3", "string4" };
                arr[i].seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
                arr[i].seq10 = new Test.MyClass[4]; // null elements.
                arr[i].d = new System.Collections.Generic.Dictionary<string, Test.MyClass>();
                arr[i].d["hi"] = arr[i];
            }
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyClassSHelper.write(@out, arr);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.MyClass[] arr2 = Test.MyClassSHelper.read(@in);
            @in.readPendingObjects();
            test(arr2.Length == arr.Length);
            for(int i = 0; i < arr2.Length; ++i)
            {
                test(arr2[i] != null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == Test.MyEnum.enum2);
                test(Compare(arr2[i].seq1, arr[i].seq1));
                test(Compare(arr2[i].seq2, arr[i].seq2));
                test(Compare(arr2[i].seq3, arr[i].seq3));
                test(Compare(arr2[i].seq4, arr[i].seq4));
                test(Compare(arr2[i].seq5, arr[i].seq5));
                test(Compare(arr2[i].seq6, arr[i].seq6));
                test(Compare(arr2[i].seq7, arr[i].seq7));
                test(Compare(arr2[i].seq8, arr[i].seq8));
                test(Compare(arr2[i].seq9, arr[i].seq9));
                test(arr2[i].d["hi"].Equals(arr2[i]));
            }
            @out.destroy();
            @in.destroy();

            Test.MyClass[][] arrS =
            {
                arr,
                new Test.MyClass[0],
                arr
            };
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyClassSSHelper.write(@out, arrS);
            data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.MyClass[][] arr2S = Test.MyClassSSHelper.read(@in);
            test(arr2S.Length == arrS.Length);
            test(arr2S[0].Length == arrS[0].Length);
            test(arr2S[1].Length == arrS[1].Length);
            test(arr2S[2].Length == arrS[2].Length);
            @out.destroy();
            @in.destroy();
        }

        {
            Test.MyInterface i = new MyInterfaceI();
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyInterfaceHelper.write(@out, i);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Test.MyInterfaceHelper helper = new Test.MyInterfaceHelper(@in);
            helper.read();
            @in.readPendingObjects();
            test(helper.value != null);
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
            Test.MyClass obj = new Test.MyClass();
            obj.s = new Test.SmallStruct();
            obj.s.e = Test.MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            @out.writeObject(writer);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            test(writer.called);
            factoryWrapper.setFactory(new TestObjectFactory());
            @in = Ice.Util.createInputStream(communicator, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            @in.readObject(cb);
            @in.readPendingObjects();
            test(cb.obj != null);
            test(cb.obj is TestObjectReader);
            TestObjectReader reader = (TestObjectReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == Test.MyEnum.enum2);
            @out.destroy();
            @in.destroy();
            factoryWrapper.setFactory(null);
        }

        {
            @out = Ice.Util.createOutputStream(communicator);
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
 
            @in = Ice.Util.createInputStream(communicator, data);
            try
            {
                @in.throwException();
                test(false);
            }
            catch(Test.MyException ex1)
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
            catch(Ice.UserException)
            {
                test(false);
            }
        }

        {
            Dictionary<byte, bool> dict = new Dictionary<byte, bool>();
            dict.Add((byte)4, true);
            dict.Add((byte)1, false);
            @out = Ice.Util.createOutputStream(communicator);
            Test.ByteBoolDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Dictionary<byte, bool> dict2 = Test.ByteBoolDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<short, int> dict = new Dictionary<short, int>();
            dict.Add((short)1, 9);
            dict.Add((short)4, 8);
            @out = Ice.Util.createOutputStream(communicator);
            Test.ShortIntDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Dictionary<short, int> dict2 = Test.ShortIntDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }


        {
            Dictionary<long, float> dict = new Dictionary<long, float>();
            dict.Add((long)123809828, (float)0.51f);
            dict.Add((long)123809829, (float)0.56f);
            @out = Ice.Util.createOutputStream(communicator);
            Test.LongFloatDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Dictionary<long, float> dict2 = Test.LongFloatDHelper.read(@in);
            test(Ice.CollectionComparer.Equals(dict2, dict));
        }

        {
            Dictionary<string, string> dict = new Dictionary<string, string>();
            dict.Add("key1", "value1");
            dict.Add("key2", "value2");
            @out = Ice.Util.createOutputStream(communicator);
            Test.StringStringDHelper.write(@out, dict);
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
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
            @out = Ice.Util.createOutputStream(communicator);
            Test.StringMyClassDHelper.write(@out, dict);
            @out.writePendingObjects();
            byte[] data = @out.finished();
            @in = Ice.Util.createInputStream(communicator, data);
            Dictionary<string, Test.MyClass> dict2 = Test.StringMyClassDHelper.read(@in);
            @in.readPendingObjects();
            test(dict2.Count == dict.Count);
            test(dict2["key1"].s.e == Test.MyEnum.enum2);
            test(dict2["key2"].s.e == Test.MyEnum.enum3);
        }

        Console.WriteLine("ok");

        return 0;
    }

    public static void Main(string[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        Debug.Listeners.Add(new ConsoleTraceListener());

        try
        {
            communicator = Ice.Util.initialize(ref args);
            status = run(args, communicator);
        }
        catch(System.Exception ex)
        {
            Console.Error.WriteLine(ex);
            status = 1;
        }

        if(communicator != null)
        {
            try
            {
                communicator.destroy();
            }
            catch(Ice.LocalException ex)
            {
                Console.Error.WriteLine(ex);
                status = 1;
            }
        }

        if(status != 0)
        {
            System.Environment.Exit(status);
        }
    }
}
