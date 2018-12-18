// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.operations;

import Ice.Current;
import test.Ice.operations.AMD.Test.*;

import java.util.*;

public final class AMDTieMyDerivedClassI implements _MyDerivedClassOperations
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Thread_opVoid extends Thread
    {
        public Thread_opVoid(AMD_MyClass_opVoid cb)
        {
            _cb = cb;
        }

        @Override
        public void
        run()
        {
            _cb.ice_response();
        }

        private AMD_MyClass_opVoid _cb;
    }

    @Override
    synchronized public void
    shutdown_async(AMD_MyClass_shutdown cb,
                   Ice.Current current)
    {
        while(_opVoidThread != null)
        {
            try
            {
                _opVoidThread.join();
                _opVoidThread = null;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    synchronized public void
    supportsCompress_async(AMD_MyClass_supportsCompress cb,
                           Ice.Current current)
    {
        cb.ice_response(true);
    }

    @Override
    synchronized public void
    opVoid_async(AMD_MyClass_opVoid cb,
                 Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Normal);

        while(_opVoidThread != null)
        {
            try
            {
                _opVoidThread.join();
                _opVoidThread = null;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        _opVoidThread = new Thread_opVoid(cb);
        _opVoidThread.start();
    }

    @Override
    public void
    opBool_async(AMD_MyClass_opBool cb,
                 boolean p1, boolean p2,
                 Ice.Current current)
    {
        cb.ice_response(p2, p1);
    }

    @Override
    public void
    opBoolS_async(AMD_MyClass_opBoolS cb,
                  boolean[] p1, boolean[] p2,
                  Ice.Current current)
    {
        boolean[] p3 = new boolean[p1.length + p2.length];
        System.arraycopy(p1, 0, p3, 0, p1.length);
        System.arraycopy(p2, 0, p3, p1.length, p2.length);

        boolean[] r = new boolean[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opBoolSS_async(AMD_MyClass_opBoolSS cb,
                   boolean[][] p1, boolean[][] p2,
                   Ice.Current current)
    {
        boolean[][] p3 = new boolean[p1.length + p2.length][];
        System.arraycopy(p1, 0, p3, 0, p1.length);
        System.arraycopy(p2, 0, p3, p1.length, p2.length);

        boolean[][] r = new boolean[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opByte_async(AMD_MyClass_opByte cb,
                 byte p1, byte p2,
                 Ice.Current current)
    {
        cb.ice_response(p1, (byte) (p1 ^ p2));
    }

    @Override
    public void
    opByteBoolD_async(AMD_MyClass_opByteBoolD cb,
                      java.util.Map<Byte, Boolean> p1, java.util.Map<Byte, Boolean> p2,
                      Ice.Current current)
    {
        java.util.Map<Byte, Boolean> p3 = p1;
        java.util.Map<Byte, Boolean> r = new java.util.HashMap<Byte, Boolean>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opByteS_async(AMD_MyClass_opByteS cb,
                  byte[] p1, byte[] p2,
                  Ice.Current current)
    {
        byte[] p3 = new byte[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            p3[i] = p1[p1.length - (i + 1)];
        }

        byte[] r = new byte[p1.length + p2.length];
        System.arraycopy(p1, 0, r, 0, p1.length);
        System.arraycopy(p2, 0, r, p1.length, p2.length);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opByteSS_async(AMD_MyClass_opByteSS cb,
                   byte[][] p1, byte[][] p2,
                   Ice.Current current)
    {
        byte[][] p3 = new byte[p1.length][];
        for(int i = 0; i < p1.length; i++)
        {
            p3[i] = p1[p1.length - (i + 1)];
        }

        byte[][] r = new byte[p1.length + p2.length][];
        System.arraycopy(p1, 0, r, 0, p1.length);
        System.arraycopy(p2, 0, r, p1.length, p2.length);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opFloatDouble_async(AMD_MyClass_opFloatDouble cb,
                        float p1, double p2,
                        Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }

    @Override
    public void
    opFloatDoubleS_async(AMD_MyClass_opFloatDoubleS cb,
                         float[] p1, double[] p2,
                         Ice.Current current)
    {
        float[] p3 = p1;
        double[] p4 = new double[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            p4[i] = p2[p2.length - (i + 1)];
        }
        double[] r = new double[p2.length + p1.length];
        System.arraycopy(p2, 0, r, 0, p2.length);
        for(int i = 0; i < p1.length; i++)
        {
            r[p2.length + i] = p1[i];
        }
        cb.ice_response(r, p3, p4);
    }

    @Override
    public void
    opFloatDoubleSS_async(AMD_MyClass_opFloatDoubleSS cb,
                          float[][] p1, double[][] p2,
                          Ice.Current current)
    {
        float[][] p3 = p1;
        double[][] p4 = new double[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            p4[i] = p2[p2.length - (i + 1)];
        }
        double[][] r = new double[p2.length * 2][];
        System.arraycopy(p2, 0, r, 0, p2.length);
        System.arraycopy(p2, 0, r, p2.length, p2.length);
        cb.ice_response(r, p3, p4);
    }

    @Override
    public void
    opLongFloatD_async(AMD_MyClass_opLongFloatD cb,
                       java.util.Map<Long, Float> p1, java.util.Map<Long, Float> p2,
                       Ice.Current current)
    {
        java.util.Map<Long, Float> p3 = p1;
        java.util.Map<Long, Float> r = new java.util.HashMap<Long, Float>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opMyClass_async(AMD_MyClass_opMyClass cb,
                    MyClassPrx p1,
                    Ice.Current current)
    {
        MyClassPrx p2 = p1;
        MyClassPrx p3 = MyClassPrxHelper.uncheckedCast(
                current.adapter.createProxy(Ice.Util.stringToIdentity("noSuchIdentity")));
        cb.ice_response(MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3);
    }

    @Override
    public void
    opMyEnum_async(AMD_MyClass_opMyEnum cb,
                   MyEnum p1,
                   Ice.Current current)
    {
        cb.ice_response(MyEnum.enum3, p1);
    }

    @Override
    public void
    opShortIntD_async(AMD_MyClass_opShortIntD cb,
                      java.util.Map<Short, Integer> p1, java.util.Map<Short, Integer> p2,
                      Ice.Current current)
    {
        java.util.Map<Short, Integer> p3 = p1;
        java.util.Map<Short, Integer> r = new java.util.HashMap<Short, Integer>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opShortIntLong_async(AMD_MyClass_opShortIntLong cb,
                         short p1, int p2, long p3,
                         Ice.Current current)
    {
        cb.ice_response(p3, p1, p2, p3);
    }

    @Override
    public void
    opShortIntLongS_async(AMD_MyClass_opShortIntLongS cb,
                          short[] p1, int[] p2, long[] p3,
                          Ice.Current current)
    {
        short[] p4 = p1;
        int[] p5 = new int[p2.length];
        for(int i = 0; i < p2.length; i++)
        {
            p5[i] = p2[p2.length - (i + 1)];
        }
        long[] p6 = new long[p3.length * 2];
        System.arraycopy(p3, 0, p6, 0, p3.length);
        System.arraycopy(p3, 0, p6, p3.length, p3.length);
        cb.ice_response(p3, p4, p5, p6);
    }

    @Override
    public void
    opShortIntLongSS_async(AMD_MyClass_opShortIntLongSS cb,
                           short[][] p1, int[][] p2, long[][] p3,
                           Ice.Current current)
    {
        short[][] p4 = p1;
        int[][] p5 = new int[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            p5[i] = p2[p2.length - (i + 1)];
        }
        long[][] p6 = new long[p3.length * 2][];
        System.arraycopy(p3, 0, p6, 0, p3.length);
        System.arraycopy(p3, 0, p6, p3.length, p3.length);
        cb.ice_response(p3, p4, p5, p6);
    }

    @Override
    public void
    opString_async(AMD_MyClass_opString cb,
                   String p1, String p2,
                   Ice.Current current)
    {
        cb.ice_response(p1 + " " + p2, p2 + " " + p1);
    }

    @Override
    public void
    opStringMyEnumD_async(AMD_MyClass_opStringMyEnumD cb,
                          java.util.Map<String, MyEnum> p1, java.util.Map<String, MyEnum> p2,
                          Ice.Current current)
    {
        java.util.Map<String, MyEnum> p3 = p1;
        java.util.Map<String, MyEnum> r = new java.util.HashMap<String, MyEnum>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opMyEnumStringD_async(AMD_MyClass_opMyEnumStringD cb,
                          java.util.Map<MyEnum, String> p1, java.util.Map<MyEnum, String> p2,
                          Ice.Current current)
    {
        java.util.Map<MyEnum, String> p3 = p1;
        java.util.Map<MyEnum, String> r = new java.util.HashMap<MyEnum, String>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opMyStructMyEnumD_async(AMD_MyClass_opMyStructMyEnumD cb,
                            java.util.Map<MyStruct, MyEnum> p1, java.util.Map<MyStruct, MyEnum> p2,
                            Ice.Current current)
    {
        java.util.Map<MyStruct, MyEnum> p3 = p1;
        java.util.Map<MyStruct, MyEnum> r = new java.util.HashMap<MyStruct, MyEnum>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opByteBoolDS_async(AMD_MyClass_opByteBoolDS cb,
                                   List<Map<Byte, Boolean>> p1,
                                   List<Map<Byte, Boolean>> p2,
                                   Ice.Current current)
    {

        ArrayList<Map<Byte, Boolean>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<Byte, Boolean>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opShortIntDS_async(AMD_MyClass_opShortIntDS cb,
                                   List<Map<Short, Integer>> p1,
                                   List<Map<Short, Integer>> p2,
                                   Ice.Current current)
    {
        List<Map<Short, Integer>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<Short, Integer>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opLongFloatDS_async(AMD_MyClass_opLongFloatDS cb,
                                    List<Map<Long, Float>> p1,
                                    List<Map<Long, Float>> p2,
                                    Ice.Current current)
    {
        List<Map<Long, Float>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<Long, Float>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opStringStringDS_async(AMD_MyClass_opStringStringDS cb,
                                       List<Map<String, String>> p1,
                                       List<Map<String, String>> p2,
                                       Ice.Current current)
    {
        List<Map<String, String>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<String, String>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opStringMyEnumDS_async(AMD_MyClass_opStringMyEnumDS cb,
                                       List<Map<String, MyEnum>> p1,
                                       List<Map<String, MyEnum>> p2,
                                       Ice.Current current)
    {
        List<Map<String, MyEnum>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<String, MyEnum>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opMyEnumStringDS_async(AMD_MyClass_opMyEnumStringDS cb,
                                       List<Map<MyEnum, String>> p1,
                                       List<Map<MyEnum, String>> p2,
                                       Ice.Current current)
    {
        List<Map<MyEnum, String>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<MyEnum, String>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opMyStructMyEnumDS_async(AMD_MyClass_opMyStructMyEnumDS cb,
                                         List<Map<MyStruct, MyEnum>> p1,
                                         List<Map<MyStruct, MyEnum>> p2,
                                         Ice.Current current)
    {
        List<Map<MyStruct, MyEnum>> p3= new ArrayList<>();
        p3.addAll(p2);
        p3.addAll(p1);

        List<Map<MyStruct, MyEnum>> r = new ArrayList<>(p1);
        Collections.reverse(r);

        cb.ice_response(r, p3);
    }

    @Override
    public void opByteByteSD_async(AMD_MyClass_opByteByteSD cb,
                                   Map<Byte, byte[]> p1,
                                   Map<Byte, byte[]> p2,
                                   Ice.Current current)
    {
        Map<Byte, byte[]> p3 = p2;
        Map<Byte, byte[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opBoolBoolSD_async(AMD_MyClass_opBoolBoolSD cb,
                                   Map<Boolean, boolean[]> p1,
                                   Map<Boolean, boolean[]> p2,
                                   Ice.Current current)
    {
        Map<Boolean, boolean[]> p3 = p2;
        Map<Boolean, boolean[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opShortShortSD_async(AMD_MyClass_opShortShortSD cb,
                                     Map<Short, short[]> p1,
                                     Map<Short, short[]> p2,
                                     Ice.Current current)
    {
        Map<Short, short[]> p3 = p2;
        Map<Short, short[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opIntIntSD_async(AMD_MyClass_opIntIntSD cb,
                                 Map<Integer, int[]> p1,
                                 Map<Integer, int[]> p2,
                                 Ice.Current current)
    {
        Map<Integer, int[]> p3 = p2;
        Map<Integer, int[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opLongLongSD_async(AMD_MyClass_opLongLongSD cb,
                                   Map<Long, long[]> p1,
                                   Map<Long, long[]> p2,
                                   Ice.Current current)
    {
        Map<Long, long[]> p3 = p2;
        Map<Long, long[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opStringFloatSD_async(AMD_MyClass_opStringFloatSD cb,
                                      Map<String, float[]> p1,
                                      Map<String, float[]> p2,
                                      Ice.Current current)
    {
        Map<String, float[]> p3 = p2;
        Map<String, float[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opStringDoubleSD_async(AMD_MyClass_opStringDoubleSD cb,
                                       Map<String, double[]> p1,
                                       Map<String, double[]> p2,
                                       Ice.Current current)
    {
        Map<String, double[]> p3 = p2;
        Map<String, double[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opStringStringSD_async(AMD_MyClass_opStringStringSD cb,
                                       Map<String, String[]> p1,
                                       Map<String, String[]> p2,
                                       Ice.Current current)
    {
        Map<String, String[]> p3 = p2;
        Map<String, String[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void opMyEnumMyEnumSD_async(AMD_MyClass_opMyEnumMyEnumSD cb,
                                       Map<MyEnum, MyEnum[]> p1,
                                       Map<MyEnum, MyEnum[]> p2,
                                       Ice.Current current)
    {
        Map<MyEnum, MyEnum[]> p3 = p2;
        Map<MyEnum, MyEnum[]> r = new HashMap<>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opIntS_async(AMD_MyClass_opIntS cb, int[] s, Ice.Current current)
    {
        int[] r = new int[s.length];
        for(int i = 0; i < r.length; ++i)
        {
            r[i] = -s[i];
        }
        cb.ice_response(r);
    }

    @Override
    public synchronized void
    opByteSOneway_async(AMD_MyClass_opByteSOneway cb, byte[] s, Ice.Current current)
    {
        ++_opByteSOnewayCallCount;
        cb.ice_response();
    }

    @Override
    public synchronized void
    opByteSOnewayCallCount_async(AMD_MyClass_opByteSOnewayCallCount cb, Ice.Current current)
    {
        int count = _opByteSOnewayCallCount;
        _opByteSOnewayCallCount = 0;
        cb.ice_response(count);
    }

    @Override
    public void
    opContext_async(AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }

    @Override
    public void
    opDoubleMarshaling_async(AMD_MyClass_opDoubleMarshaling cb, double p1, double[] p2, Ice.Current current)
    {
        double d = 1278312346.0 / 13.0;
        test(p1 == d);
        for(int i = 0; i < p2.length; ++i)
        {
            test(p2[i] == d);
        }
        cb.ice_response();
    }

    @Override
    public void
    opStringS_async(AMD_MyClass_opStringS cb,
                    String[] p1, String[] p2,
                    Ice.Current current)
    {
        String[] p3 = new String[p1.length + p2.length];
        System.arraycopy(p1, 0, p3, 0, p1.length);
        System.arraycopy(p2, 0, p3, p1.length, p2.length);

        String[] r = new String[p1.length];
        for(int i = 0; i < p1.length; i++)
        {
            r[i] = p1[p1.length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opStringSS_async(AMD_MyClass_opStringSS cb,
                     String[][] p1, String[][] p2,
                     Ice.Current current)
    {
        String[][] p3 = new String[p1.length + p2.length][];
        System.arraycopy(p1, 0, p3, 0, p1.length);
        System.arraycopy(p2, 0, p3, p1.length, p2.length);

        String[][] r = new String[p2.length][];
        for(int i = 0; i < p2.length; i++)
        {
            r[i] = p2[p2.length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opStringSSS_async(AMD_MyClass_opStringSSS cb,
                      String[][][] p1, String[][][] p2,
                      Ice.Current current)
    {
        String[][][] p3 = new String[p1.length + p2.length][][];
        System.arraycopy(p1, 0, p3, 0, p1.length);
        System.arraycopy(p2, 0, p3, p1.length, p2.length);

        String[][][] r = new String[p2.length][][];
        for(int i = 0; i < p2.length; i++)
        {
            r[i] = p2[p2.length - (i + 1)];
        }
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opStringStringD_async(AMD_MyClass_opStringStringD cb,
                          java.util.Map<String, String> p1, java.util.Map<String, String> p2,
                          Ice.Current current)
    {
        java.util.Map<String, String> p3 = p1;
        java.util.Map<String, String> r = new java.util.HashMap<String, String>();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    @Override
    public void
    opStruct_async(AMD_MyClass_opStruct cb,
                   Structure p1, Structure p2,
                   Ice.Current current)
    {
        Structure p3 = p1;
        p3.s.s = "a new string";
        cb.ice_response(p2, p3);
    }

    @Override
    public void
    opIdempotent_async(AMD_MyClass_opIdempotent cb,
                       Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        cb.ice_response();
    }

    @Override
    public void
    opNonmutating_async(AMD_MyClass_opNonmutating cb,
                        Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        cb.ice_response();
    }

    @Override
    public void
    opDerived_async(AMD_MyDerivedClass_opDerived cb,
                    Ice.Current current)
    {
        cb.ice_response();
    }

    @Override
    public void opByte1_async(AMD_MyClass_opByte1 cb, byte value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opShort1_async(AMD_MyClass_opShort1 cb, short value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opInt1_async(AMD_MyClass_opInt1 cb, int value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opLong1_async(AMD_MyClass_opLong1 cb, long value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opFloat1_async(AMD_MyClass_opFloat1 cb, float value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opDouble1_async(AMD_MyClass_opDouble1 cb, double value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opString1_async(AMD_MyClass_opString1 cb, String value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opStringS1_async(AMD_MyClass_opStringS1 cb, String[] value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opByteBoolD1_async(AMD_MyClass_opByteBoolD1 cb, Map<Byte, Boolean> value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opStringS2_async(AMD_MyClass_opStringS2 cb, String[] value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opByteBoolD2_async(AMD_MyClass_opByteBoolD2 cb, Map<Byte, Boolean> value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opMyClass1_async(AMD_MyDerivedClass_opMyClass1 cb, MyClass1 value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opMyStruct1_async(AMD_MyDerivedClass_opMyStruct1 cb, MyStruct1 value, Ice.Current current)
    {
        cb.ice_response(value);
    }

    @Override
    public void opStringLiterals_async(AMD_MyClass_opStringLiterals cb, Ice.Current current)
    {
        cb.ice_response(new String[]
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
            });
    }

    @Override
    public void opWStringLiterals_async(AMD_MyClass_opWStringLiterals cb, Ice.Current current)
    {
        cb.ice_response(new String[]
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
            });
    }

    @Override
    public void opMStruct1_async(AMD_MyClass_opMStruct1 cb, Ice.Current current)
    {
        cb.ice_response(new Structure());
    }

    @Override
    public void opMStruct2_async(AMD_MyClass_opMStruct2 cb, Structure p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void opMSeq1_async(AMD_MyClass_opMSeq1 cb, Ice.Current current)
    {
        cb.ice_response(new String[0]);
    }

    @Override
    public void opMSeq2_async(AMD_MyClass_opMSeq2 cb, String[] p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    @Override
    public void opMDict1_async(AMD_MyClass_opMDict1 cb, Ice.Current current)
    {
        cb.ice_response(new java.util.HashMap<String, String>());
    }

    @Override
    public void opMDict2_async(AMD_MyClass_opMDict2 cb, java.util.Map<String, String> p1, Ice.Current current)
    {
        cb.ice_response(p1, p1);
    }

    private Thread _opVoidThread;
    private int _opByteSOnewayCallCount = 0;
}
