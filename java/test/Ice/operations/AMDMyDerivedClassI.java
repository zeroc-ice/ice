// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import test.Ice.operations.AMD.Test.AMD_MyClass_opBool;
import test.Ice.operations.AMD.Test.AMD_MyClass_opBoolS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opBoolSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opByte;
import test.Ice.operations.AMD.Test.AMD_MyClass_opByteBoolD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opByteS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opByteSOneway;
import test.Ice.operations.AMD.Test.AMD_MyClass_opByteSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opContext;
import test.Ice.operations.AMD.Test.AMD_MyClass_opDoubleMarshaling;
import test.Ice.operations.AMD.Test.AMD_MyClass_opFloatDouble;
import test.Ice.operations.AMD.Test.AMD_MyClass_opFloatDoubleS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opFloatDoubleSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opIdempotent;
import test.Ice.operations.AMD.Test.AMD_MyClass_opIntS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opLongFloatD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opMyClass;
import test.Ice.operations.AMD.Test.AMD_MyClass_opMyEnum;
import test.Ice.operations.AMD.Test.AMD_MyClass_opMyStructMyEnumD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opNonmutating;
import test.Ice.operations.AMD.Test.AMD_MyClass_opShortIntD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opShortIntLong;
import test.Ice.operations.AMD.Test.AMD_MyClass_opShortIntLongS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opShortIntLongSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opString;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStringMyEnumD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opMyEnumStringD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStringS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStringSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStringSSS;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStringStringD;
import test.Ice.operations.AMD.Test.AMD_MyClass_opStruct;
import test.Ice.operations.AMD.Test.AMD_MyClass_opVoid;
import test.Ice.operations.AMD.Test.AMD_MyClass_shutdown;
import test.Ice.operations.AMD.Test.AMD_MyClass_delay;
import test.Ice.operations.AMD.Test.AMD_MyDerivedClass_opDerived;
import test.Ice.operations.AMD.Test.MyClassPrx;
import test.Ice.operations.AMD.Test.MyClassPrxHelper;
import test.Ice.operations.AMD.Test.MyDerivedClass;
import test.Ice.operations.AMD.Test.MyEnum;
import test.Ice.operations.AMD.Test.MyStruct;
import test.Ice.operations.AMD.Test.Structure;

public final class AMDMyDerivedClassI extends MyDerivedClass
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
        public
        Thread_opVoid(AMD_MyClass_opVoid cb)
        {
            _cb = cb;
        }
        
        public void
        run()
        {
            _cb.ice_response();
        }

        private AMD_MyClass_opVoid _cb;
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

    public void
    delay_async(AMD_MyClass_delay cb, int ms, Ice.Current current)
    {
        try
        {
            Thread.sleep(ms);
        }
        catch(InterruptedException ex)
        {
        }
        cb.ice_response();
    }

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

    public void
    opBool_async(AMD_MyClass_opBool cb,
                 boolean p1, boolean p2,
                 Ice.Current current)
    {
        cb.ice_response(p2, p1);
    }

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

    public void
    opByte_async(AMD_MyClass_opByte cb,
                 byte p1, byte p2,
                 Ice.Current current)
    {
        cb.ice_response(p1, (byte)(p1 ^ p2));
    }

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

    public void
    opFloatDouble_async(AMD_MyClass_opFloatDouble cb,
                        float p1, double p2,
                        Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }

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

    public void
    opMyClass_async(AMD_MyClass_opMyClass cb,
                    MyClassPrx p1,
                    Ice.Current current)
    {
        MyClassPrx p2 = p1;
        MyClassPrx p3 = MyClassPrxHelper.uncheckedCast(
            current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        cb.ice_response(MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3);
    }

    public void
    opMyEnum_async(AMD_MyClass_opMyEnum cb,
                   MyEnum p1,
                   Ice.Current current)
    {
        cb.ice_response(MyEnum.enum3, p1);
    }

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

    public void
    opShortIntLong_async(AMD_MyClass_opShortIntLong cb,
                         short p1, int p2, long p3,
                         Ice.Current current)
    {
        cb.ice_response(p3, p1, p2, p3);
    }

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

    public void
    opString_async(AMD_MyClass_opString cb,
                   String p1, String p2,
                   Ice.Current current)
    {
        cb.ice_response(p1 + " " + p2, p2 + " " + p1);
    }

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

    public void
    opByteSOneway_async(AMD_MyClass_opByteSOneway cb, byte[] s, Ice.Current current)
    {
        cb.ice_response();
    }

    public void
    opContext_async(AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }

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

    public void
    opStruct_async(AMD_MyClass_opStruct cb,
                   Structure p1, Structure p2,
                   Ice.Current current)
    {
        Structure p3 = p1;
        p3.s.s = "a new string";
        cb.ice_response(p2, p3);
    }

    public void
    opIdempotent_async(AMD_MyClass_opIdempotent cb,
                       Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Idempotent);
        cb.ice_response();
    }

    public void
    opNonmutating_async(AMD_MyClass_opNonmutating cb,
                        Ice.Current current)
    {
        test(current.mode == Ice.OperationMode.Nonmutating);
        cb.ice_response();
    }

    public void
    opDerived_async(AMD_MyDerivedClass_opDerived cb,
                    Ice.Current current)
    {
        cb.ice_response();
    }

    private Thread _opVoidThread;
}
