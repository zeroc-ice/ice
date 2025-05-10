// Copyright (c) ZeroC, Inc.

package test.Ice.stream;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.CompositeSliceLoader;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InputStream;
import com.zeroc.Ice.MarshalException;
import com.zeroc.Ice.ModuleToPackageSliceLoader;
import com.zeroc.Ice.OutputStream;
import com.zeroc.Ice.SliceLoader;
import com.zeroc.Ice.UserException;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.Value;

import test.Ice.stream.Serialize.Small;
import test.Ice.stream.Test.BoolSSHelper;
import test.Ice.stream.Test.ByteBoolDHelper;
import test.Ice.stream.Test.ByteSSHelper;
import test.Ice.stream.Test.DoubleSSHelper;
import test.Ice.stream.Test.FloatSSHelper;
import test.Ice.stream.Test.IntSSHelper;
import test.Ice.stream.Test.LargeStruct;
import test.Ice.stream.Test.LongFloatDHelper;
import test.Ice.stream.Test.LongSSHelper;
import test.Ice.stream.Test.MyClass;
import test.Ice.stream.Test.MyClassSHelper;
import test.Ice.stream.Test.MyClassSSHelper;
import test.Ice.stream.Test.MyEnum;
import test.Ice.stream.Test.MyEnumSHelper;
import test.Ice.stream.Test.MyEnumSSHelper;
import test.Ice.stream.Test.MyException;
import test.Ice.stream.Test.MyInterfacePrx;
import test.Ice.stream.Test.OptionalClass;
import test.Ice.stream.Test.ShortIntDHelper;
import test.Ice.stream.Test.ShortSSHelper;
import test.Ice.stream.Test.StringMyClassDHelper;
import test.Ice.stream.Test.StringSSHelper;
import test.Ice.stream.Test.StringStringDHelper;
import test.TestHelper;

