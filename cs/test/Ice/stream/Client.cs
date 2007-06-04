// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;

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
            if(!i1.Current.Equals(i2.Current))
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
        }

        {
            String[] arr =
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
            String[] arr2 = Test.StringSHelper.read(@in);
            test(Compare(arr2, arr));
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
                arr[i].seq8 = new String[] { "string1", "string2", "string3", "string4" };
                arr[i].seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
                arr[i].seq10 = new Test.MyClass[4]; // null elements.
                arr[i].d = new Test.StringMyClassD();
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
        catch(Ice.LocalException ex)
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
