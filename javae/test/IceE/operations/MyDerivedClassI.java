// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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

    public
    MyDerivedClassI()
    {
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    public void
    opVoid(Ice.Current current)
    {
    }

    public void
    opSleep(int duration, Ice.Current current)
    {
	while(true)
	{
	    try
	    {
		Thread.currentThread().sleep(duration);
		break;
	    }
	    catch(java.lang.InterruptedException ex)
	    {
	    }
	}
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
	    Test.BoolSHolder p3,
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
	     Test.BoolSSHolder p3,
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

    protected void
    copyHashtable(java.util.Hashtable from, java.util.Hashtable to)
    {
	java.util.Enumeration e = from.keys();
	while(e.hasMoreElements())
	{
	    java.lang.Object key = e.nextElement();
	    to.put(key, from.get(key));
	}
    }

    public java.util.Hashtable
    opByteBoolD(java.util.Hashtable p1, java.util.Hashtable p2,
		Test.ByteBoolDHolder p3,
                Ice.Current current)
    {
        p3.value = p1;
        java.util.Hashtable r = new java.util.Hashtable();
	copyHashtable(p1, r);
	copyHashtable(p2, r);
        return r;
    }

    public byte[]
    opByteS(byte[] p1, byte[] p2,
	    Test.ByteSHolder p3,
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
	     Test.ByteSSHolder p3,
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
		   Test.FloatSHolder p3, Test.DoubleSHolder p4,
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
		    Test.FloatSSHolder p3, Test.DoubleSSHolder p4,
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

    public java.util.Hashtable
    opLongFloatD(java.util.Hashtable p1, java.util.Hashtable p2,
		 Test.LongFloatDHolder p3,
                 Ice.Current current)
    {
        p3.value = p1;
        java.util.Hashtable r = new java.util.Hashtable();
	copyHashtable(p1, r);
	copyHashtable(p2, r);
        return r;
    }

    public Test.MyClassPrx
    opMyClass(Test.MyClassPrx p1,
	      Test.MyClassPrxHolder p2, Test.MyClassPrxHolder p3,
	      Ice.Current current)
    {
        p2.value = p1;
        p3.value = Test.MyClassPrxHelper.uncheckedCast(
	    current.adapter.createProxy(current.adapter.getCommunicator().stringToIdentity("noSuchIdentity")));
        return Test.MyClassPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
    }

    public Test.MyEnum
    opMyEnum(Test.MyEnum p1,
	     Test.MyEnumHolder p2,
	     Ice.Current current)
    {
        p2.value = p1;
        return Test.MyEnum.enum3;
    }

    public java.util.Hashtable
    opShortIntD(java.util.Hashtable p1, java.util.Hashtable p2,
		Test.ShortIntDHolder p3,
                Ice.Current current)
    {
        p3.value = p1;
        java.util.Hashtable r = new java.util.Hashtable();
	copyHashtable(p1, r);
	copyHashtable(p2, r);
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
		    Test.ShortSHolder p4, Test.IntSHolder p5, Test.LongSHolder p6,
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
                     Test.ShortSSHolder p4, Test.IntSSHolder p5, Test.LongSSHolder p6,
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

    public java.util.Hashtable
    opStringMyEnumD(java.util.Hashtable p1, java.util.Hashtable p2,
                    Test.StringMyEnumDHolder p3,
		    Ice.Current current)
    {
        p3.value = p1;
        java.util.Hashtable r = new java.util.Hashtable();
	copyHashtable(p1, r);
	copyHashtable(p2, r);
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

    public java.util.Hashtable
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
	      Test.StringSHolder p3,
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
	       Test.StringSSHolder p3,
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
	       Test.StringSSSHolder p3,
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

    public java.util.Hashtable
    opStringStringD(java.util.Hashtable p1, java.util.Hashtable p2,
                    Test.StringStringDHolder p3,
		    Ice.Current current)
    {
        p3.value = p1;
        java.util.Hashtable r = new java.util.Hashtable();
	copyHashtable(p1, r);
	copyHashtable(p2, r);
        return r;
    }

    public Test.Structure
    opStruct(Test.Structure p1, Test.Structure p2,
	     Test.StructureHolder p3,
             Ice.Current current)
    {
        p3.value = p1;
        p3.value.s.s = "a new string";
        return p2;
    }

    public void
    opDerived(Ice.Current current)
    {
    }
}