import java.io.PrintWriter;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class Client extends TestHelper {
    private static class TestObjectWriter extends Value {
        TestObjectWriter(MyClass obj) {
            this.obj = obj;
        }

        @Override
        public void _iceRead(InputStream in) {
            assert false;
        }

        @Override
        public void _iceWrite(OutputStream out) {
            obj._iceWrite(out);
            called = true;
        }

        MyClass obj;
        boolean called;
    }

    private static class TestObjectReader extends Value {
        @Override
        public void _iceRead(InputStream in) {
            obj = new MyClass();
            obj._iceRead(in);
            called = true;
        }

        @Override
        public void _iceWrite(OutputStream out) {
            assert false;
        }

        MyClass obj;
        boolean called;
    }

    private static class CustomSliceLoader implements SliceLoader {
        private boolean _useReader;

        public void useReader(boolean value) {
            _useReader = value;
        }

        @Override
        public Object newInstance(String typeId) {
            if (typeId.equals(MyClass.ice_staticId())) {
                if (_useReader) {
                    return new TestObjectReader();
                } else {
                    return new MyClass();
                }
            }
            return null;
        }
    }

    private static class Wrapper<T extends Value> {
        T obj;
    }

    public void run(String[] args) {
        var customSliceLoader = new CustomSliceLoader();
        var initData = new InitializationData();
        initData.properties = createTestProperties(args);
        initData.sliceLoader = new CompositeSliceLoader(
            customSliceLoader,
            new ModuleToPackageSliceLoader("::Test", "test.Ice.stream.Test"));

        try (Communicator communicator = initialize(initData)) {
            InputStream in;
            OutputStream out;

            PrintWriter printWriter = getWriter();
            printWriter.print("testing primitive types... ");
            printWriter.flush();

            {
                byte[] data = new byte[0];
                in = new InputStream(communicator, data);
            }

            {
                out = new OutputStream(communicator);
                out.startEncapsulation();
                out.writeBool(true);
                out.endEncapsulation();
                byte[] data = out.finished();

                in = new InputStream(communicator, data);
                in.startEncapsulation();
                test(in.readBool());
                in.endEncapsulation();
            }

            {
                byte[] data = new byte[0];
                in = new InputStream(communicator, data);
                try {
                    in.readBool();
                    test(false);
                } catch (MarshalException ex) {}
            }

            {
                out = new OutputStream(communicator);
                out.writeBool(true);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readBool());
            }

            {
                out = new OutputStream(communicator);
                out.writeByte((byte) 1);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readByte() == (byte) 1);
            }

            {
                out = new OutputStream(communicator);
                out.writeShort((short) 2);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readShort() == (short) 2);
            }

            {
                out = new OutputStream(communicator);
                out.writeInt(3);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readInt() == 3);
            }

            {
                out = new OutputStream(communicator);
                out.writeLong(4);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readLong() == 4);
            }

            {
                out = new OutputStream(communicator);
                out.writeFloat((float) 5.0);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readFloat() == (float) 5.0);
            }

            {
                out = new OutputStream(communicator);
                out.writeDouble(6.0);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(in.readDouble() == 6.0);
            }

            {
                out = new OutputStream(communicator);
                out.writeString("hello world");
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test("hello world".equals(in.readString()));
            }

            printWriter.println("ok");

            printWriter.print("testing constructed types... ");
            printWriter.flush();

            {
                out = new OutputStream(communicator);
                MyEnum.ice_write(out, MyEnum.enum3);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                test(MyEnum.ice_read(in) == MyEnum.enum3);
            }

            {
                out = new OutputStream(communicator);
                LargeStruct s = new LargeStruct();
                s.bo = true;
                s.by = (byte) 1;
                s.sh = (short) 2;
                s.i = 3;
                s.l = 4;
                s.f = (float) 5.0;
                s.d = 6.0;
                s.str = "7";
                s.e = MyEnum.enum2;
                s.p = MyInterfacePrx.createProxy(communicator, "test:default");
                LargeStruct.ice_write(out, s);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                LargeStruct s2 = LargeStruct.ice_read(in);
                test(s2.equals(s));
            }

            {
                out = new OutputStream(communicator);
                OptionalClass o = new OptionalClass();
                o.bo = true;
                o.by = (byte) 5;
                o.setSh((short) 4);
                o.setI(3);
                out.writeValue(o);
                out.writePendingValues();
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                final Wrapper<OptionalClass> cb = new Wrapper<>();
                in.readValue(value -> cb.obj = value, OptionalClass.class);
                in.readPendingValues();
                OptionalClass o2 = cb.obj;
                test(o2.bo == o.bo);
                test(o2.by == o.by);
                if ("1.0"
                    .equals(communicator
                        .getProperties()
                        .getIceProperty("Ice.Default.EncodingVersion"))) {
                    test(!o2.hasSh());
                    test(!o2.hasI());
                } else {
                    test(o2.getSh() == o.getSh());
                    test(o2.getI() == o.getI());
                }
            }

            {
                out = new OutputStream(Util.Encoding_1_0, false);
                OptionalClass o = new OptionalClass();
                o.bo = true;
                o.by = (byte) 5;
                o.setSh((short) 4);
                o.setI(3);
                out.writeValue(o);
                out.writePendingValues();
                byte[] data = out.finished();
                in = new InputStream(communicator, Util.Encoding_1_0, data);
                final Wrapper<OptionalClass> cb = new Wrapper<>();
                in.readValue(value -> cb.obj = value, OptionalClass.class);
                in.readPendingValues();
                OptionalClass o2 = cb.obj;
                test(o2.bo == o.bo);
                test(o2.by == o.by);
                test(!o2.hasSh());
                test(!o2.hasI());
            }

            {
                final boolean[] arr = {true, false, true, false};
                out = new OutputStream(communicator);
                out.writeBoolSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                boolean[] arr2 = in.readBoolSeq();
                test(Arrays.equals(arr2, arr));

                final boolean[][] arrS = {arr, new boolean[0], arr};
                out = new OutputStream(communicator);
                BoolSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                boolean[][] arr2S = BoolSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final byte[] arr = {(byte) 0x01, (byte) 0x11, (byte) 0x12, (byte) 0x22};
                out = new OutputStream(communicator);
                out.writeByteSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                byte[] arr2 = in.readByteSeq();
                test(Arrays.equals(arr2, arr));

                final byte[][] arrS = {arr, new byte[0], arr};
                out = new OutputStream(communicator);
                ByteSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                byte[][] arr2S = ByteSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                Small small = new Small();
                small.i = 99;
                out = new OutputStream(communicator);
                out.writeSerializable(small);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Small small2 =
                    in.readSerializable(Small.class);
                test(small2.i == 99);
            }

            {
                final short[] arr = {(short) 0x01, (short) 0x11, (short) 0x12, (short) 0x22};
                out = new OutputStream(communicator);
                out.writeShortSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                short[] arr2 = in.readShortSeq();
                test(Arrays.equals(arr2, arr));

                final short[][] arrS = {arr, new short[0], arr};
                out = new OutputStream(communicator);
                ShortSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                short[][] arr2S = ShortSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final int[] arr = {0x01, 0x11, 0x12, 0x22};
                out = new OutputStream(communicator);
                out.writeIntSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                int[] arr2 = in.readIntSeq();
                test(Arrays.equals(arr2, arr));

                final int[][] arrS = {arr, new int[0], arr};
                out = new OutputStream(communicator);
                IntSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                int[][] arr2S = IntSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final long[] arr = {0x01, 0x11, 0x12, 0x22};
                out = new OutputStream(communicator);
                out.writeLongSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                long[] arr2 = in.readLongSeq();
                test(Arrays.equals(arr2, arr));

                final long[][] arrS = {arr, new long[0], arr};
                out = new OutputStream(communicator);
                LongSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                long[][] arr2S = LongSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final float[] arr = {1, 2, 3, 4};
                out = new OutputStream(communicator);
                out.writeFloatSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                float[] arr2 = in.readFloatSeq();
                test(Arrays.equals(arr2, arr));

                final float[][] arrS = {arr, new float[0], arr};
                out = new OutputStream(communicator);
                FloatSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                float[][] arr2S = FloatSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final double[] arr = {1, 2, 3, 4};
                out = new OutputStream(communicator);
                out.writeDoubleSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                double[] arr2 = in.readDoubleSeq();
                test(Arrays.equals(arr2, arr));

                final double[][] arrS = {arr, new double[0], arr};
                out = new OutputStream(communicator);
                DoubleSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                double[][] arr2S = DoubleSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final String[] arr = {"string1", "string2", "string3", "string4"};
                out = new OutputStream(communicator);
                out.writeStringSeq(arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                String[] arr2 = in.readStringSeq();
                test(Arrays.equals(arr2, arr));

                final String[][] arrS = {arr, new String[0], arr};
                out = new OutputStream(communicator);
                StringSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                String[][] arr2S = StringSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                final MyEnum[] arr = {MyEnum.enum3, MyEnum.enum2, MyEnum.enum1, MyEnum.enum2};
                out = new OutputStream(communicator);
                MyEnumSHelper.write(out, arr);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                MyEnum[] arr2 = MyEnumSHelper.read(in);
                test(Arrays.equals(arr2, arr));

                final MyEnum[][] arrS = {arr, new MyEnum[0], arr};
                out = new OutputStream(communicator);
                MyEnumSSHelper.write(out, arrS);
                data = out.finished();
                in = new InputStream(communicator, data);
                MyEnum[][] arr2S = MyEnumSSHelper.read(in);
                test(Arrays.deepEquals(arr2S, arrS));
            }

            {
                MyClass[] arr = new MyClass[4];
                for (int i = 0; i < arr.length; i++) {
                    arr[i] = new MyClass();
                    arr[i].c = arr[i];
                    arr[i].o = arr[i];
                    arr[i].s = new LargeStruct();
                    arr[i].s.e = MyEnum.enum2;
                    arr[i].seq1 = new boolean[]{true, false, true, false};
                    arr[i].seq2 = new byte[]{(byte) 1, (byte) 2, (byte) 3, (byte) 4};
                    arr[i].seq3 = new short[]{(short) 1, (short) 2, (short) 3, (short) 4};
                    arr[i].seq4 = new int[]{1, 2, 3, 4};
                    arr[i].seq5 = new long[]{1, 2, 3, 4};
                    arr[i].seq6 = new float[]{1, 2, 3, 4};
                    arr[i].seq7 = new double[]{1, 2, 3, 4};
                    arr[i].seq8 = new String[]{"string1", "string2", "string3", "string4"};
                    arr[i].seq9 = new MyEnum[]{MyEnum.enum3, MyEnum.enum2, MyEnum.enum1};
                    arr[i].seq10 = new MyClass[4]; // null elements.
                    arr[i].d = new HashMap<>();
                    arr[i].d.put("hi", arr[i]);
                }
                out = new OutputStream(communicator);
                MyClassSHelper.write(out, arr);
                out.writePendingValues();
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                MyClass[] arr2 = MyClassSHelper.read(in);
                in.readPendingValues();
                test(arr2.length == arr.length);
                for (int i = 0; i < arr2.length; i++) {
                    test(arr2[i] != null);
                    test(arr2[i].c == arr2[i]);
                    test(arr2[i].o == arr2[i]);
                    test(arr2[i].s.e == MyEnum.enum2);
                    test(Arrays.equals(arr2[i].seq1, arr[i].seq1));
                    test(Arrays.equals(arr2[i].seq2, arr[i].seq2));
                    test(Arrays.equals(arr2[i].seq3, arr[i].seq3));
                    test(Arrays.equals(arr2[i].seq4, arr[i].seq4));
                    test(Arrays.equals(arr2[i].seq5, arr[i].seq5));
                    test(Arrays.equals(arr2[i].seq6, arr[i].seq6));
                    test(Arrays.equals(arr2[i].seq7, arr[i].seq7));
                    test(Arrays.equals(arr2[i].seq8, arr[i].seq8));
                    test(Arrays.equals(arr2[i].seq9, arr[i].seq9));
                    test(arr2[i].d.get("hi") == arr2[i]);
                }

                final MyClass[][] arrS = {arr, new MyClass[0], arr};
                out = new OutputStream(communicator);
                MyClassSSHelper.write(out, arrS);
                out.writePendingValues();
                data = out.finished();
                in = new InputStream(communicator, data);
                MyClass[][] arr2S = MyClassSSHelper.read(in);
                in.readPendingValues();
                test(arr2S.length == arrS.length);
                test(arr2S[0].length == arrS[0].length);
                test(arr2S[1].length == arrS[1].length);
                test(arr2S[2].length == arrS[2].length);

                for (int j = 0; j < arr2S.length; j++) {
                    for (int k = 0; k < arr2S[j].length; k++) {
                        test(arr2S[j][k].c == arr2S[j][k]);
                        test(arr2S[j][k].o == arr2S[j][k]);
                        test(arr2S[j][k].s.e == MyEnum.enum2);
                        test(Arrays.equals(arr2S[j][k].seq1, arr[k].seq1));
                        test(Arrays.equals(arr2S[j][k].seq2, arr[k].seq2));
                        test(Arrays.equals(arr2S[j][k].seq3, arr[k].seq3));
                        test(Arrays.equals(arr2S[j][k].seq4, arr[k].seq4));
                        test(Arrays.equals(arr2S[j][k].seq5, arr[k].seq5));
                        test(Arrays.equals(arr2S[j][k].seq6, arr[k].seq6));
                        test(Arrays.equals(arr2S[j][k].seq7, arr[k].seq7));
                        test(Arrays.equals(arr2S[j][k].seq8, arr[k].seq8));
                        test(Arrays.equals(arr2S[j][k].seq9, arr[k].seq9));
                        test(arr2S[j][k].d.get("hi") == arr2S[j][k]);
                    }
                }
            }

            {
                out = new OutputStream(communicator);
                MyClass obj = new MyClass();
                obj.s = new LargeStruct();
                obj.s.e = MyEnum.enum2;
                TestObjectWriter writer = new TestObjectWriter(obj);
                out.writeValue(writer);
                out.writePendingValues();
                out.finished();
                test(writer.called);
            }

            {
                out = new OutputStream(communicator);
                MyClass obj = new MyClass();
                obj.s = new LargeStruct();
                obj.s.e = MyEnum.enum2;
                TestObjectWriter writer = new TestObjectWriter(obj);
                out.writeValue(writer);
                out.writePendingValues();
                byte[] data = out.finished();
                test(writer.called);
                customSliceLoader.useReader(true);
                in = new InputStream(communicator, data);
                final Wrapper<TestObjectReader> cb = new Wrapper<>();
                in.readValue(value -> cb.obj = value, TestObjectReader.class);
                in.readPendingValues();
                test(cb.obj != null);
                TestObjectReader reader = cb.obj;
                test(reader.called);
                test(reader.obj != null);
                test(reader.obj.s.e == MyEnum.enum2);
                customSliceLoader.useReader(false);
            }

            {
                out = new OutputStream(communicator);
                final MyException ex = new MyException();

                MyClass c = new MyClass();
                c.c = c;
                c.o = c;
                c.s = new LargeStruct();
                c.s.e = MyEnum.enum2;
                c.seq1 = new boolean[]{true, false, true, false};
                c.seq2 = new byte[]{(byte) 1, (byte) 2, (byte) 3, (byte) 4};
                c.seq3 = new short[]{(short) 1, (short) 2, (short) 3, (short) 4};
                c.seq4 = new int[]{1, 2, 3, 4};
                c.seq5 = new long[]{1, 2, 3, 4};
                c.seq6 = new float[]{1, 2, 3, 4};
                c.seq7 = new double[]{1, 2, 3, 4};
                c.seq8 = new String[]{"string1", "string2", "string3", "string4"};
                c.seq9 = new MyEnum[]{MyEnum.enum3, MyEnum.enum2, MyEnum.enum1};
                c.seq10 = new MyClass[4]; // null elements.
                c.d = new HashMap<>();
                c.d.put("hi", c);

                ex.c = c;

                out.writeException(ex);
                byte[] data = out.finished();

                in = new InputStream(communicator, data);
                try {
                    in.throwException();
                    test(false);
                } catch (MyException ex1) {
                    test(ex1.c.s.e == c.s.e);
                    test(Arrays.equals(ex1.c.seq1, c.seq1));
                    test(Arrays.equals(ex1.c.seq2, c.seq2));
                    test(Arrays.equals(ex1.c.seq3, c.seq3));
                    test(Arrays.equals(ex1.c.seq4, c.seq4));
                    test(Arrays.equals(ex1.c.seq5, c.seq5));
                    test(Arrays.equals(ex1.c.seq6, c.seq6));
                    test(Arrays.equals(ex1.c.seq7, c.seq7));
                    test(Arrays.equals(ex1.c.seq8, c.seq8));
                    test(Arrays.equals(ex1.c.seq9, c.seq9));
                } catch (UserException ex1) {
                    test(false);
                }
            }

            {
                Map<Byte, Boolean> dict = new HashMap<>();
                dict.put((byte) 4, true);
                dict.put((byte) 1, false);
                out = new OutputStream(communicator);
                ByteBoolDHelper.write(out, dict);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Map<Byte, Boolean> dict2 = ByteBoolDHelper.read(in);
                test(dict2.equals(dict));
            }

            {
                Map<Short, Integer> dict = new HashMap<>();
                dict.put((short) 1, 9);
                dict.put((short) 4, 8);
                out = new OutputStream(communicator);
                ShortIntDHelper.write(out, dict);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Map<Short, Integer> dict2 = ShortIntDHelper.read(in);
                test(dict2.equals(dict));
            }

            {
                Map<Long, Float> dict = new HashMap<>();
                dict.put((long) 123809828, 0.51f);
                dict.put((long) 123809829, 0.56f);
                out = new OutputStream(communicator);
                LongFloatDHelper.write(out, dict);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Map<Long, Float> dict2 = LongFloatDHelper.read(in);
                test(dict2.equals(dict));
            }

            {
                Map<String, String> dict = new HashMap<>();
                dict.put("key1", "value1");
                dict.put("key2", "value2");
                out = new OutputStream(communicator);
                StringStringDHelper.write(out, dict);
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Map<String, String> dict2 = StringStringDHelper.read(in);
                test(dict2.equals(dict));
            }

            {
                Map<String, MyClass> dict = new HashMap<>();
                MyClass c;
                c = new MyClass();
                c.s = new LargeStruct();
                c.s.e = MyEnum.enum2;
                dict.put("key1", c);
                c = new MyClass();
                c.s = new LargeStruct();
                c.s.e = MyEnum.enum3;
                dict.put("key2", c);
                out = new OutputStream(communicator);
                StringMyClassDHelper.write(out, dict);
                out.writePendingValues();
                byte[] data = out.finished();
                in = new InputStream(communicator, data);
                Map<String, MyClass> dict2 = StringMyClassDHelper.read(in);
                in.readPendingValues();
                test(dict2.size() == dict.size());
                test(dict2.get("key1").s.e == MyEnum.enum2);
                test(dict2.get("key2").s.e == MyEnum.enum3);
            }

            printWriter.println("ok");
        }
    }
}
