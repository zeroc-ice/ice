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
            obj._iceWrite(out);
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
            obj._iceRead(in);
            called = true;
        }

        MyClass obj;
        boolean called = false;
    }

    private static class TestValueFactory implements Ice.ValueFactory
    {
        @Override
        public Ice.Object
        create(String type)
        {
            assert(type.equals(MyClass.ice_staticId()));
            return new TestObjectReader();
        }
    }

    private static class MyInterfaceI extends _MyInterfaceDisp
    {
    }

    private static class MyInterfaceFactory implements Ice.ValueFactory
    {
        @Override
        public Ice.Object
        create(String type)
        {
            assert(type.equals(_MyInterfaceDisp.ice_staticId()));
            return new MyInterfaceI();
        }
    }

    private static class TestReadValueCallback implements Ice.ReadValueCallback
    {
        @Override
        public void valueReady(Ice.Object obj)
        {
            this.obj = obj;
        }

        Ice.Object obj;
    }

    private static class MyClassFactoryWrapper implements Ice.ValueFactory
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

        void
        setFactory(Ice.ValueFactory factory)
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

        private Ice.ValueFactory _factory;
    }

    @Override
    public int
    run(String[] args)
    {
        Ice.Communicator comm = communicator();
        MyClassFactoryWrapper factoryWrapper = new MyClassFactoryWrapper();
        comm.getValueFactoryManager().add(factoryWrapper, MyClass.ice_staticId());
        comm.getValueFactoryManager().add(new MyInterfaceFactory(), _MyInterfaceDisp.ice_staticId());

        Ice.InputStream in;
        Ice.OutputStream out;

        PrintWriter printWriter = getWriter();
        printWriter.print("testing primitive types... ");
        printWriter.flush();

        {
            byte[] data = new byte[0];
            in = new Ice.InputStream(comm, data);
        }

        {
            out = new Ice.OutputStream(comm);
            out.startEncapsulation();
            out.writeBool(true);
            out.endEncapsulation();
            byte[] data = out.finished();

            in = new Ice.InputStream(comm, data);
            in.startEncapsulation();
            test(in.readBool());
            in.endEncapsulation();
        }

        {
            byte[] data = new byte[0];
            in = new Ice.InputStream(comm, data);
            try
            {
                in.readBool();
                test(false);
            }
            catch(Ice.UnmarshalOutOfBoundsException ex)
            {
            }
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeBool(true);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readBool());
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeByte((byte)1);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readByte() == (byte)1);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeShort((short)2);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readShort() == (short)2);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeInt(3);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readInt() == 3);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeLong(4);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readLong() == 4);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeFloat((float)5.0);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readFloat() == (float)5.0);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeDouble(6.0);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readDouble() == 6.0);
        }

        {
            out = new Ice.OutputStream(comm);
            out.writeString("hello world");
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(in.readString().equals("hello world"));
        }

        printWriter.println("ok");

        printWriter.print("testing constructed types... ");
        printWriter.flush();

        {
            out = new Ice.OutputStream(comm);
            MyEnum.ice_write(out, MyEnum.enum3);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test(MyEnum.ice_read(in) == MyEnum.enum3);
        }

        {
            out = new Ice.OutputStream(comm);
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
            s.p = MyInterfacePrxHelper.uncheckedCast(comm.stringToProxy("test:default"));
            SmallStruct.ice_write(out, s);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            SmallStruct s2 = SmallStruct.ice_read(in);
            test(s2.equals(s));
        }

        {
            out = new Ice.OutputStream(comm);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = (byte)5;
            o.setSh((short)4);
            o.setI(3);
            out.writeValue(o);
            out.writePendingValues();
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            TestReadValueCallback cb = new TestReadValueCallback();
            in.readValue(cb);
            in.readPendingValues();
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
        }

        {
            out = new Ice.OutputStream(comm, Ice.Util.Encoding_1_0);
            OptionalClass o = new OptionalClass();
            o.bo = true;
            o.by = (byte)5;
            o.setSh((short)4);
            o.setI(3);
            out.writeValue(o);
            out.writePendingValues();
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, Ice.Util.Encoding_1_0, data);
            TestReadValueCallback cb = new TestReadValueCallback();
            in.readValue(cb);
            in.readPendingValues();
            OptionalClass o2 = (OptionalClass)cb.obj;
            test(o2.bo == o.bo);
            test(o2.by == o.by);
            test(!o2.hasSh());
            test(!o2.hasI());
        }

        {
            final boolean[] arr =
            {
                true,
                false,
                true,
                false
            };
            out = new Ice.OutputStream(comm);
            BoolSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            boolean[] arr2 = BoolSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final boolean[][] arrS =
            {
                arr,
                new boolean[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            BoolSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            boolean[][] arr2S = BoolSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final byte[] arr =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            out = new Ice.OutputStream(comm);
            ByteSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            byte[] arr2 = ByteSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final byte[][] arrS =
            {
                arr,
                new byte[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            ByteSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            byte[][] arr2S = ByteSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            test.Ice.stream.Serialize.Small small = new test.Ice.stream.Serialize.Small();
            small.i = 99;
            out = new Ice.OutputStream(comm);
            out.writeSerializable(small);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            test.Ice.stream.Serialize.Small small2 = (test.Ice.stream.Serialize.Small)in.readSerializable();
            test(small2.i == 99);
        }

        {
            final short[] arr =
            {
                (short)0x01,
                (short)0x11,
                (short)0x12,
                (short)0x22
            };
            out = new Ice.OutputStream(comm);
            ShortSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            short[] arr2 = ShortSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final short[][] arrS =
            {
                arr,
                new short[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            ShortSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            short[][] arr2S = ShortSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final int[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            out = new Ice.OutputStream(comm);
            IntSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            int[] arr2 = IntSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final int[][] arrS =
            {
                arr,
                new int[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            IntSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            int[][] arr2S = IntSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final long[] arr =
            {
                0x01,
                0x11,
                0x12,
                0x22
            };
            out = new Ice.OutputStream(comm);
            LongSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            long[] arr2 = LongSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final long[][] arrS =
            {
                arr,
                new long[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            LongSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            long[][] arr2S = LongSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final float[] arr =
            {
                1,
                2,
                3,
                4
            };
            out = new Ice.OutputStream(comm);
            FloatSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            float[] arr2 = FloatSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final float[][] arrS =
            {
                arr,
                new float[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            FloatSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            float[][] arr2S = FloatSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final double[] arr =
            {
                1,
                2,
                3,
                4
            };
            out = new Ice.OutputStream(comm);
            DoubleSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            double[] arr2 = DoubleSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final double[][] arrS =
            {
                arr,
                new double[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            DoubleSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            double[][] arr2S = DoubleSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final String[] arr =
            {
                "string1",
                "string2",
                "string3",
                "string4"
            };
            out = new Ice.OutputStream(comm);
            StringSeqHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            String[] arr2 = StringSeqHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final String[][] arrS =
            {
                arr,
                new String[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            StringSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            String[][] arr2S = StringSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
        }

        {
            final MyEnum[] arr =
            {
                MyEnum.enum3,
                MyEnum.enum2,
                MyEnum.enum1,
                MyEnum.enum2
            };
            out = new Ice.OutputStream(comm);
            MyEnumSHelper.write(out, arr);
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            MyEnum[] arr2 = MyEnumSHelper.read(in);
            test(java.util.Arrays.equals(arr2, arr));

            final MyEnum[][] arrS =
            {
                arr,
                new MyEnum[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            MyEnumSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            MyEnum[][] arr2S = MyEnumSSHelper.read(in);
            test(java.util.Arrays.deepEquals(arr2S, arrS));
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
            out = new Ice.OutputStream(comm);
            MyClassSHelper.write(out, arr);
            out.writePendingValues();
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            MyClass[] arr2 = MyClassSHelper.read(in);
            in.readPendingValues();
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

            final MyClass[][] arrS =
            {
                arr,
                new MyClass[0],
                arr
            };
            out = new Ice.OutputStream(comm);
            MyClassSSHelper.write(out, arrS);
            data = out.finished();
            in = new Ice.InputStream(comm, data);
            MyClass[][] arr2S = MyClassSSHelper.read(in);
            test(arr2S.length == arrS.length);
            test(arr2S[0].length == arrS[0].length);
            test(arr2S[1].length == arrS[1].length);
            test(arr2S[2].length == arrS[2].length);
        }

        {
            MyInterface i = new MyInterfaceI();
            out = new Ice.OutputStream(comm);
            out.writeValue(i);
            out.writePendingValues();
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            MyInterfaceHolder j = new MyInterfaceHolder();
            in.readValue(j);
            in.readPendingValues();
            test(j.value != null);
        }

        {
            out = new Ice.OutputStream(comm);
            MyClass obj = new MyClass();
            obj.s = new SmallStruct();
            obj.s.e = MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            out.writeValue(writer);
            out.writePendingValues();
            out.finished();
            test(writer.called);
        }

        {
            out = new Ice.OutputStream(comm);
            MyClass obj = new MyClass();
            obj.s = new SmallStruct();
            obj.s.e = MyEnum.enum2;
            TestObjectWriter writer = new TestObjectWriter(obj);
            out.writeValue(writer);
            out.writePendingValues();
            byte[] data = out.finished();
            test(writer.called);
            factoryWrapper.setFactory(new TestValueFactory());
            in = new Ice.InputStream(comm, data);
            TestReadValueCallback cb = new TestReadValueCallback();
            in.readValue(cb);
            in.readPendingValues();
            test(cb.obj != null);
            test(cb.obj instanceof TestObjectReader);
            TestObjectReader reader = (TestObjectReader)cb.obj;
            test(reader.called);
            test(reader.obj != null);
            test(reader.obj.s.e == MyEnum.enum2);
            factoryWrapper.setFactory(null);
        }

        {
            out = new Ice.OutputStream(comm);
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

            in = new Ice.InputStream(comm, data);
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
            out = new Ice.OutputStream(comm);
            ByteBoolDHelper.write(out, dict);
            byte data[] = out.finished();
            in = new Ice.InputStream(comm, data);
            java.util.Map<Byte, Boolean> dict2 = ByteBoolDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<Short, Integer> dict = new java.util.HashMap<Short, Integer>();
            dict.put((short)1, 9);
            dict.put((short)4, 8);
            out = new Ice.OutputStream(comm);
            ShortIntDHelper.write(out, dict);
            byte data[] = out.finished();
            in = new Ice.InputStream(comm, data);
            java.util.Map<Short, Integer> dict2 = ShortIntDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<Long, Float> dict = new java.util.HashMap<Long, Float>();
            dict.put((long)123809828, 0.51f);
            dict.put((long)123809829, 0.56f);
            out = new Ice.OutputStream(comm);
            LongFloatDHelper.write(out, dict);
            byte data[] = out.finished();
            in = new Ice.InputStream(comm, data);
            java.util.Map<Long, Float> dict2 = LongFloatDHelper.read(in);
            test(dict2.equals(dict));
        }

        {
            java.util.Map<String, String> dict = new java.util.HashMap<String, String>();
            dict.put("key1", "value1");
            dict.put("key2", "value2");
            out = new Ice.OutputStream(comm);
            StringStringDHelper.write(out, dict);
            byte data[] = out.finished();
            in = new Ice.InputStream(comm, data);
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
            out = new Ice.OutputStream(comm);
            StringMyClassDHelper.write(out, dict);
            out.writePendingValues();
            byte[] data = out.finished();
            in = new Ice.InputStream(comm, data);
            java.util.Map<String, MyClass> dict2 = StringMyClassDHelper.read(in);
            in.readPendingValues();
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
        Ice.InitializationData initData = super.getInitData(argsH);
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
