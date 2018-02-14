// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.util.*;

import com.zeroc.Ice.Current;

import test.Ice.operations.Test.*;

public final class MyDerivedClassI implements MyDerivedClass
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    //
    // Override the Object "pseudo" operations to verify the operation mode.
    //

    @Override
    public boolean ice_isA(String id, Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_isA(id, current);
    }

    @Override
    public void ice_ping(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        MyDerivedClass.super.ice_ping(current);
    }

    @Override
    public String[] ice_ids(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_ids(current);
    }

    @Override
    public String ice_id(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
        return MyDerivedClass.super.ice_id(current);
    }

    @Override
    public void shutdown(Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public void opVoid(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Normal);
    }

    @Override
    public MyClass.OpBoolResult opBool(boolean p1, boolean p2, Current current)
    {
        return new MyClass.OpBoolResult(p2, p1);
    }

    @Override
    public MyClass.OpBoolSResult opBoolS(boolean[] p1, boolean[] p2, Current current)
    {
        MyClass.OpBoolSResult r = new MyClass.OpBoolSResult();
        r.p3 = new boolean[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public MyClass.OpBoolSSResult opBoolSS(boolean[][] p1, boolean[][] p2, Current current)
    {
        MyClass.OpBoolSSResult r = new MyClass.OpBoolSSResult();
        r.p3 = new boolean[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new boolean[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public MyClass.OpByteResult opByte(byte p1, byte p2, Current current)
    {
        return new MyClass.OpByteResult(p1, (byte) (p1 ^ p2));
    }

    @Override
    public MyClass.OpByteBoolDResult opByteBoolD(java.util.Map<Byte, Boolean> p1, java.util.Map<Byte, Boolean> p2,
                                                 Current current)
    {
        MyClass.OpByteBoolDResult r = new MyClass.OpByteBoolDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpByteSResult opByteS(byte[] p1, byte[] p2, Current current)
    {
        MyClass.OpByteSResult r = new MyClass.OpByteSResult();
        r.p3 = new byte[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return r;
    }

    @Override
    public MyClass.OpByteSSResult opByteSS(byte[][] p1, byte[][] p2, Current current)
    {
        MyClass.OpByteSSResult r = new MyClass.OpByteSSResult();
        r.p3 = new byte[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r.p3[i] = p1[p1.length - (i + 1)];
        }

        r.returnValue = new byte[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.returnValue, 0, p1.length);
        System.arraycopy(p2, 0, r.returnValue, p1.length, p2.length);
        return r;
    }

    @Override
    public MyClass.OpFloatDoubleResult opFloatDouble(float p1, double p2, Current current)
    {
        return new MyClass.OpFloatDoubleResult(p2, p1, p2);
    }

    @Override
    public MyClass.OpFloatDoubleSResult opFloatDoubleS(float[] p1, double[] p2, Current current)
    {
        MyClass.OpFloatDoubleSResult r = new MyClass.OpFloatDoubleSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length + p1.length];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[p2.length + i] = p1[i];
        }
        return r;
    }

    @Override
    public MyClass.OpFloatDoubleSSResult opFloatDoubleSS(float[][] p1, double[][] p2, Current current)
    {
        MyClass.OpFloatDoubleSSResult r = new MyClass.OpFloatDoubleSSResult();
        r.p3 = p1;
        r.p4 = new double[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.p4[i] = p2[p2.length - (i + 1)];
        }
        r.returnValue = new double[p2.length * 2][];
        System.arraycopy(p2, 0, r.returnValue, 0, p2.length);
        System.arraycopy(p2, 0, r.returnValue, p2.length, p2.length);
        return r;
    }

    @Override
    public MyClass.OpLongFloatDResult opLongFloatD(java.util.Map<Long, Float> p1, java.util.Map<Long, Float> p2,
                                                   Current current)
    {
        MyClass.OpLongFloatDResult r = new MyClass.OpLongFloatDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpMyClassResult opMyClass(MyClassPrx p1, Current current)
    {
        MyClass.OpMyClassResult r = new MyClass.OpMyClassResult();
        r.p2 = p1;
        r.p3 = MyClassPrx.uncheckedCast(
                current.adapter.createProxy(com.zeroc.Ice.Util.stringToIdentity("noSuchIdentity")));
        r.returnValue = MyClassPrx.uncheckedCast(current.adapter.createProxy(current.id));
        return r;
    }

    @Override
    public MyClass.OpMyEnumResult opMyEnum(MyEnum p1, Current current)
    {
        return new MyClass.OpMyEnumResult(MyEnum.enum3, p1);
    }

    @Override
    public MyClass.OpShortIntDResult opShortIntD(java.util.Map<Short, Integer> p1, java.util.Map<Short, Integer> p2,
                                                 Current current)
    {
        MyClass.OpShortIntDResult r = new MyClass.OpShortIntDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpShortIntLongResult opShortIntLong(short p1, int p2, long p3, Current current)
    {
        return new MyClass.OpShortIntLongResult(p3, p1, p2, p3);
    }

    @Override
    public MyClass.OpShortIntLongSResult opShortIntLongS(short[] p1, int[] p2, long[] p3, Current current)
    {
        MyClass.OpShortIntLongSResult r = new MyClass.OpShortIntLongSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return r;
    }

    @Override
    public MyClass.OpShortIntLongSSResult opShortIntLongSS(short[][] p1, int[][] p2, long[][] p3, Current current)
    {
        MyClass.OpShortIntLongSSResult r = new MyClass.OpShortIntLongSSResult();
        r.p4 = p1;
        r.p5 = new int[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.p5[i] = p2[p2.length - (i + 1)];
        }
        r.p6 = new long[p3.length * 2][];
        System.arraycopy(p3, 0, r.p6, 0, p3.length);
        System.arraycopy(p3, 0, r.p6, p3.length, p3.length);
        r.returnValue = p3;
        return r;
    }

    @Override
    public MyClass.OpStringResult opString(String p1, String p2, Current current)
    {
        MyClass.OpStringResult r = new MyClass.OpStringResult();
        r.p3 = p2 + " " + p1;
        r.returnValue = p1 + " " + p2;
        return r;
    }

    @Override
    public MyClass.OpStringMyEnumDResult opStringMyEnumD(java.util.Map<String, MyEnum> p1,
                                                         java.util.Map<String, MyEnum> p2,
                                                         Current current)
    {
        MyClass.OpStringMyEnumDResult r = new MyClass.OpStringMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpMyEnumStringDResult opMyEnumStringD(java.util.Map<MyEnum, String> p1,
                                                         java.util.Map<MyEnum, String> p2,
                                                         Current current)
    {
        MyClass.OpMyEnumStringDResult r = new MyClass.OpMyEnumStringDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpMyStructMyEnumDResult opMyStructMyEnumD(java.util.Map<MyStruct, MyEnum> p1,
                                                             java.util.Map<MyStruct, MyEnum> p2,
                                                             Current current)
    {
        MyClass.OpMyStructMyEnumDResult r = new MyClass.OpMyStructMyEnumDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpByteBoolDSResult opByteBoolDS(List<Map<Byte, Boolean>> p1, List<Map<Byte, Boolean>> p2,
                                                   Current current)
    {
        MyClass.OpByteBoolDSResult r = new MyClass.OpByteBoolDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpShortIntDSResult opShortIntDS(List<Map<Short, Integer>> p1, List<Map<Short, Integer>> p2,
                                                   Current current)
    {
        MyClass.OpShortIntDSResult r = new MyClass.OpShortIntDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpLongFloatDSResult opLongFloatDS(List<Map<Long, Float>> p1, List<Map<Long, Float>> p2,
                                                     Current current)
    {
        MyClass.OpLongFloatDSResult r = new MyClass.OpLongFloatDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpStringStringDSResult opStringStringDS(List<Map<String, String>> p1, List<Map<String, String>> p2,
                                                           Current current)
    {
        MyClass.OpStringStringDSResult r =  new MyClass.OpStringStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpStringMyEnumDSResult opStringMyEnumDS(List<Map<String, MyEnum>> p1, List<Map<String, MyEnum>> p2,
                                                           Current current)
    {
        MyClass.OpStringMyEnumDSResult r = new MyClass.OpStringMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpMyEnumStringDSResult opMyEnumStringDS(List<Map<MyEnum, String>> p1, List<Map<MyEnum, String>> p2,
                                                           Current current)
    {
        MyClass.OpMyEnumStringDSResult r = new MyClass.OpMyEnumStringDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpMyStructMyEnumDSResult opMyStructMyEnumDS(List<Map<MyStruct, MyEnum>> p1,
                                                               List<Map<MyStruct, MyEnum>> p2,
                                                               Current current)
    {
        MyClass.OpMyStructMyEnumDSResult r = new MyClass.OpMyStructMyEnumDSResult();
        r.p3 = new ArrayList<>();
        r.p3.addAll(p2);
        r.p3.addAll(p1);

        r.returnValue = new ArrayList<>(p1);
        Collections.reverse(r.returnValue);

        return r;
    }

    @Override
    public MyClass.OpByteByteSDResult opByteByteSD(Map<Byte, byte[]> p1, Map<Byte, byte[]> p2, Current current)
    {
        MyClass.OpByteByteSDResult r = new MyClass.OpByteByteSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpBoolBoolSDResult opBoolBoolSD(Map<Boolean, boolean[]> p1, Map<Boolean, boolean[]> p2,
                                                   Current current)
    {
        MyClass.OpBoolBoolSDResult r = new MyClass.OpBoolBoolSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpShortShortSDResult opShortShortSD(Map<Short, short[]> p1, Map<Short, short[]> p2, Current current)
    {
        MyClass.OpShortShortSDResult r = new MyClass.OpShortShortSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpIntIntSDResult opIntIntSD(Map<Integer, int[]> p1, Map<Integer, int[]> p2, Current current)
    {
        MyClass.OpIntIntSDResult r = new MyClass.OpIntIntSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpLongLongSDResult opLongLongSD(Map<Long, long[]> p1, Map<Long, long[]> p2, Current current)
    {
        MyClass.OpLongLongSDResult r = new MyClass.OpLongLongSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpStringFloatSDResult opStringFloatSD(Map<String, float[]> p1, Map<String, float[]> p2,
                                                         Current current)
    {
        MyClass.OpStringFloatSDResult r = new MyClass.OpStringFloatSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpStringDoubleSDResult opStringDoubleSD(Map<String, double[]> p1, Map<String, double[]> p2,
                                                           Current current)
    {
        MyClass.OpStringDoubleSDResult r = new MyClass.OpStringDoubleSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpStringStringSDResult opStringStringSD(Map<String, String[]> p1, Map<String, String[]> p2,
                                                           Current current)
    {
        MyClass.OpStringStringSDResult r = new MyClass.OpStringStringSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpMyEnumMyEnumSDResult opMyEnumMyEnumSD(Map<MyEnum, MyEnum[]> p1, Map<MyEnum, MyEnum[]> p2,
                                                           Current current)
    {
        MyClass.OpMyEnumMyEnumSDResult r = new MyClass.OpMyEnumMyEnumSDResult();
        r.p3 = p2;
        r.returnValue = new HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public int[] opIntS(int[] s, Current current)
    {
        int[] r = new int[s.length];
        for(int i = 0; i < r.length; ++i)
        {
            r[i] = -s[i];
        }
        return r;
    }

    @Override
    public synchronized void opByteSOneway(byte[] s, Current current)
    {
        ++_opByteSOnewayCallCount;
    }

    @Override
    public synchronized int opByteSOnewayCallCount(Current current)
    {
        int count = _opByteSOnewayCallCount;
        _opByteSOnewayCallCount = 0;
        return count;
    }

    @Override
    public java.util.Map<String, String> opContext(Current current)
    {
        return current.ctx;
    }

    @Override
    public void opDoubleMarshaling(double p1, double[] p2, Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.length; ++i)
        {
            test(p2[i] == d);
        }
    }

    @Override
    public MyClass.OpStringSResult opStringS(String[] p1, String[] p2, Current current)
    {
        MyClass.OpStringSResult r = new MyClass.OpStringSResult();
        r.p3 = new String[p1.length + p2.length];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r.returnValue[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public MyClass.OpStringSSResult opStringSS(String[][] p1, String[][] p2, Current current)
    {
        MyClass.OpStringSSResult r = new MyClass.OpStringSSResult();
        r.p3 = new String[p1.length + p2.length][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return r;
    }

    @Override
    public MyClass.OpStringSSSResult opStringSSS(String[][][] p1, String[][][] p2, Current current)
    {
        MyClass.OpStringSSSResult r = new MyClass.OpStringSSSResult();
        r.p3 = new String[p1.length + p2.length][][];
        System.arraycopy(p1, 0, r.p3, 0, p1.length);
        System.arraycopy(p2, 0, r.p3, p1.length, p2.length);

        r.returnValue = new String[p2.length][][];
        for(int i = 0; i < p2.length; i++)
        {
            r.returnValue[i] = p2[p2.length - (i + 1)];
        }
        return r;
    }

    @Override
    public MyClass.OpStringStringDResult opStringStringD(java.util.Map<String, String> p1,
                                                         java.util.Map<String, String> p2,
                                                         Current current)
    {
        MyClass.OpStringStringDResult r = new MyClass.OpStringStringDResult();
        r.p3 = p1;
        r.returnValue = new java.util.HashMap<>();
        r.returnValue.putAll(p1);
        r.returnValue.putAll(p2);
        return r;
    }

    @Override
    public MyClass.OpStructResult opStruct(Structure p1, Structure p2, Current current)
    {
        MyClass.OpStructResult r = new MyClass.OpStructResult();
        r.p3 = p1;
        r.p3.s.s = "a new string";
        r.returnValue = p2;
        return r;
    }

    @Override
    public void opIdempotent(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Idempotent);
    }

    @Override
    public void opNonmutating(Current current)
    {
        test(current.mode == com.zeroc.Ice.OperationMode.Nonmutating);
    }

    @Override
    public void opDerived(Current current)
    {
    }

    @Override
    public byte opByte1(byte value, Current current)
    {
        return value;
    }

    @Override
    public short opShort1(short value, Current current)
    {
        return value;
    }

    @Override
    public int opInt1(int value, Current current)
    {
        return value;
    }

    @Override
    public long opLong1(long value, Current current)
    {
        return value;
    }

    @Override
    public float opFloat1(float value, Current current)
    {
        return value;
    }

    @Override
    public double opDouble1(double value, Current current)
    {
        return value;
    }

    @Override
    public String opString1(String value, Current current)
    {
        return value;
    }

    @Override
    public String[] opStringS1(String[] value, Current current)
    {
        return value;
    }

    @Override
    public Map<Byte, Boolean> opByteBoolD1(Map<Byte, Boolean> value, Current current)
    {
        return value;
    }

    @Override
    public String[] opStringS2(String[] value, Current current)
    {
        return value;
    }

    @Override
    public Map<Byte, Boolean> opByteBoolD2(Map<Byte, Boolean> value, Current current)
    {
        return value;
    }

    @Override
    public MyClass1 opMyClass1(MyClass1 value, Current current)
    {
        return value;
    }

    @Override
    public MyStruct1 opMyStruct1(MyStruct1 value, Current current)
    {
        return value;
    }

    @Override
    public String[] opStringLiterals(Current current)
    {
        return new String[]
            {
                s0.value,
                s1.value,
                s2.value,
                s3.value,
                s4.value,
                s5.value,
                s6.value,
                s7.value,
                s8.value,
                s9.value,
                s10.value,

                sw0.value,
                sw1.value,
                sw2.value,
                sw3.value,
                sw4.value,
                sw5.value,
                sw6.value,
                sw7.value,
                sw8.value,
                sw9.value,
                sw10.value,

                ss0.value,
                ss1.value,
                ss2.value,
                ss3.value,
                ss4.value,
                ss5.value,

                su0.value,
                su1.value,
                su2.value
            };
    }

    @Override
    public String[] opWStringLiterals(Current current)
    {
        return opStringLiterals(current);
    }

    @Override
    public OpMStruct1MarshaledResult opMStruct1(Current current)
    {
        return new OpMStruct1MarshaledResult(new Structure(), current);
    }

    @Override
    public OpMStruct2MarshaledResult opMStruct2(Structure p1, Current current)
    {
        return new OpMStruct2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMSeq1MarshaledResult opMSeq1(Current current)
    {
        return new OpMSeq1MarshaledResult(new String[0], current);
    }

    @Override
    public OpMSeq2MarshaledResult opMSeq2(String[] p1, Current current)
    {
        return new OpMSeq2MarshaledResult(p1, p1, current);
    }

    @Override
    public OpMDict1MarshaledResult opMDict1(Current current)
    {
        return new OpMDict1MarshaledResult(new java.util.HashMap<>(), current);
    }

    @Override
    public OpMDict2MarshaledResult opMDict2(java.util.Map<String, String> p1, Current current)
    {
        return new OpMDict2MarshaledResult(p1, p1, current);
    }

    private int _opByteSOnewayCallCount = 0;
}
