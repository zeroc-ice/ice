// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public final class MyDerivedClassI extends Test.MyDerivedClass
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
        Thread_opVoid(Test.AMD_MyClass_opVoid cb)
        {
            _cb = cb;
        }
        
        public void
        run()
        {
            _cb.ice_response();
        }

        private Test.AMD_MyClass_opVoid _cb;
    }

    public void
    shutdown_async(Test.AMD_MyClass_shutdown cb,
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
    opVoid_async(Test.AMD_MyClass_opVoid cb,
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

        _opVoidThread = new Thread_opVoid(cb);
        _opVoidThread.start();
    }

    public void
    opBool_async(Test.AMD_MyClass_opBool cb,
                 boolean p1, boolean p2,
                 Ice.Current current)
    {
        cb.ice_response(p2, p1);
    }

    public void
    opBoolS_async(Test.AMD_MyClass_opBoolS cb,
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
    opBoolSS_async(Test.AMD_MyClass_opBoolSS cb,
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
    opByte_async(Test.AMD_MyClass_opByte cb,
                 byte p1, byte p2,
                 Ice.Current current)
    {
        cb.ice_response(p1, (byte)(p1 ^ p2));
    }

    public void
    opByteBoolD_async(Test.AMD_MyClass_opByteBoolD cb,
                      java.util.Map p1, java.util.Map p2,
                      Ice.Current current)
    {
        java.util.Map p3 = p1;
        java.util.Map r = new java.util.HashMap();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    public void
    opByteS_async(Test.AMD_MyClass_opByteS cb,
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
    opByteSS_async(Test.AMD_MyClass_opByteSS cb,
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
    opFloatDouble_async(Test.AMD_MyClass_opFloatDouble cb,
                        float p1, double p2,
                        Ice.Current current)
    {
        cb.ice_response(p2, p1, p2);
    }

    public void
    opFloatDoubleS_async(Test.AMD_MyClass_opFloatDoubleS cb,
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
    opFloatDoubleSS_async(Test.AMD_MyClass_opFloatDoubleSS cb,
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
    opLongFloatD_async(Test.AMD_MyClass_opLongFloatD cb,
                       java.util.Map p1, java.util.Map p2,
                       Ice.Current current)
    {
        java.util.Map p3 = p1;
        java.util.Map r = new java.util.HashMap();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    public void
    opMyClass_async(Test.AMD_MyClass_opMyClass cb,
                    Test.MyClassPrx p1,
                    Ice.Current current)
    {
        Test.MyClassPrx p2 = p1;
        Test.MyClassPrx p3 = Test.MyClassPrxHelper.uncheckedCast(
            current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        cb.ice_response(Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id)), p2, p3);
    }

    public void
    opMyEnum_async(Test.AMD_MyClass_opMyEnum cb,
                   Test.MyEnum p1,
                   Ice.Current current)
    {
        cb.ice_response(Test.MyEnum.enum3, p1);
    }

    public void
    opShortIntD_async(Test.AMD_MyClass_opShortIntD cb,
                      java.util.Map p1, java.util.Map p2,
                      Ice.Current current)
    {
        java.util.Map p3 = p1;
        java.util.Map r = new java.util.HashMap();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    public void
    opShortIntLong_async(Test.AMD_MyClass_opShortIntLong cb,
                         short p1, int p2, long p3,
                         Ice.Current current)
    {
        cb.ice_response(p3, p1, p2, p3);
    }

    public void
    opShortIntLongS_async(Test.AMD_MyClass_opShortIntLongS cb,
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
    opShortIntLongSS_async(Test.AMD_MyClass_opShortIntLongSS cb,
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
    opString_async(Test.AMD_MyClass_opString cb,
                   String p1, String p2,
                   Ice.Current current)
    {
        cb.ice_response(p1 + " " + p2, p2 + " " + p1);
    }

    public void
    opStringMyEnumD_async(Test.AMD_MyClass_opStringMyEnumD cb,
                          java.util.Map p1, java.util.Map p2,
                          Ice.Current current)
    {
        java.util.Map p3 = p1;
        java.util.Map r = new java.util.HashMap();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    public void
    opIntS_async(Test.AMD_MyClass_opIntS cb, int[] s, Ice.Current current)
    {
        int[] r = new int[s.length];
        for(int i = 0; i < r.length; ++i)
        {
            r[i] = -s[i];
        }
        cb.ice_response(r);
    }

    public void
    opByteSOneway_async(Test.AMD_MyClass_opByteSOneway cb, byte[] s, Ice.Current current)
    {
        cb.ice_response();
    }

    public void
    opContext_async(Test.AMD_MyClass_opContext cb, Ice.Current current)
    {
        cb.ice_response(current.ctx);
    }

    public void
    opDoubleMarshaling_async(Test.AMD_MyClass_opDoubleMarshaling cb, double p1, double[] p2, Ice.Current current)
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
    opStringS_async(Test.AMD_MyClass_opStringS cb,
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
    opStringSS_async(Test.AMD_MyClass_opStringSS cb,
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
    opStringSSS_async(Test.AMD_MyClass_opStringSSS cb,
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
    opStringStringD_async(Test.AMD_MyClass_opStringStringD cb,
                          java.util.Map p1, java.util.Map p2,
                          Ice.Current current)
    {
        java.util.Map p3 = p1;
        java.util.Map r = new java.util.HashMap();
        r.putAll(p1);
        r.putAll(p2);
        cb.ice_response(r, p3);
    }

    public void
    opStruct_async(Test.AMD_MyClass_opStruct cb,
                   Test.Structure p1, Test.Structure p2,
                   Ice.Current current)
    {
        Test.Structure p3 = p1;
        p3.s.s = "a new string";
        cb.ice_response(p2, p3);
    }

    public void
    opDerived_async(Test.AMD_MyDerivedClass_opDerived cb,
                    Ice.Current current)
    {
        cb.ice_response();
    }

    private Thread _opVoidThread;
}
