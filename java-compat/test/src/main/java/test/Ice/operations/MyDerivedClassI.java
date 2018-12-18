// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import Ice.Current;
import test.Ice.operations.Test.*;

import java.util.*;

public final class MyDerivedClassI extends _MyDerivedClassDisp
{
    private static void
    test(boolean b)
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
    public boolean
    ice_isA(String id, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_isA(id, current);
    }

    @Override
    public void
    ice_ping(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        super.ice_ping(current);
    }

    @Override
    public String[]
    ice_ids(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_ids(current);
    }

    @Override
    public String
    ice_id(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_id(current);
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public boolean
    supportsCompress(Ice.Current current)
    {
        return IceInternal.BZip2.supported();
    }

    @Override
    public void
    opVoid(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);
    }

    @Override
    public boolean
    opBool(boolean p1, boolean p2,
           Ice.BooleanHolder p3,
           Ice.Current current)
    {
        p3.value = p1;
        return p2;
    }

    @Override
    public boolean[]
    opBoolS(boolean[] p1, boolean[] p2,
            BoolSHolder p3,
            Ice.Current current)
    {
        p3.value = new boolean[p1.length + p2.length];
        System.arraycopy(p1, 0, p3.value, 0, p1.length);
        System.arraycopy(p2, 0, p3.value, p1.length, p2.length);

        boolean[] r = new boolean[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public boolean[][]
    opBoolSS(boolean[][] p1, boolean[][] p2,
             BoolSSHolder p3,
             Ice.Current current)
    {
        p3.value = new boolean[p1.length + p2.length][];
        System.arraycopy(p1, 0, p3.value, 0, p1.length);
        System.arraycopy(p2, 0, p3.value, p1.length, p2.length);

        boolean[][] r = new boolean[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public byte
    opByte(byte p1, byte p2,
           Ice.ByteHolder p3,
           Ice.Current current)
    {
        p3.value = (byte) (p1 ^ p2);
        return p1;
    }

    @Override
    public java.util.Map<Byte, Boolean>
    opByteBoolD(java.util.Map<Byte, Boolean> p1, java.util.Map<Byte, Boolean> p2, ByteBoolDHolder p3,
                Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<Byte, Boolean> r = new java.util.HashMap<Byte, Boolean>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public byte[]
    opByteS(byte[] p1, byte[] p2,
            ByteSHolder p3,
            Ice.Current current)
    {
        p3.value = new byte[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            p3.value[i] = p1[p1.length - (i + 1)];
        }

        byte[] r = new byte[p1.length + p2.length];
        System.arraycopy(p1, 0, r, 0, p1.length);
        System.arraycopy(p2, 0, r, p1.length, p2.length);
        return r;
    }

    @Override
    public byte[][]
    opByteSS(byte[][] p1, byte[][] p2,
             ByteSSHolder p3,
             Ice.Current current)
    {
        p3.value = new byte[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            p3.value[i] = p1[p1.length - (i + 1)];
        }

        byte[][] r = new byte[p1.length + p2.length][];
        System.arraycopy(p1, 0, r, 0, p1.length);
        System.arraycopy(p2, 0, r, p1.length, p2.length);
        return r;
    }

    @Override
    public double
    opFloatDouble(float p1, double p2,
                  Ice.FloatHolder p3, Ice.DoubleHolder p4,
                  Ice.Current current)
    {
        p3.value = p1;
        p4.value = p2;
        return p2;
    }

    @Override
    public double[]
    opFloatDoubleS(float[] p1, double[] p2,
                   FloatSHolder p3, DoubleSHolder p4,
                   Ice.Current current)
    {
        p3.value = p1;
        p4.value = new double[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            p4.value[i] = p2[p2.length - (i + 1)];
        }
        double[] r = new double[p2.length + p1.length];
        System.arraycopy(p2, 0, r, 0, p2.length);
        for(int i = 0; i < p1.length; i++)
        {
            r[p2.length + i] = p1[i];
        }
        return r;
    }

    @Override
    public double[][]
    opFloatDoubleSS(float[][] p1, double[][] p2,
                    FloatSSHolder p3, DoubleSSHolder p4,
                    Ice.Current current)
    {
        p3.value = p1;
        p4.value = new double[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            p4.value[i] = p2[p2.length - (i + 1)];
        }
        double[][] r = new double[p2.length * 2][];
        System.arraycopy(p2, 0, r, 0, p2.length);
        System.arraycopy(p2, 0, r, p2.length, p2.length);
        return r;
    }

    @Override
    public java.util.Map<Long, Float>
    opLongFloatD(java.util.Map<Long, Float> p1, java.util.Map<Long, Float> p2, LongFloatDHolder p3,
                 Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<Long, Float> r = new java.util.HashMap<Long, Float>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public MyClassPrx
    opMyClass(MyClassPrx p1,
              MyClassPrxHolder p2, MyClassPrxHolder p3,
              Ice.Current current)
    {
        p2.value = p1;
        p3.value = MyClassPrxHelper.uncheckedCast(
                current.adapter.createProxy(Ice.Util.stringToIdentity("noSuchIdentity")));
        return MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
    }

    @Override
    public MyEnum
    opMyEnum(MyEnum p1,
             MyEnumHolder p2,
             Ice.Current current)
    {
        p2.value = p1;
        return MyEnum.enum3;
    }

    @Override
    public java.util.Map<Short, Integer>
    opShortIntD(java.util.Map<Short, Integer> p1, java.util.Map<Short, Integer> p2, ShortIntDHolder p3,
                Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<Short, Integer> r = new java.util.HashMap<Short, Integer>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public long
    opShortIntLong(short p1, int p2, long p3,
                   Ice.ShortHolder p4, Ice.IntHolder p5, Ice.LongHolder p6,
                   Ice.Current current)
    {
        p4.value = p1;
        p5.value = p2;
        p6.value = p3;
        return p3;
    }

    @Override
    public long[]
    opShortIntLongS(short[] p1, int[] p2, long[] p3,
                    ShortSHolder p4, IntSHolder p5, LongSHolder p6,
                    Ice.Current current)
    {
        p4.value = p1;
        p5.value = new int[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            p5.value[i] = p2[p2.length - (i + 1)];
        }
        p6.value = new long[p3.length * 2];
        System.arraycopy(p3, 0, p6.value, 0, p3.length);
        System.arraycopy(p3, 0, p6.value, p3.length, p3.length);
        return p3;
    }

    @Override
    public long[][]
    opShortIntLongSS(short[][] p1, int[][] p2, long[][] p3,
                     ShortSSHolder p4, IntSSHolder p5, LongSSHolder p6,
                     Ice.Current current)
    {
        p4.value = p1;
        p5.value = new int[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            p5.value[i] = p2[p2.length - (i + 1)];
        }
        p6.value = new long[p3.length * 2][];
        System.arraycopy(p3, 0, p6.value, 0, p3.length);
        System.arraycopy(p3, 0, p6.value, p3.length, p3.length);
        return p3;
    }

    @Override
    public String
    opString(String p1, String p2,
             Ice.StringHolder p3,
             Ice.Current current)
    {
        p3.value = p2 + " " + p1;
        return p1 + " " + p2;
    }

    @Override
    public java.util.Map<String, MyEnum>
    opStringMyEnumD(java.util.Map<String, MyEnum> p1, java.util.Map<String, MyEnum> p2, StringMyEnumDHolder p3,
                    Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<String, MyEnum> r = new java.util.HashMap<String, MyEnum>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public java.util.Map<MyEnum, String>
    opMyEnumStringD(java.util.Map<MyEnum, String> p1, java.util.Map<MyEnum, String> p2, MyEnumStringDHolder p3,
                    Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<MyEnum, String> r = new java.util.HashMap<MyEnum, String>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public java.util.Map<MyStruct, MyEnum>
    opMyStructMyEnumD(java.util.Map<MyStruct, MyEnum> p1, java.util.Map<MyStruct, MyEnum> p2, MyStructMyEnumDHolder p3,
                      Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<MyStruct, MyEnum> r = new java.util.HashMap<MyStruct, MyEnum>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public List<Map<Byte, Boolean>> opByteBoolDS(List<Map<Byte, Boolean>> p1,
                                                 List<Map<Byte, Boolean>> p2,
                                                 ByteBoolDSHolder p3,
                                                 Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<Byte, Boolean>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<Short, Integer>> opShortIntDS(List<Map<Short, Integer>> p1,
                                                  List<Map<Short, Integer>> p2,
                                                  ShortIntDSHolder p3,
                                                  Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<Short, Integer>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<Long, Float>> opLongFloatDS(List<Map<Long, Float>> p1,
                                                List<Map<Long, Float>> p2,
                                                LongFloatDSHolder p3,
                                                Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<Long, Float>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<String, String>> opStringStringDS(List<Map<String, String>> p1,
                                                      List<Map<String, String>> p2,
                                                      StringStringDSHolder p3,
                                                      Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<String, String>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<String, MyEnum>> opStringMyEnumDS(List<Map<String, MyEnum>> p1,
                                                      List<Map<String, MyEnum>> p2,
                                                      StringMyEnumDSHolder p3,
                                                      Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<String, MyEnum>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<MyEnum, String>> opMyEnumStringDS(List<Map<MyEnum, String>> p1,
                                                      List<Map<MyEnum, String>> p2,
                                                      MyEnumStringDSHolder p3,
                                                      Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<MyEnum, String>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public List<Map<MyStruct, MyEnum>> opMyStructMyEnumDS(List<Map<MyStruct, MyEnum>> p1,
                                                          List<Map<MyStruct, MyEnum>> p2,
                                                          MyStructMyEnumDSHolder p3,
                                                          Ice.Current current)
    {
        p3.value = new ArrayList<>();
        p3.value.addAll(p2);
        p3.value.addAll(p1);

        List<Map<MyStruct, MyEnum>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        return r;
    }

    @Override
    public Map<Byte, byte[]> opByteByteSD(Map<Byte, byte[]> p1,
                                          Map<Byte, byte[]> p2,
                                          ByteByteSDHolder p3,
                                          Ice.Current current)
    {
        p3.value = p2;
        Map<Byte, byte[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<Boolean, boolean[]> opBoolBoolSD(Map<Boolean, boolean[]> p1,
                                                Map<Boolean, boolean[]> p2,
                                                BoolBoolSDHolder p3,
                                                Ice.Current current)
    {
        p3.value = p2;
        Map<Boolean, boolean[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<Short, short[]> opShortShortSD(Map<Short, short[]> p1,
                                              Map<Short, short[]> p2,
                                              ShortShortSDHolder p3,
                                              Ice.Current current)
    {
        p3.value = p2;
        Map<Short, short[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<Integer, int[]> opIntIntSD(Map<Integer, int[]> p1,
                                          Map<Integer, int[]> p2,
                                          IntIntSDHolder p3,
                                          Ice.Current current)
    {
        p3.value = p2;
        Map<Integer, int[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<Long, long[]> opLongLongSD(Map<Long, long[]> p1,
                                          Map<Long, long[]> p2,
                                          LongLongSDHolder p3,
                                          Ice.Current current)
    {
        p3.value = p2;
        Map<Long, long[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<String, float[]> opStringFloatSD(Map<String, float[]> p1,
                                                Map<String, float[]> p2,
                                                StringFloatSDHolder p3,
                                                Ice.Current current)
    {
        p3.value = p2;
        Map<String, float[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<String, double[]> opStringDoubleSD(Map<String, double[]> p1,
                                                  Map<String, double[]> p2,
                                                  StringDoubleSDHolder p3,
                                                  Ice.Current current)
    {
        p3.value = p2;
        Map<String, double[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<String, String[]> opStringStringSD(Map<String, String[]> p1,
                                                  Map<String, String[]> p2,
                                                  StringStringSDHolder p3,
                                                  Ice.Current current)
    {
        p3.value = p2;
        Map<String, String[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Map<MyEnum, MyEnum[]> opMyEnumMyEnumSD(Map<MyEnum, MyEnum[]> p1,
                                                  Map<MyEnum, MyEnum[]> p2,
                                                  MyEnumMyEnumSDHolder p3,
                                                  Ice.Current current)
    {
        p3.value = p2;
        Map<MyEnum, MyEnum[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public int[]
    opIntS(int[] s, Ice.Current current)
    {
        int[] r = new int[s.length];
        for(int i = 0; i < r.length; ++i)
        {
            r[i] = -s[i];
        }
        return r;
    }

    @Override
    public synchronized void
    opByteSOneway(byte[] s, Ice.Current current)
    {
        ++_opByteSOnewayCallCount;
    }

    @Override
    public synchronized int
    opByteSOnewayCallCount(Ice.Current current)
    {
        int count = _opByteSOnewayCallCount;
        _opByteSOnewayCallCount = 0;
        return count;
    }

    @Override
    public java.util.Map<String, String>
    opContext(Ice.Current current)
    {
        return current.ctx;
    }

    @Override
    public void
    opDoubleMarshaling(double p1, double[] p2, Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.length; ++i)
        {
            test(p2[i] == d);
        }
    }

    @Override
    public String[]
    opStringS(String[] p1, String[] p2,
              StringSHolder p3,
              Ice.Current current)
    {
        p3.value = new String[p1.length + p2.length];
        System.arraycopy(p1, 0, p3.value, 0, p1.length);
        System.arraycopy(p2, 0, p3.value, p1.length, p2.length);

        String[] r = new String[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        return r;
    }

    @Override
    public String[][]
    opStringSS(String[][] p1, String[][] p2,
               StringSSHolder p3,
               Ice.Current current)
    {
        p3.value = new String[p1.length + p2.length][];
        System.arraycopy(p1, 0, p3.value, 0, p1.length);
        System.arraycopy(p2, 0, p3.value, p1.length, p2.length);

        String[][] r = new String[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r[i] = p2[p2.length - (i + 1)];
        }
        return r;
    }

    @Override
    public String[][][]
    opStringSSS(String[][][] p1, String[][][] p2,
                StringSSSHolder p3,
                Ice.Current current)
    {
        p3.value = new String[p1.length + p2.length][][];
        System.arraycopy(p1, 0, p3.value, 0, p1.length);
        System.arraycopy(p2, 0, p3.value, p1.length, p2.length);

        String[][][] r = new String[p2.length][][];
        for(int i = 0; i < p2.length; i++)
        {
            r[i] = p2[p2.length - (i + 1)];
        }
        return r;
    }

    @Override
    public java.util.Map<String, String>
    opStringStringD(java.util.Map<String, String> p1, java.util.Map<String, String> p2, StringStringDHolder p3,
                    Ice.Current current)
    {
        p3.value = p1;
        java.util.Map<String, String> r = new java.util.HashMap<String, String>();
        r.putAll(p1);
        r.putAll(p2);
        return r;
    }

    @Override
    public Structure
    opStruct(Structure p1, Structure p2,
             StructureHolder p3,
             Ice.Current current)
    {
        p3.value = p1;
        p3.value.s.s = "a new string";
        return p2;
    }

    @Override
    public void
    opIdempotent(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
    }

    @Override
    public void
    opNonmutating(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
    }

    @Override
    public void
    opDerived(Ice.Current current)
    {
    }

    @Override
    public byte opByte1(byte value, Ice.Current current)
    {
        return value;
    }

    @Override
    public short opShort1(short value, Ice.Current current)
    {
        return value;
    }

    @Override
    public int opInt1(int value, Ice.Current current)
    {
        return value;
    }

    @Override
    public long opLong1(long value, Ice.Current current)
    {
        return value;
    }

    @Override
    public float opFloat1(float value, Ice.Current current)
    {
        return value;
    }

    @Override
    public double opDouble1(double value, Ice.Current current)
    {
        return value;
    }

    @Override
    public String opString1(String value, Ice.Current current)
    {
        return value;
    }

    @Override
    public String[] opStringS1(String[] value, Ice.Current current)
    {
        return value;
    }

    @Override
    public Map<Byte, Boolean> opByteBoolD1(Map<Byte, Boolean> value, Ice.Current current)
    {
        return value;
    }

    @Override
    public String[] opStringS2(String[] value, Ice.Current current)
    {
        return value;
    }

    @Override
    public Map<Byte, Boolean> opByteBoolD2(Map<Byte, Boolean> value, Ice.Current current)
    {
        return value;
    }

    @Override
    public MyClass1 opMyClass1(MyClass1 value, Ice.Current current)
    {
        return value;
    }

    @Override
    public MyStruct1 opMyStruct1(MyStruct1 value, Ice.Current current)
    {
        return value;
    }

    @Override
    public String[] opStringLiterals(Ice.Current current)
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
    public String[] opWStringLiterals(Ice.Current current)
    {
        return opStringLiterals(current);
    }

    @Override
    public Structure opMStruct1(Ice.Current current)
    {
        return new Structure();
    }

    @Override
    public Structure opMStruct2(Structure p1, StructureHolder p2, Ice.Current current)
    {
        p2.value = p1;
        return p1;
    }

    @Override
    public String[] opMSeq1(Ice.Current current)
    {
        return new String[0];
    }

    @Override
    public String[] opMSeq2(String[] p1, StringSHolder p2, Ice.Current current)
    {
        p2.value = p1;
        return p1;
    }

    @Override
    public Map<String, String> opMDict1(Ice.Current current)
    {
        return new HashMap<String, String>();
    }

    @Override
    public Map<String, String> opMDict2(Map<String, String> p1, StringStringDHolder p2, Ice.Current current)
    {
        p2.value = p1;
        return p1;
    }

    private int _opByteSOnewayCallCount = 0;
}
