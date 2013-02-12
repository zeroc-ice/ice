// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.Test.BoolSHolder;
import test.Ice.operations.Test.BoolSSHolder;
import test.Ice.operations.Test.ByteBoolDHolder;
import test.Ice.operations.Test.ByteSHolder;
import test.Ice.operations.Test.ByteSSHolder;
import test.Ice.operations.Test.DoubleSHolder;
import test.Ice.operations.Test.DoubleSSHolder;
import test.Ice.operations.Test.FloatSHolder;
import test.Ice.operations.Test.FloatSSHolder;
import test.Ice.operations.Test.IntSHolder;
import test.Ice.operations.Test.IntSSHolder;
import test.Ice.operations.Test.LongFloatDHolder;
import test.Ice.operations.Test.LongSHolder;
import test.Ice.operations.Test.LongSSHolder;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyClassPrxHolder;
import test.Ice.operations.Test.MyDerivedClass;
import test.Ice.operations.Test.MyEnum;
import test.Ice.operations.Test.MyEnumHolder;
import test.Ice.operations.Test.MyStruct;
import test.Ice.operations.Test.MyStructMyEnumDHolder;
import test.Ice.operations.Test.ShortIntDHolder;
import test.Ice.operations.Test.ShortSHolder;
import test.Ice.operations.Test.ShortSSHolder;
import test.Ice.operations.Test.StringMyEnumDHolder;
import test.Ice.operations.Test.MyEnumStringDHolder;
import test.Ice.operations.Test.StringSHolder;
import test.Ice.operations.Test.StringSSHolder;
import test.Ice.operations.Test.StringSSSHolder;
import test.Ice.operations.Test.StringStringDHolder;
import test.Ice.operations.Test.Structure;
import test.Ice.operations.Test.StructureHolder;

public final class MyDerivedClassI extends MyDerivedClass
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

    public boolean
    ice_isA(String id, Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_isA(id, current);
    }

    public void
    ice_ping(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        super.ice_ping(current);
    }

    public String[]
    ice_ids(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_ids(current);
    }

    public String
    ice_id(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        return super.ice_id(current);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public void
    delay(int ms, Ice.Current current)
    {
        try
        {
            Thread.sleep(ms);
        }
        catch(InterruptedException ex)
        {
        }
    }

    public void
    opVoid(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);
    }

    public boolean
    opBool(boolean p1, boolean p2,
           Ice.BooleanHolder p3,
           Ice.Current current)
    {
        p3.value = p1;
        return p2;
    }

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

    public byte
    opByte(byte p1, byte p2,
           Ice.ByteHolder p3,
           Ice.Current current)
    {
        p3.value = (byte)(p1 ^ p2);
        return p1;
    }

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

    public double
    opFloatDouble(float p1, double p2,
                  Ice.FloatHolder p3, Ice.DoubleHolder p4,
                  Ice.Current current)
    {
        p3.value = p1;
        p4.value = p2;
        return p2;
    }

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

    public MyClassPrx
    opMyClass(MyClassPrx p1,
              MyClassPrxHolder p2, MyClassPrxHolder p3,
              Ice.Current current)
    {
        p2.value = p1;
        p3.value = MyClassPrxHelper.uncheckedCast(
            current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        return MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
    }

    public MyEnum
    opMyEnum(MyEnum p1,
             MyEnumHolder p2,
             Ice.Current current)
    {
        p2.value = p1;
        return MyEnum.enum3;
    }

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

    public String
    opString(String p1, String p2,
             Ice.StringHolder p3,
             Ice.Current current)
    {
        p3.value = p2 + " " + p1;
        return p1 + " " + p2;
    }

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

    public void
    opByteSOneway(byte[] s, Ice.Current current)
    {
    }

    public java.util.Map<String, String>
    opContext(Ice.Current current)
    {
        return current.ctx;
    }

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

    public Structure
    opStruct(Structure p1, Structure p2,
             StructureHolder p3,
             Ice.Current current)
    {
        p3.value = p1;
        p3.value.s.s = "a new string";
        return p2;
    }

    public void
    opIdempotent(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
    }

    public void
    opNonmutating(Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
    }

    public void
    opDerived(Ice.Current current)
    {
    }
}
