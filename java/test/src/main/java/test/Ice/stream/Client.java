// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.stream;

import java.io.PrintWriter;

import Ice.BoolSeqHelper;
import Ice.ByteSeqHelper;
import Ice.DoubleSeqHelper;
import Ice.FloatSeqHelper;
import Ice.ShortSeqHelper;
import Ice.IntSeqHelper;
import Ice.LongSeqHelper;
import Ice.StringSeqHelper;
import test.Ice.stream.Test.*;

public class Client extends test.Util.Application
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
        TestObjectWriter(MyClass obj)
        {
            this.obj = obj;
        }

        @Override
        public void
        write(Ice.OutputStream out)
        {
            obj.__write(out);
            called = true;
        }

        MyClass obj;
        boolean called = false;
    }

    private static class TestObjectReader extends Ice.ObjectReader
    {
        @Override
        public void
        read(Ice.InputStream in)
        {
            obj = new MyClass();
            obj.__read(in);
            called = true;
        }

        MyClass obj;
        boolean called = false;
    }

    private static class TestObjectFactory implements Ice.ObjectFactory
    {
        @Override
        public Ice.Object
        create(String type)
        {
            assert(type.equals(MyClass.ice_staticId()));
            return new TestObjectReader();
        }

        @Override
        public void
        destroy()
        {
        }
    }

    private static class MyInterfaceI extends _MyInterfaceDisp
    {
    };

    private static class MyInterfaceFactory implements Ice.ObjectFactory
    {
        @Override
        public Ice.Object
        create(String type)
        {
            assert(type.equals(_MyInterfaceDisp.ice_staticId()));
            return new MyInterfaceI();
        }
        
        @Override
        public void
        destroy()
        {
        }
    }

    private static class TestReadObjectCallback implements Ice.ReadObjectCallback
    {
        @Override
        public void
        invoke(Ice.Object obj)
        {
            this.obj = obj;
        }

        Ice.Object obj;
    }

    private static class MyClassFactoryWrapper implements Ice.ObjectFactory
    {
        MyClassFactoryWrapper()
        {
            _factory = MyClass.ice_factory();
        }

        @Override
        public Ice.Object
        create(String type)
        {
            return _factory.create(type);
        }

        @Override
        public void
        destroy()
        {
        }

        void
        setFactory(Ice.ObjectFactory factory)
        {
            if(factory == null)
            {
                _factory = MyClass.ice_factory();
            }
            else
            {
                _factory = factory;
            }
        }

        private Ice.ObjectFactory _factory;
    }

    @Override
    public int
    run(String[] args)
    {
        Ice.Communicator comm = communicator();
        MyClassFactoryWrapper factoryWrapper = new MyClassFactoryWrapper();
        comm.addObjectFactory(factoryWrapper, MyClass.ice_staticId());
        comm.addObjectFactory(new MyInterfaceFactory(), _MyInterfaceDisp.ice_staticId());

        Ice.InputStream in;
        Ice.OutputStream out;

        PrintWriter printWriter = getWriter();
        printWriter.print("testing primitive types... ");
        printWriter.flush();

        {
            byte[] data = new byte[0];
            in = Ice.Util.createInputStream(comm, data);
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.startEncapsulation();
            out.writeBool(true);
            out.endEncapsulation();
            byte[] data = out.finished();
            out.destroy();

            in = Ice.Util.createInputStream(comm, data);
            in.startEncapsulation();
            test(in.readBool());
            in.endEncapsulation();
            in.destroy();

            in = Ice.Util.wrapInputStream(comm, data);
            in.startEncapsulation();
            test(in.readBool());
            in.endEncapsulation();
            in.destroy();
        }

        {
            byte[] data = new byte[0];
            in = Ice.Util.createInputStream(comm, data);
            try
            {
                in.readBool();
                test(false);
            }
            catch(Ice.UnmarshalOutOfBoundsException ex)
            {
            }
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeBool(true);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readBool());
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeByte((byte)1);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readByte() == (byte)1);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeShort((short)2);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readShort() == (short)2);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeInt(3);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readInt() == 3);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeLong(4);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readLong() == 4);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeFloat((float)5.0);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readFloat() == (float)5.0);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeDouble(6.0);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readDouble() == 6.0);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            out.writeString("hello world");
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(in.readString().equals("hello world"));
            out.destroy();
            in.destroy();
        }

        printWriter.println("ok");

        printWriter.print("testing constructed types... ");
        printWriter.flush();

        {
            out = Ice.Util.createOutputStream(comm);
            MyEnum.enum3.ice_write(out);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test(MyEnum.ice_read(in) == MyEnum.enum3);
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            SmallStruct s = new SmallStruct();
            s.bo = true;
            s.by = (byte)1;
            s.sh = (short)2;
            s.i = 3;
            s.l = 4;
            s.f = (float)5.0;
            s.d = 6.0;
            s.str = "7";
            s.e = MyEnum.enum2;
            s.p = MyClassPrxHelper.uncheckedCast(comm.stringToProxy("test:default"));
            s.ice_write(out);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            SmallStruct s2 = new SmallStruct();
            s2.ice_read(in);
            test(s2.equals(s));
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = (byte)5;
            o.setSh((short)4);
            o.setI(3);
            out.writeObject(o);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            in.readObject(cb);
            in.readPendingObjects();
            OptionalClass o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            if(comm.getProperties().getProperty("Ice.Default.EncodingVersion").equals("1.0"))
            {
                test(!o2.hasSh());
                test(!o2.hasI());
            }
            else
            {
                test(o2.getSh() == o.getSh());
                test(o2.getI() == o.getI());
            }
            out.destroy();
            in.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm, Ice.Util.Encoding_1_0);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = (byte)5;
            o.setSh((short)4);
            o.setI(3);
            out.writeObject(o);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data, Ice.Util.Encoding_1_0);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            in.readObject(cb);
            in.readPendingObjects();
            OptionalClass o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            test(!o2.hasSh());
            test(!o2.hasI());
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
            out = Ice.Util.createOutputStream(comm);
            BoolSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            boolean[] arr2 = BoolSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final boolean[][] arrS =
            {
                arr,
                new boolean[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            BoolSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            boolean[][] arr2S = BoolSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
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
            out = Ice.Util.createOutputStream(comm);
            ByteSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            byte[] arr2 = ByteSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final byte[][] arrS =
            {
                arr,
                new byte[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            ByteSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            byte[][] arr2S = ByteSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
            out.destroy();
            in.destroy();
        }

        {
            test.Ice.stream.Serialize.Small small = new test.Ice.stream.Serialize.Small();
            small.i = 99;
            out = Ice.Util.createOutputStream(comm);
            out.writeSerializable(small);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            test.Ice.stream.Serialize.Small small2 = (test.Ice.stream.Serialize.Small)in.readSerializable();
            test(small2.i == 99);
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
            out = Ice.Util.createOutputStream(comm);
            ShortSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            short[] arr2 = ShortSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final short[][] arrS =
            {
                arr,
                new short[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            ShortSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            short[][] arr2S = ShortSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
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
            out = Ice.Util.createOutputStream(comm);
            IntSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            int[] arr2 = IntSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final int[][] arrS =
            {
                arr,
                new int[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            IntSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            int[][] arr2S = IntSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
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
            out = Ice.Util.createOutputStream(comm);
            LongSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            long[] arr2 = LongSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final long[][] arrS =
            {
                arr,
                new long[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            LongSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            long[][] arr2S = LongSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
            out.destroy();
            in.destroy();
        }

        {
            final float[] arr =
            {
                1,
                2,
                3,
                4
            };
            out = Ice.Util.createOutputStream(comm);
            FloatSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            float[] arr2 = FloatSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final float[][] arrS =
            {
                arr,
                new float[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            FloatSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            float[][] arr2S = FloatSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
            out.destroy();
            in.destroy();
        }

        {
            final double[] arr =
            {
                1,
                2,
                3,
                4
            };
            out = Ice.Util.createOutputStream(comm);
            DoubleSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            double[] arr2 = DoubleSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final double[][] arrS =
            {
                arr,
                new double[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            DoubleSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            double[][] arr2S = DoubleSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
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
            out = Ice.Util.createOutputStream(comm);
            StringSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            String[] arr2 = StringSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final String[][] arrS =
            {
                arr,
                new String[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            StringSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            String[][] arr2S = StringSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
            out.destroy();
            in.destroy();
        }

        {
            final MyEnum[] arr =
            {
                MyEnum.enum3,
                MyEnum.enum2,
                MyEnum.enum1,
                MyEnum.enum2
            };
            out = Ice.Util.createOutputStream(comm);
            MyEnumSHelper.write(out, arr);
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            MyEnum[] arr2 = MyEnumSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));
            out.destroy();
            in.destroy();

            final MyEnum[][] arrS =
            {
                arr,
                new MyEnum[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            MyEnumSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            MyEnum[][] arr2S = MyEnumSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
            out.destroy();
            in.destroy();
        }

        {
            MyClass[] arr = new MyClass[4];
            for(int i = 0; i < arr.length; ++i)
            {
                arr[i] = new MyClass();
                arr[i].c = arr[i];
                arr[i].o = arr[i];
                arr[i].s = new SmallStruct();
                arr[i].s.e = MyEnum.enum2;
                arr[i].seq1 = new boolean[] { true, false, true, false };
                arr[i].seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
                arr[i].seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
                arr[i].seq4 = new int[] { 1, 2, 3, 4 };
                arr[i].seq5 = new long[] { 1, 2, 3, 4 };
                arr[i].seq6 = new float[] { 1, 2, 3, 4 };
                arr[i].seq7 = new double[] { 1, 2, 3, 4 };
                arr[i].seq8 = new String[] { "string1", "string2", "string3", "string4" };
                arr[i].seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
                arr[i].seq10 = new MyClass[4]; // null elements.
                arr[i].d = new java.util.HashMap<String, MyClass>();
                arr[i].d.put("hi", arr[i]);
            }
            out = Ice.Util.createOutputStream(comm);
            MyClassSHelper.write(out, arr);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            MyClass[] arr2 = MyClassSHelper.read(in);
            in.readPendingObjects();
            test(arr2.length == arr.length);
            for(int i = 0; i < arr2.length; ++i)
            {
                test(arr2[i] != null);
                test(arr2[i].c == arr2[i]);
                test(arr2[i].o == arr2[i]);
                test(arr2[i].s.e == MyEnum.enum2);
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

            final MyClass[][] arrS =
            {
                arr,
                new MyClass[0],
                arr
            };
            out = Ice.Util.createOutputStream(comm);
            MyClassSSHelper.write(out, arrS);
            data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            MyClass[][] arr2S = MyClassSSHelper.read(in);
            test(arr2S.length == arrS.length);
            test(arr2S[0].length == arrS[0].length);
            test(arr2S[1].length == arrS[1].length);
            test(arr2S[2].length == arrS[2].length);
            out.destroy();
            in.destroy();
        }
        
        {
            MyInterface i = new MyInterfaceI();
            out = Ice.Util.createOutputStream(comm);
            MyInterfaceHelper.write(out, i);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            MyInterfaceHolder j = new MyInterfaceHolder();
            MyInterfaceHelper.read(in, j);
            in.readPendingObjects();
            test(j.value != null);
        }

        {
            out = Ice.Util.createOutputStream(comm);
            MyClass obj = new MyClass();
            obj.s = new SmallStruct();
            obj.s.e = MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            out.writeObject(writer);
            out.writePendingObjects();
            out.finished();
            test(writer.called);
            out.destroy();
        }

        {
            out = Ice.Util.createOutputStream(comm);
            MyClass obj = new MyClass();
            obj.s = new SmallStruct();
            obj.s.e = MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            out.writeObject(writer);
            out.writePendingObjects();
            byte[] data = out.finished();
            test(writer.called);
            factoryWrapper.setFactory(new TestObjectFactory());
            in = Ice.Util.createInputStream(comm, data);
            TestReadObjectCallback cb = new TestReadObjectCallback();
            in.readObject(cb);
            in.readPendingObjects();
            test(cb.obj != null);
            test(cb.obj instanceof TestObjectReader);
            TestObjectReader reader = (TestObjectReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == MyEnum.enum2);
            out.destroy();
            in.destroy();
            factoryWrapper.setFactory(null);
        }

        {
            out = Ice.Util.createOutputStream(comm);
            MyException ex = new MyException();

            MyClass c = new MyClass();
            c.c = c;
            c.o = c;
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum2;
            c.seq1 = new boolean[] { true, false, true, false };
            c.seq2 = new byte[] { (byte)1, (byte)2, (byte)3, (byte)4 };
            c.seq3 = new short[] { (short)1, (short)2, (short)3, (short)4 };
            c.seq4 = new int[] { 1, 2, 3, 4 };
            c.seq5 = new long[] { 1, 2, 3, 4 };
            c.seq6 = new float[] { 1, 2, 3, 4 };
            c.seq7 = new double[] { 1, 2, 3, 4 };
            c.seq8 = new String[] { "string1", "string2", "string3", "string4" };
            c.seq9 = new MyEnum[] { MyEnum.enum3, MyEnum.enum2, MyEnum.enum1 };
            c.seq10 = new MyClass[4]; // null elements.
            c.d = new java.util.HashMap<String, MyClass>();
            c.d.put("hi", c);

            ex.c = c;
        
            out.writeException(ex);
            byte[] data = out.finished();
 
            in = Ice.Util.createInputStream(comm, data);
            try
            {
                in.throwException();
                test(false);
            }
            catch(MyException ex1)
            {
                test(ex1.c.s.e == c.s.e);
                test(java.util.Arrays.equals(ex1.c.seq1, c.seq1));
                test(java.util.Arrays.equals(ex1.c.seq2, c.seq2));
                test(java.util.Arrays.equals(ex1.c.seq3, c.seq3));
                test(java.util.Arrays.equals(ex1.c.seq4, c.seq4));
                test(java.util.Arrays.equals(ex1.c.seq5, c.seq5));
                test(java.util.Arrays.equals(ex1.c.seq6, c.seq6));
                test(java.util.Arrays.equals(ex1.c.seq7, c.seq7));
                test(java.util.Arrays.equals(ex1.c.seq8, c.seq8));
                test(java.util.Arrays.equals(ex1.c.seq9, c.seq9));
            }
            catch(Ice.UserException ex1)
            {
                test(false);
            }
        }

        {
            java.util.Map<Byte, Boolean> dict = new java.util.HashMap<Byte, Boolean>();
            dict.put((byte)4, true);
            dict.put((byte)1, false);
            out = Ice.Util.createOutputStream(comm);
            ByteBoolDHelper.write(out, dict);
            byte data[] = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            java.util.Map<Byte, Boolean> dict2 = ByteBoolDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<Short, Integer> dict = new java.util.HashMap<Short, Integer>();
            dict.put((short)1, 9);
            dict.put((short)4, 8);
            out = Ice.Util.createOutputStream(comm);
            ShortIntDHelper.write(out, dict);
            byte data[] = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            java.util.Map<Short, Integer> dict2 = ShortIntDHelper.read(in);
            test(dict2.equals(dict));
        }


        {
            java.util.Map<Long, Float> dict = new java.util.HashMap<Long, Float>();
            dict.put((long)123809828, 0.51f);
            dict.put((long)123809829, 0.56f);
            out = Ice.Util.createOutputStream(comm);
            LongFloatDHelper.write(out, dict);
            byte data[] = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            java.util.Map<Long, Float> dict2 = LongFloatDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<String, String> dict = new java.util.HashMap<String, String>();
            dict.put("key1", "value1");
            dict.put("key2", "value2");
            out = Ice.Util.createOutputStream(comm);
            StringStringDHelper.write(out, dict);
            byte data[] = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            java.util.Map<String, String> dict2 = StringStringDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<String, MyClass> dict = new java.util.HashMap<String, MyClass>();
            MyClass c;
            c = new MyClass();
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum2;
            dict.put("key1", c);
            c = new MyClass();
            c.s = new SmallStruct();
            c.s.e = MyEnum.enum3;
            dict.put("key2", c);
            out = Ice.Util.createOutputStream(comm);
            StringMyClassDHelper.write(out, dict);
            out.writePendingObjects();
            byte[] data = out.finished();
            in = Ice.Util.createInputStream(comm, data);
            java.util.Map<String, MyClass> dict2 = StringMyClassDHelper.read(in);
            in.readPendingObjects();
            test(dict2.size() == dict.size());
            test(dict2.get("key1").s.e == MyEnum.enum2);
            test(dict2.get("key2").s.e == MyEnum.enum3);
        }

        printWriter.println("ok");

        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.stream");
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
