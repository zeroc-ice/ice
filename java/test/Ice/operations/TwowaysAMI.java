// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class TwowaysAMI
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static class Callback
    {
	Callback()
	{
	    _called = false;
	}

	public synchronized boolean
	check()
	{
	    while(!_called)
	    {
		try
		{
		    wait(5000);
		}
		catch(InterruptedException ex)
		{
		    continue;
		}

		if(!_called)
		{
		    return false; // Must be timeout.
		}
	    }

	    return true;
	}
	
	public synchronized void
	called()
	{
	    assert(!_called);
	    _called = true;
	    notify();
	}

	private boolean _called;
    };

    static class AMI_MyClass_opVoidI extends Test.AMI_MyClass_opVoid
    {
	public void
	ice_response()
	{
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opByteI extends Test.AMI_MyClass_opByte
    {
	public void
	ice_response(byte r, byte b)
	{
	    test(b == (byte)0xf0);
	    test(r == (byte)0xff);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opBoolI extends Test.AMI_MyClass_opBool
    {
	public void
	ice_response(boolean r, boolean b)
	{
	    test(b);
	    test(!r);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opShortIntLongI extends Test.AMI_MyClass_opShortIntLong
    {
	public void
	ice_response(long r, short s, int i, long l)
	{
	    test(s == 10);
	    test(i == 11);
	    test(l == 12);
	    test(r == 12);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opFloatDoubleI extends Test.AMI_MyClass_opFloatDouble
    {
	public void
	ice_response(double r, float f, double d)
	{
	    test(f == 3.14f);
	    test(d == 1.1E10);
	    test(r == 1.1E10);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStringI extends Test.AMI_MyClass_opString
    {
	public void
	ice_response(String r, String s)
	{
	    test(s == "world hello");
	    test(r == "hello world");
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opMyEnumI extends Test.AMI_MyClass_opMyEnum
    {
	public void
	ice_response(Test.MyEnum r, Test.MyEnum e)
	{
            test(e == Test.MyEnum.enum2);
            test(r == Test.MyEnum.enum3);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opMyClassI extends Test.AMI_MyClass_opMyClass
    {
	public void
	ice_response(Test.MyClassPrx r, Test.MyClassPrx c1, Test.MyClassPrx c2)
	{
	    test(c1.ice_getIdentity() == Ice.Util.stringToIdentity("test"));
	    test(c2.ice_getIdentity() == Ice.Util.stringToIdentity("noSuchIdentity"));
	    test(r.ice_getIdentity() == Ice.Util.stringToIdentity("test"));
	    r.opVoid();
	    c1.opVoid();
	    try
	    {
		c2.opVoid();
		test(false);
	    }
	    catch(Ice.ObjectNotExistException ex)
	    {
	    }
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStructI extends Test.AMI_MyClass_opStruct
    {
	public void
	ice_response(Test.Structure rso, Test.Structure so)
	{
	    test(rso.p == null);
	    test(rso.e == Test.MyEnum.enum2);
	    test(rso.s.s == "def");
	    test(so.e == Test.MyEnum.enum3);
	    test(so.s.s == "a new string");
	    so.p.opVoid();
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opByteSI extends Test.AMI_MyClass_opByteS
    {
	public void
	ice_response(byte[] rso, byte[] bso)
	{
	    test(bso.length == 4);
	    test(bso[0] == (byte)0x22);
	    test(bso[1] == (byte)0x12);
	    test(bso[2] == (byte)0x11);
	    test(bso[3] == (byte)0x01);
	    test(rso.length == 8);
	    test(rso[0] == (byte)0x01);
	    test(rso[1] == (byte)0x11);
	    test(rso[2] == (byte)0x12);
	    test(rso[3] == (byte)0x22);
	    test(rso[4] == (byte)0xf1);
	    test(rso[5] == (byte)0xf2);
	    test(rso[6] == (byte)0xf3);
	    test(rso[7] == (byte)0xf4);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opBoolSI extends Test.AMI_MyClass_opBoolS
    {
	public void
	ice_response(boolean[] rso, boolean[] bso)
	{
	    test(bso.length == 4);
	    test(bso[0]);
	    test(bso[1]);
	    test(!bso[2]);
	    test(!bso[3]);
	    test(rso.length == 3);
	    test(!rso[0]);
	    test(rso[1]);
	    test(rso[2]);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opShortIntLongSI extends Test.AMI_MyClass_opShortIntLongS
    {
	public void
	ice_response(long[] rso, short[] sso, int[] iso,
				 long[] lso)
	{
	    test(sso.length == 3);
	    test(sso[0] == 1);
	    test(sso[1] == 2);
	    test(sso[2] == 3);
	    test(iso.length == 4);
	    test(iso[0] == 8);
	    test(iso[1] == 7);
	    test(iso[2] == 6);
	    test(iso[3] == 5);
	    test(lso.length == 6);
	    test(lso[0] == 10);
	    test(lso[1] == 30);
	    test(lso[2] == 20);
	    test(lso[3] == 10);
	    test(lso[4] == 30);
	    test(lso[5] == 20);
	    test(rso.length == 3);
	    test(rso[0] == 10);
	    test(rso[1] == 30);
	    test(rso[2] == 20);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opFloatDoubleSI extends Test.AMI_MyClass_opFloatDoubleS
    {
	public void
	ice_response(double[] rso, float[] fso, double[] dso)
	{
	    test(fso.length == 2);
	    test(fso[0] == 3.14f);
	    test(fso[1] == 1.11f);
	    test(dso.length == 3);
	    test(dso[0] == 1.3E10);
	    test(dso[1] == 1.2E10);
	    test(dso[2] == 1.1E10);
	    test(rso.length == 5);
	    test(rso[0] == 1.1E10);
	    test(rso[1] == 1.2E10);
	    test(rso[2] == 1.3E10);
	    test(rso[3] == 3.14);
	    test(rso[4] == 1.11);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStringSI extends Test.AMI_MyClass_opStringS
    {
	public void
	ice_response(String[] rso, String[] sso)
	{
	    test(sso.length == 4);
	    test(sso[0] == "abc");
	    test(sso[1] == "de");
	    test(sso[2] == "fghi");
	    test(sso[3] == "xyz");
	    test(rso.length == 3);
	    test(rso[0] == "fghi");
	    test(rso[1] == "de");
	    test(rso[2] == "abc");
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opByteSSI extends Test.AMI_MyClass_opByteSS
    {
	public void
	ice_response(byte[][] rso, byte[][] bso)
	{
	    test(bso.length == 2);
	    test(bso[0].length == 1);
	    test(bso[0][0] == (byte)0xff);
	    test(bso[1].length == 3);
	    test(bso[1][0] == (byte)0x01);
	    test(bso[1][1] == (byte)0x11);
	    test(bso[1][2] == (byte)0x12);
	    test(rso.length == 4);
	    test(rso[0].length == 3);
	    test(rso[0][0] == (byte)0x01);
	    test(rso[0][1] == (byte)0x11);
	    test(rso[0][2] == (byte)0x12);
	    test(rso[1].length == 1);
	    test(rso[1][0] == (byte)0xff);
	    test(rso[2].length == 1);
	    test(rso[2][0] == (byte)0x0e);
	    test(rso[3].length == 2);
	    test(rso[3][0] == (byte)0xf2);
	    test(rso[3][1] == (byte)0xf1);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opBoolSSI extends Test.AMI_MyClass_opBoolSS
    {
	public void
	ice_response(boolean[][] rso, boolean[][] bso)
	{
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opShortIntLongSSI extends Test.AMI_MyClass_opShortIntLongSS
    {
	public void
	ice_response(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
	{
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opFloatDoubleSSI extends Test.AMI_MyClass_opFloatDoubleSS
    {
	public void
	ice_response(double[][] rso, float[][] fso, double[][] dso)
	{
	    test(fso.length == 3);
	    test(fso[0].length == 1);
	    test(fso[0][0] == 3.14f);
	    test(fso[1].length == 1);
	    test(fso[1][0] == 1.11f);
	    test(fso[2].length == 0);
	    test(dso.length == 1);
	    test(dso[0].length == 3);
	    test(dso[0][0] == 1.1E10);
	    test(dso[0][1] == 1.2E10);
	    test(dso[0][2] == 1.3E10);
	    test(rso.length == 2);
	    test(rso[0].length == 3);
	    test(rso[0][0] == 1.1E10);
	    test(rso[0][1] == 1.2E10);
	    test(rso[0][2] == 1.3E10);
	    test(rso[1].length == 3);
	    test(rso[1][0] == 1.1E10);
	    test(rso[1][1] == 1.2E10);
	    test(rso[1][2] == 1.3E10);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStringSSI extends Test.AMI_MyClass_opStringSS
    {
	public void
	ice_response(String[][] rso, String[][] sso)
	{
	    test(sso.length == 5);
	    test(sso[0].length == 1);
	    test(sso[0][0] == "abc");
	    test(sso[1].length == 2);
	    test(sso[1][0] == "de");
	    test(sso[1][1] == "fghi");
	    test(sso[2].length == 0);
	    test(sso[3].length == 0);
	    test(sso[4].length == 1);
	    test(sso[4][0] == "xyz");
	    test(rso.length == 3);
	    test(rso[0].length == 1);
	    test(rso[0][0] == "xyz");
	    test(rso[1].length == 0);
	    test(rso[2].length == 0);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

/*
    static class AMI_MyClass_opByteBoolDI extends Test.AMI_MyClass_opByteBoolD
    {
	public void
	ice_response(Test.ByteBoolD ro, Test.ByteBoolD _do)
	{
	    Test.ByteBoolD di1;
	    di1[10] = true;
	    di1[100] = false;
	    test(_do == di1);
	    test(ro.length == 4);
	    test(ro.find(10) != ro.end());
	    test(ro.find(10).second == true);
	    test(ro.find(11) != ro.end());
	    test(ro.find(11).second == false);
	    test(ro.find(100) != ro.end());
	    test(ro.find(100).second == false);
	    test(ro.find(101) != ro.end());
	    test(ro.find(101).second == true);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	return callback.callcheck();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opShortIntDI extends Test.AMI_MyClass_opShortIntD
    {
	public void
	ice_response(Test.ShortIntD ro, Test.ShortIntD _do)
	{
	    Test.ShortIntD di1;
	    di1[110] = -1;
	    di1[1100] = 123123;
	    test(_do == di1);
	    test(ro.length == 4);
	    test(ro.find(110) != ro.end());
	    test(ro.find(110).second == -1);
	    test(ro.find(111) != ro.end());
	    test(ro.find(111).second == -100);
	    test(ro.find(1100) != ro.end());
	    test(ro.find(1100).second == 123123);
	    test(ro.find(1101) != ro.end());
	    test(ro.find(1101).second == 0);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	return callback.callcheck();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opLongFloatDI extends Test.AMI_MyClass_opLongFloatD
    {
	public void
	ice_response(Test.LongFloatD ro, Test.LongFloatD _do)
	{
	    Test.LongFloatD di1;
	    di1[999999110] = -1.1f;
	    di1[9999991100] = 123123.2f;
	    test(_do == di1);
	    test(ro.length == 4);
	    test(ro.find(999999110) != ro.end());
	    test(ro.find(999999110).second == -1.1f);
	    test(ro.find(999999111) != ro.end());
	    test(ro.find(999999111).second == -100.4f);
	    test(ro.find(9999991100) != ro.end());
	    test(ro.find(9999991100).second == 123123.2f);
	    test(ro.find(9999991101) != ro.end());
	    test(ro.find(9999991101).second == 0.5f);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	return callback.callcheck();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStringStringDI extends Test.AMI_MyClass_opStringStringD
    {
	public void
	ice_response(String[]tringD ro, String[]tringD _do)
	{
	    String[]tringD di1;
	    di1["foo"] = "abc -1.1";
	    di1["bar"] = "abc 123123.2";
	    test(_do == di1);
	    test(ro.length == 4);
	    test(ro.find("foo") != ro.end());
	    test(ro.find("foo").second == "abc -1.1");
	    test(ro.find("FOO") != ro.end());
	    test(ro.find("FOO").second == "abc -100.4");
	    test(ro.find("bar") != ro.end());
	    test(ro.find("bar").second == "abc 123123.2");
	    test(ro.find("BAR") != ro.end());
	    test(ro.find("BAR").second == "abc 0.5");
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	return callback.callcheck();
	}

	private Callback callback = new Callback();
    };

    static class AMI_MyClass_opStringMyEnumDI extends Test.AMI_MyClass_opStringMyEnumD
    {
	public void
	ice_response(Test.StringMyEnumD ro, Test.StringMyEnumD _do)
	{
	    Test.StringMyEnumD di1;
	    di1["abc"] = Test.enum1;
	    di1[""] = Test.enum2;
	    test(_do == di1);
	    test(ro.length == 4);
	    test(ro.find("abc") != ro.end());
	    test(ro.find("abc").second == Test.enum1);
	    test(ro.find("qwerty") != ro.end());
	    test(ro.find("qwerty").second == Test.enum3);
	    test(ro.find("") != ro.end());
	    test(ro.find("").second == Test.enum2);
	    test(ro.find("Hello!!") != ro.end());
	    test(ro.find("Hello!!").second == Test.enum2);
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	return callback.callcheck();
	}

	private Callback callback = new Callback();
    };
*/

    static class AMI_MyDerivedClass_opDerivedI extends Test.AMI_MyDerivedClass_opDerived
    {
	public void
	ice_response()
	{
	    callback.called();
	}

	public void
	ice_exception(Ice.LocalException ex)
	{
	    test(false);
	}

	public boolean
	check()
	{
	    return callback.check();
	}

	private Callback callback = new Callback();
    };

    static void
    twowaysAMI(Test.MyClassPrx p)
    {
	{
	    AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
	    p.opVoid_async(cb);
	    test(cb.check());
	}
	
/*
	{
	    AMI_MyClass_opByteI cb = new AMI_MyClass_opByteI();
	    p.opByte_async(cb, (byte)0xff, (byte)0x0f);
	    test(cb.check());
	}

    {
	AMI_MyClass_opBoolI cb = new AMI_MyClass_opBoolI();
	p.opBool_async(cb, true, false);
	test(cb.check());
    }

    {
	AMI_MyClass_opShortIntLongI cb = new AMI_MyClass_opShortIntLongI();
	p.opShortIntLong_async(cb, 10, 11, 12);
	test(cb.check());
    }

    {
	AMI_MyClass_opFloatDoubleI cb = new AMI_MyClass_opFloatDoubleI();
	p.opFloatDouble_async(cb, Ice::Float(3.14), Ice::Double(1.1E10));
	test(cb.check());
    }

    {
	AMI_MyClass_opStringI cb = new AMI_MyClass_opStringI();
	p.opString_async(cb, "hello", "world");
	test(cb.check());
    }

    {
	AMI_MyClass_opMyEnumI cb = new AMI_MyClass_opMyEnumI();
	p.opMyEnum_async(cb, Test::enum2);
	test(cb.check());
    }

    {
	AMI_MyClass_opMyClassI cb = new AMI_MyClass_opMyClassI();
	p.opMyClass_async(cb, p);
	test(cb.check());
    }

    {
	Test::Structure si1;
	si1.p = p;
	si1.e = Test::enum3;
	si1.s.s = "abc";
	Test::Structure si2;
	si2.p = 0;
	si2.e = Test::enum2;
	si2.s.s = "def";
	
	AMI_MyClass_opStructI cb = new AMI_MyClass_opStructI();
	p.opStruct_async(cb, si1, si2);
	test(cb.check());
    }

    {
	Test::ByteS bsi1;
	Test::ByteS bsi2;

	bsi1.push_back((byte)0x01);
	bsi1.push_back((byte)0x11);
	bsi1.push_back((byte)0x12);
	bsi1.push_back((byte)0x22);

	bsi2.push_back((byte)0xf1);
	bsi2.push_back((byte)0xf2);
	bsi2.push_back((byte)0xf3);
	bsi2.push_back((byte)0xf4);

	AMI_MyClass_opByteSI cb = new AMI_MyClass_opByteSI();
	p.opByteS_async(cb, bsi1, bsi2);
	test(cb.check());
    }

    {
	Test::BoolS bsi1;
	Test::BoolS bsi2;

	bsi1.push_back(true);
	bsi1.push_back(true);
	bsi1.push_back(false);

	bsi2.push_back(false);

	AMI_MyClass_opBoolSI cb = new AMI_MyClass_opBoolSI();
	p.opBoolS_async(cb, bsi1, bsi2);
	test(cb.check());
    }

    {
	Test::ShortS ssi;
	Test::IntS isi;
	Test::LongS lsi;

	ssi.push_back(1);
	ssi.push_back(2);
	ssi.push_back(3);

	isi.push_back(5);
	isi.push_back(6);
	isi.push_back(7);
	isi.push_back(8);

	lsi.push_back(10);
	lsi.push_back(30);
	lsi.push_back(20);

	AMI_MyClass_opShortIntLongSI cb = new AMI_MyClass_opShortIntLongSI();
	p.opShortIntLongS_async(cb, ssi, isi, lsi);
	test(cb.check());
    }

    {
	Test::FloatS fsi;
	Test::DoubleS dsi;

	fsi.push_back(Ice::Float(3.14));
	fsi.push_back(Ice::Float(1.11));

	dsi.push_back(Ice::Double(1.1E10));
	dsi.push_back(Ice::Double(1.2E10));
	dsi.push_back(Ice::Double(1.3E10));

	AMI_MyClass_opFloatDoubleSI cb = new AMI_MyClass_opFloatDoubleSI();
	p.opFloatDoubleS_async(cb, fsi, dsi);
	test(cb.check());
    }

    {
	Test::StringS ssi1;
	Test::StringS ssi2;

	ssi1.push_back("abc");
	ssi1.push_back("de");
	ssi1.push_back("fghi");

	ssi2.push_back("xyz");

	AMI_MyClass_opStringSI cb = new AMI_MyClass_opStringSI();
	p.opStringS_async(cb, ssi1, ssi2);
	test(cb.check());
    }

    {
	Test::ByteSS bsi1;
	bsi1.resize(2);
	Test::ByteSS bsi2;
	bsi2.resize(2);

	bsi1[0].push_back((byte)0x01);
	bsi1[0].push_back((byte)0x11);
	bsi1[0].push_back((byte)0x12);
	bsi1[1].push_back((byte)0xff);

	bsi2[0].push_back((byte)0x0e);
	bsi2[1].push_back((byte)0xf2);
	bsi2[1].push_back((byte)0xf1);

	AMI_MyClass_opByteSSI cb = new AMI_MyClass_opByteSSI();
	p.opByteSS_async(cb, bsi1, bsi2);
	test(cb.check());
    }

    {
	Test::FloatSS fsi;
	fsi.resize(3);
	Test::DoubleSS dsi;
	dsi.resize(1);

	fsi[0].push_back(Ice::Float(3.14));
	fsi[1].push_back(Ice::Float(1.11));

	dsi[0].push_back(Ice::Double(1.1E10));
	dsi[0].push_back(Ice::Double(1.2E10));
	dsi[0].push_back(Ice::Double(1.3E10));

	AMI_MyClass_opFloatDoubleSSI cb = new AMI_MyClass_opFloatDoubleSSI();
	p.opFloatDoubleSS_async(cb, fsi, dsi);
	test(cb.check());
    }

    {
	Test::StringSS ssi1;
	ssi1.resize(2);
	Test::StringSS ssi2;
	ssi2.resize(3);

	ssi1[0].push_back("abc");
	ssi1[1].push_back("de");
	ssi1[1].push_back("fghi");

	ssi2[2].push_back("xyz");

	AMI_MyClass_opStringSSI cb = new AMI_MyClass_opStringSSI();
	p.opStringSS_async(cb, ssi1, ssi2);
	test(cb.check());
    }

    {
	Test::ByteBoolD di1;
	di1[10] = true;
	di1[100] = false;
	Test::ByteBoolD di2;
	di2[10] = true;
	di2[11] = false;
	di2[101] = true;

	AMI_MyClass_opByteBoolDI cb = new AMI_MyClass_opByteBoolDI();
	p.opByteBoolD_async(cb, di1, di2);
	test(cb.check());
    }

    {
	Test::ShortIntD di1;
	di1[110] = -1;
	di1[1100] = 123123;
	Test::ShortIntD di2;
	di2[110] = -1;
	di2[111] = -100;
	di2[1101] = 0;

	AMI_MyClass_opShortIntDI cb = new AMI_MyClass_opShortIntDI();
	p.opShortIntD_async(cb, di1, di2);
	test(cb.check());
    }

    {
	Test::LongFloatD di1;
	di1[999999110] = Ice::Float(-1.1);
	di1[9999991100] = Ice::Float(123123.2);
	Test::LongFloatD di2;
	di2[999999110] = Ice::Float(-1.1);
	di2[999999111] = Ice::Float(-100.4);
	di2[9999991101] = Ice::Float(0.5);

	AMI_MyClass_opLongFloatDI cb = new AMI_MyClass_opLongFloatDI();
	p.opLongFloatD_async(cb, di1, di2);
	test(cb.check());
	}

    {
	Test::StringStringD di1;
	di1["foo"] = "abc -1.1";
	di1["bar"] = "abc 123123.2";
	Test::StringStringD di2;
	di2["foo"] = "abc -1.1";
	di2["FOO"] = "abc -100.4";
	di2["BAR"] = "abc 0.5";

	AMI_MyClass_opStringStringDI cb = new AMI_MyClass_opStringStringDI();
	p.opStringStringD_async(cb, di1, di2);
	test(cb.check());
    }

    {
	Test::StringMyEnumD di1;
	di1["abc"] = Test::enum1;
	di1[""] = Test::enum2;
	Test::StringMyEnumD di2;
	di2["abc"] = Test::enum1;
	di2["qwerty"] = Test::enum3;
	di2["Hello!!"] = Test::enum2;

	AMI_MyClass_opStringMyEnumDI cb = new AMI_MyClass_opStringMyEnumDI();
	p.opStringMyEnumD_async(cb, di1, di2);
	test(cb.check());
    }

    {
        Test.MyDerivedClassPrx derived = Test.MyDerivedClassPrxHelper.checkedCast(p);
	test(derived != null);
	AMI_MyDerivedClass_opDerivedI cb = new AMI_MyDerivedClass_opDerivedI();
	derived.opDerived_async(cb);
	test(cb.check());
    }
    }
*/
    }
};
