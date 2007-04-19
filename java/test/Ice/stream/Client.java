// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class Client
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static class TestObjectWriter extends Ice.ObjectWriter
    {
        TestObjectWriter(Test.MyClass obj)
        {
            this.obj = obj;
        }

        public void
        write(Ice.OutputStream out)
        {
            obj.__write(out);
            called = true;
        }

        Test.MyClass obj;
        boolean called = false;
    }

    private static class TestObjectReader extends Ice.ObjectReader
    {
        public void
        read(Ice.InputStream in, boolean rid)
        {
            obj = new Test.MyClass();
            obj.__read(in, rid);
            called = true;
        }

        Test.MyClass obj;
        boolean called = false;
    }

    private static class TestObjectFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals(Test.MyClass.ice_staticId()));
            return new TestObjectReader();
        }

        public void
        destroy()
        {
        }
    }

    private static class MyInterfaceI extends Test._MyInterfaceDisp
    {
    };

    private static class MyInterfaceFactory extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        public Ice.Object
        create(String type)
        {
            assert(type.equals(Test._MyInterfaceDisp.ice_staticId()));
            return new MyInterfaceI();
        }
        
        public void
        destroy()
        {
        }
    }

    private static class TestReadObjectCallback implements Ice.ReadObjectCallback
    {
        public void
        invoke(Ice.Object obj)
        {
            this.obj = obj;
        }

        Ice.Object obj;
    }

    private static class MyClassFactoryWrapper extends Ice.LocalObjectImpl implements Ice.ObjectFactory
    {
        MyClassFactoryWrapper()
        {
            _factory = Test.MyClass.ice_factory();
        }

        public Ice.Object
        create(String type)
        {
            return _factory.create(type);
        }

        public void
        destroy()
        {
        }

        void
        setFactory(Ice.ObjectFactory factory)
        {
            _factory = factory;
        }

        private Ice.ObjectFactory _factory;
    }

    private static int
    run(String[] args, Ice.Communicator communicator)
    {
        MyClassFactoryWrapper factoryWrapper = new MyClassFactoryWrapper();
        communicator.addObjectFactory(factoryWrapper, Test.MyClass.ice_staticId());
        communicator.addObjectFactory(new MyInterfaceFactory(), Test._MyInterfaceDisp.ice_staticId());

        Ice.InputStream in;
        Ice.OutputStream out;

        System.out.print("testing primitive types... ");
        System.out.flush();

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeBool(true);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readBool());
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeByte((byte)1);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readByte() == (byte)1);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeShort((short)2);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readShort() == (short)2);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeInt(3);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readInt() == 3);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeLong(4);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readLong() == 4);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeFloat((float)5.0);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readFloat() == (float)5.0);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeDouble(6.0);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readDouble() == 6.0);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            out.writeString("hello world");
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(in.readString().equals("hello world"));
            out.destroy();
            in.destroy();
        }

        System.out.println("ok");

        System.out.print("testing constructed types... ");
        System.out.flush();

        {
            out = Ice.Util.createOutputStream(communicator);
            Test.MyEnum.enum3.ice_write(out);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            test(Test.MyEnum.ice_read(in) == Test.MyEnum.enum3);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(communicator);
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
            s.ice_write(out);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            Test.SmallStruct s2 = new Test.SmallStruct();
            s2.ice_read(in);
            test(s2.equals(s));
            out.destroy();
            in.destroy();
        }

        {
            final boolean[] arr =
            {
                true,
                false,
                true,
                false
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.BoolSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            boolean[] arr2 = Test.BoolSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final byte[] arr =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.ByteSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            byte[] arr2 = Test.ByteSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final short[] arr =
            {
                (short)0x01,
                (short)0x11,
                (short)0x12,
                (short)0x22
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.ShortSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            short[] arr2 = Test.ShortSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final int[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.IntSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            int[] arr2 = Test.IntSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final long[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.LongSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            long[] arr2 = Test.LongSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final float[] arr =
            {
                (float)1,
                (float)2,
                (float)3,
                (float)4
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.FloatSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            float[] arr2 = Test.FloatSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final double[] arr =
            {
                (double)1,
                (double)2,
                (double)3,
                (double)4
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.DoubleSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            double[] arr2 = Test.DoubleSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final String[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.StringSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            String[] arr2 = Test.StringSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            final Test.MyEnum[] arr =
            {
                Test.MyEnum.enum3,
                Test.MyEnum.enum2,
                Test.MyEnum.enum1,
                Test.MyEnum.enum2
            };
            out = Ice.Util.createOutputStream(communicator);
            Test.MyEnumSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            Test.MyEnum[] arr2 = Test.MyEnumSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();
        }

        {
            Test.MyClass[] arr = new Test.MyClass[4];
            for(int i = 0; i < arr.length; ++i)
            {
                arr[i] = new Test.MyClass();
                arr[i].c = arr[i];
                arr[i].o = arr[i];
                arr[i].s = new Test.SmallStruct();
                arr[i].s.e = Test.MyEnum.enum2;
                arr[i].seq1 = new boolean[] { true, false, true, false };
                arr[i].seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
                arr[i].seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
                arr[i].seq4 = new int[] { 1, 2, 3, 4 };
                arr[i].seq5 = new long[] { 1, 2, 3, 4 };
                arr[i].seq6 = new float[] { (float)1, (float)2, (float)3, (float)4 };
                arr[i].seq7 = new double[] { (double)1, (double)2, (double)3, (double)4 };
                arr[i].seq8 = new String[] { "string1", "string2", "string3", "string4" };
                arr[i].seq9 = new Test.MyEnum[] { Test.MyEnum.enum3, Test.MyEnum.enum2, Test.MyEnum.enum1 };
                arr[i].seq10 = new Test.MyClass[4]; // null elements.
                arr[i].d = new java.util.HashMap();
                arr[i].d.put("hi", arr[i]);
            }
            out = Ice.Util.createOutputStream(communicator);
            Test.MyClassSHelper.write(out, arr);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            Test.MyClass[] arr2 = Test.MyClassSHelper.read(in);
            in.readPendingObjects();
            test(arr2.length == arr.length);
            for(int i = 0; i < arr2.length; ++i)
            {
                test(arr2[i] != null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == Test.MyEnum.enum2);
                test(java.util.Arrays.equals(arr2[i].seq1, arr[i].seq1));
                test(java.util.Arrays.equals(arr2[i].seq2, arr[i].seq2));
                test(java.util.Arrays.equals(arr2[i].seq3, arr[i].seq3));
                test(java.util.Arrays.equals(arr2[i].seq4, arr[i].seq4));
                test(java.util.Arrays.equals(arr2[i].seq5, arr[i].seq5));
                test(java.util.Arrays.equals(arr2[i].seq6, arr[i].seq6));
                test(java.util.Arrays.equals(arr2[i].seq7, arr[i].seq7));
                test(java.util.Arrays.equals(arr2[i].seq8, arr[i].seq8));
                test(java.util.Arrays.equals(arr2[i].seq9, arr[i].seq9));
                test(arr2[i].d.get("hi") == arr2[i]);
            }
            out.destroy();
            in.destroy();
        }
        
        {
            Test.MyInterface i = new MyInterfaceI();
            out = Ice.Util.createOutputStream(communicator);
            Test.MyInterfaceHelper.write(out, i);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(communicator, data);
            Test.MyInterfaceHolder j = new Test.MyInterfaceHolder();
            Test.MyInterfaceHelper.read(in, j);
            in.readPendingObjects();
            test(j.value != null);
        }

        {
            out = Ice.Util.createOutputStream(communicator);
            Test.MyClass obj = new Test.MyClass();
            obj.s = new Test.SmallStruct();
            obj.s.e = Test.MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            out.writeObject(writer);
            out.writePendingObjects();
            byte[] data = out.finished();
            test(writer.called);
            factoryWrapper.setFactory(new TestObjectFactory());
            in = Ice.Util.createInputStream(communicator, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            in.readObject(cb);
            in.readPendingObjects();
            test(cb.obj != null);
            test(cb.obj instanceof TestObjectReader);
            TestObjectReader reader = (TestObjectReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == Test.MyEnum.enum2);
            out.destroy();
            in.destroy();
        }

        System.out.println("ok");

        return 0;
    }

    public static void
    main(String[] args)
    {
        int status = 0;
        Ice.Communicator communicator = null;

        try
        {
            communicator = Ice.Util.initialize(args);
            status = run(args, communicator);
        }
        catch(Ice.LocalException ex)
        {
            ex.printStackTrace();
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
                ex.printStackTrace();
                status = 1;
            }
        }

        System.gc();
        System.exit(status);
    }
}
