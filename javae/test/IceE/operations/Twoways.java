// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

class Twoways
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    static void
    twoways(Ice.Communicator communicator, Ice.InitializationData initData, Test.MyClassPrx p)
    {
        {
            p.opVoid();
        }

        {
            Ice.ByteHolder b = new Ice.ByteHolder();
            byte r;

            r = p.opByte((byte)0xff, (byte)0x0f, b);
            test(b.value == (byte)0xf0);
            test(r == (byte)0xff);
        }

        {
            Ice.BooleanHolder b = new Ice.BooleanHolder();
            boolean r;

            r = p.opBool(true, false, b);
            test(b.value);
            test(!r);
        }

        {
            Ice.ShortHolder s = new Ice.ShortHolder();
            Ice.IntHolder i = new Ice.IntHolder();
            Ice.LongHolder l = new Ice.LongHolder();
            long r;

            r = p.opShortIntLong((short)10, 11, 12L, s, i, l);
            test(s.value == 10);
            test(i.value == 11);
            test(l.value == 12);
            test(r == 12L);

            r = p.opShortIntLong(Short.MIN_VALUE, Integer.MIN_VALUE,
                                 Long.MIN_VALUE, s, i, l);
            test(s.value == Short.MIN_VALUE);
            test(i.value == Integer.MIN_VALUE);
            test(l.value == Long.MIN_VALUE);
            test(r == Long.MIN_VALUE);

            r = p.opShortIntLong(Short.MAX_VALUE, Integer.MAX_VALUE,
                                 Long.MAX_VALUE, s, i, l);
            test(s.value == Short.MAX_VALUE);
            test(i.value == Integer.MAX_VALUE);
            test(l.value == Long.MAX_VALUE);
            test(r == Long.MAX_VALUE);
        }

        {
            Ice.FloatHolder f = new Ice.FloatHolder();
            Ice.DoubleHolder d = new Ice.DoubleHolder();
            double r;

            r = p.opFloatDouble(3.14f, 1.1E10, f, d);
            test(f.value == 3.14f);
            test(d.value == 1.1E10);
            test(r == 1.1E10);

            r = p.opFloatDouble(Float.MIN_VALUE, Double.MIN_VALUE, f, d);
            test(f.value == Float.MIN_VALUE);
            test(d.value == Double.MIN_VALUE);
            test(r == Double.MIN_VALUE);

            r = p.opFloatDouble(Float.MAX_VALUE, Double.MAX_VALUE, f, d);
            test(f.value == Float.MAX_VALUE);
            test(d.value == Double.MAX_VALUE);
            test(r == Double.MAX_VALUE);
        }

        {
            Ice.StringHolder s = new Ice.StringHolder();
            String r;

            r = p.opString("hello", "world", s);
            test(s.value.equals("world hello"));
            test(r.equals("hello world"));
        }

        {
            Test.MyEnumHolder e = new Test.MyEnumHolder();
            Test.MyEnum r;

            r = p.opMyEnum(Test.MyEnum.enum2, e);
            test(e.value == Test.MyEnum.enum2);
            test(r == Test.MyEnum.enum3);
        }
        {
            Test.MyClassPrxHolder c1 = new Test.MyClassPrxHolder();
            Test.MyClassPrxHolder c2 = new Test.MyClassPrxHolder();
            Test.MyClassPrx r;

            r = p.opMyClass(p, c1, c2);
            test(Ice.Util.proxyIdentityAndFacetCompare(c1.value, p) == 0);
            test(Ice.Util.proxyIdentityAndFacetCompare(c2.value, p) != 0);
            test(Ice.Util.proxyIdentityAndFacetCompare(r, p) == 0);
            test(c1.value.ice_getIdentity().equals(communicator.stringToIdentity("test")));
            test(c2.value.ice_getIdentity().equals(communicator.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().equals(communicator.stringToIdentity("test")));
            r.opVoid();
            c1.value.opVoid();
            try
            {
                c2.value.opVoid();
                test(false);
            }
            catch(Ice.ObjectNotExistException ex)
            {
            }

            r = p.opMyClass(null, c1, c2);
            test(c1.value == null);
            test(c2.value != null);
            test(Ice.Util.proxyIdentityAndFacetCompare(r, p) == 0);
            r.opVoid();
        }

        {
            Test.Structure si1 = new Test.Structure();
            si1.p = p;
            si1.e = Test.MyEnum.enum3;
            si1.s = new Test.AnotherStruct();
            si1.s.s = "abc";
            Test.Structure si2 = new Test.Structure();
            si2.p = null;
            si2.e = Test.MyEnum.enum2;
            si2.s = new Test.AnotherStruct();
            si2.s.s = "def";

            Test.StructureHolder so = new Test.StructureHolder();
            Test.Structure rso = p.opStruct(si1, si2, so);
            test(rso.p == null);
            test(rso.e == Test.MyEnum.enum2);
            test(rso.s.s.equals("def"));
            test(so.value.p.equals(p)); 
            test(so.value.e == Test.MyEnum.enum3);
            test(so.value.s.s.equals("a new string"));
            so.value.p.opVoid();
        }

        {
            final byte[] bsi1 =
            {
                (byte)0x01,
                (byte)0x11,
                (byte)0x12,
                (byte)0x22
            };
            final byte[] bsi2 =
            {
                (byte)0xf1,
                (byte)0xf2,
                (byte)0xf3,
                (byte)0xf4
            };

            Test.ByteSHolder bso = new Test.ByteSHolder();
            byte[] rso;

            rso = p.opByteS(bsi1, bsi2, bso);
            test(bso.value.length == 4);
            test(bso.value[0] == (byte)0x22);
            test(bso.value[1] == (byte)0x12);
            test(bso.value[2] == (byte)0x11);
            test(bso.value[3] == (byte)0x01);
            test(rso.length == 8);
            test(rso[0] == (byte)0x01);
            test(rso[1] == (byte)0x11);
            test(rso[2] == (byte)0x12);
            test(rso[3] == (byte)0x22);
            test(rso[4] == (byte)0xf1);
            test(rso[5] == (byte)0xf2);
            test(rso[6] == (byte)0xf3);
            test(rso[7] == (byte)0xf4);
        }

        {
            final boolean[] bsi1 = { true, true, false };
            final boolean[] bsi2 = { false };

            Test.BoolSHolder bso = new Test.BoolSHolder();
            boolean[] rso;

            rso = p.opBoolS(bsi1, bsi2, bso);
            test(bso.value.length == 4);
            test(bso.value[0]);
            test(bso.value[1]);
            test(!bso.value[2]);
            test(!bso.value[3]);
            test(rso.length == 3);
            test(!rso[0]);
            test(rso[1]);
            test(rso[2]);
        }

        {
            final short[] ssi = { 1, 2, 3 };
            final int[] isi = { 5, 6, 7, 8 };
            final long[] lsi = { 10, 30, 20 };

            Test.ShortSHolder sso = new Test.ShortSHolder();
            Test.IntSHolder iso = new Test.IntSHolder();
            Test.LongSHolder lso = new Test.LongSHolder();
            long[] rso;

            rso = p.opShortIntLongS(ssi, isi, lsi, sso, iso, lso);
            test(sso.value.length == 3);
            test(sso.value[0] == 1);
            test(sso.value[1] == 2);
            test(sso.value[2] == 3);
            test(iso.value.length == 4);
            test(iso.value[0] == 8);
            test(iso.value[1] == 7);
            test(iso.value[2] == 6);
            test(iso.value[3] == 5);
            test(lso.value.length == 6);
            test(lso.value[0] == 10);
            test(lso.value[1] == 30);
            test(lso.value[2] == 20);
            test(lso.value[3] == 10);
            test(lso.value[4] == 30);
            test(lso.value[5] == 20);
            test(rso.length == 3);
            test(rso[0] == 10);
            test(rso[1] == 30);
            test(rso[2] == 20);
        }

        {
            final float[] fsi = { 3.14f, 1.11f };
            final double[] dsi = { 1.1E10, 1.2E10, 1.3E10 };

            Test.FloatSHolder fso = new Test.FloatSHolder();
            Test.DoubleSHolder dso = new Test.DoubleSHolder();
            double[] rso;

            rso = p.opFloatDoubleS(fsi, dsi, fso, dso);
            test(fso.value.length == 2);
            test(fso.value[0] == 3.14f);
            test(fso.value[1] == 1.11f);
            test(dso.value.length == 3);
            test(dso.value[0] == 1.3E10);
            test(dso.value[1] == 1.2E10);
            test(dso.value[2] == 1.1E10);
            test(rso.length == 5);
            test(rso[0] == 1.1E10);
            test(rso[1] == 1.2E10);
            test(rso[2] == 1.3E10);
            test((float)rso[3] == 3.14f);
            test((float)rso[4] == 1.11f);
        }

        {
            final String[] ssi1 = { "abc", "de", "fghi" };
            final String[] ssi2 = { "xyz" };

            Test.StringSHolder sso = new Test.StringSHolder();
            String[] rso;

            rso = p.opStringS(ssi1, ssi2, sso);
            test(sso.value.length == 4);
            test(sso.value[0].equals("abc"));
            test(sso.value[1].equals("de"));
            test(sso.value[2].equals("fghi"));
            test(sso.value[3].equals("xyz"));
            test(rso.length == 3);
            test(rso[0].equals("fghi"));
            test(rso[1].equals("de"));
            test(rso[2].equals("abc"));
        }

        {
            final byte[][] bsi1 =
            {
                { (byte)0x01, (byte)0x11, (byte)0x12 },
                { (byte)0xff }
            };
            final byte[][] bsi2 =
            {
                { (byte)0x0e },
                { (byte)0xf2, (byte)0xf1 }
            };

            Test.ByteSSHolder bso = new Test.ByteSSHolder();
            byte[][] rso;

            rso = p.opByteSS(bsi1, bsi2, bso);
            test(bso.value.length == 2);
            test(bso.value[0].length == 1);
            test(bso.value[0][0] == (byte)0xff);
            test(bso.value[1].length == 3);
            test(bso.value[1][0] == (byte)0x01);
            test(bso.value[1][1] == (byte)0x11);
            test(bso.value[1][2] == (byte)0x12);
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
        }

        {
            final float[][] fsi =
            {
                { 3.14f },
                { 1.11f },
                { },
            };
            final double[][] dsi =
            {
                { 1.1E10, 1.2E10, 1.3E10 }
            };

            Test.FloatSSHolder fso = new Test.FloatSSHolder();
            Test.DoubleSSHolder dso = new Test.DoubleSSHolder();
            double[][] rso;

            rso = p.opFloatDoubleSS(fsi, dsi, fso, dso);
            test(fso.value.length == 3);
            test(fso.value[0].length == 1);
            test(fso.value[0][0] == 3.14f);
            test(fso.value[1].length == 1);
            test(fso.value[1][0] == 1.11f);
            test(fso.value[2].length == 0);
            test(dso.value.length == 1);
            test(dso.value[0].length == 3);
            test(dso.value[0][0] == 1.1E10);
            test(dso.value[0][1] == 1.2E10);
            test(dso.value[0][2] == 1.3E10);
            test(rso.length == 2);
            test(rso[0].length == 3);
            test(rso[0][0] == 1.1E10);
            test(rso[0][1] == 1.2E10);
            test(rso[0][2] == 1.3E10);
            test(rso[1].length == 3);
            test(rso[1][0] == 1.1E10);
            test(rso[1][1] == 1.2E10);
            test(rso[1][2] == 1.3E10);
        }

        {
            final String[][] ssi1 =
            {
                { "abc" },
                { "de", "fghi" }
            };
            final String[][] ssi2 =
            {
                { },
                { },
                { "xyz" }
            };

            Test.StringSSHolder sso = new Test.StringSSHolder();
            String[][] rso;

            rso = p.opStringSS(ssi1, ssi2, sso);
            test(sso.value.length == 5);
            test(sso.value[0].length == 1);
            test(sso.value[0][0].equals("abc"));
            test(sso.value[1].length == 2);
            test(sso.value[1][0].equals("de"));
            test(sso.value[1][1].equals("fghi"));
            test(sso.value[2].length == 0);
            test(sso.value[3].length == 0);
            test(sso.value[4].length == 1);
            test(sso.value[4][0].equals("xyz"));
            test(rso.length == 3);
            test(rso[0].length == 1);
            test(rso[0][0].equals("xyz"));
            test(rso[1].length == 0);
            test(rso[2].length == 0);
        }

	{
	    final String[][][] sssi1 =
	    {
		{
		    {
			"abc", "de"
		    },
		    {
			"xyz"
		    }
		},
		{
		    {
			"hello"
		    }
		}
	    };

	    final String[][][] sssi2 =
	    {
		{
		    {
			"", ""
		    },
		    {
			"abcd"
		    }
		},
		{
		    {
			""
		    }
		},
		{
		}
	    };

	    Test.StringSSSHolder ssso = new Test.StringSSSHolder();
	    String rsso[][][];

	    rsso = p.opStringSSS(sssi1, sssi2, ssso);
	    test(ssso.value.length == 5);
	    test(ssso.value[0].length == 2);
	    test(ssso.value[0][0].length == 2);
	    test(ssso.value[0][1].length == 1);
	    test(ssso.value[1].length == 1);
	    test(ssso.value[1][0].length == 1);
	    test(ssso.value[2].length == 2);
	    test(ssso.value[2][0].length == 2);
	    test(ssso.value[2][1].length == 1);
	    test(ssso.value[3].length == 1);
	    test(ssso.value[3][0].length == 1);
	    test(ssso.value[4].length == 0);
	    test(ssso.value[0][0][0].equals("abc"));
	    test(ssso.value[0][0][1].equals("de"));
	    test(ssso.value[0][1][0].equals("xyz"));
	    test(ssso.value[1][0][0].equals("hello"));
	    test(ssso.value[2][0][0].equals(""));
	    test(ssso.value[2][0][1].equals(""));
	    test(ssso.value[2][1][0].equals("abcd"));
	    test(ssso.value[3][0][0].equals(""));

	    test(rsso.length == 3);
	    test(rsso[0].length == 0);
	    test(rsso[1].length == 1);
	    test(rsso[1][0].length == 1);
	    test(rsso[2].length == 2);
	    test(rsso[2][0].length == 2);
	    test(rsso[2][1].length == 1);
	    test(rsso[1][0][0].equals(""));
	    test(rsso[2][0][0].equals(""));
	    test(rsso[2][0][1].equals(""));
	    test(rsso[2][1][0].equals("abcd"));
	}

        {
            java.util.Hashtable di1 = new java.util.Hashtable();
            di1.put(new Byte((byte)10), Boolean.TRUE);
            di1.put(new Byte((byte)100), Boolean.FALSE);
            java.util.Hashtable di2 = new java.util.Hashtable();
            di2.put(new Byte((byte)10), Boolean.TRUE);
            di2.put(new Byte((byte)11), Boolean.FALSE);
            di2.put(new Byte((byte)101), Boolean.TRUE);

            Test.ByteBoolDHolder _do = new Test.ByteBoolDHolder();
            java.util.Hashtable ro = p.opByteBoolD(di1, di2, _do);

            test(IceUtil.Hashtable.equals(_do.value, di1));
            test(ro.size() == 4);
            test(((Boolean)ro.get(new Byte((byte)10))).booleanValue() == true);
            test(((Boolean)ro.get(new Byte((byte)11))).booleanValue() == false);
            test(((Boolean)ro.get(new Byte((byte)100))).booleanValue() == false);
            test(((Boolean)ro.get(new Byte((byte)101))).booleanValue() == true);
        }

        {
            java.util.Hashtable di1 = new java.util.Hashtable();
            di1.put(new Short((short)110), new Integer(-1));
            di1.put(new Short((short)1100), new Integer(123123));
            java.util.Hashtable di2 = new java.util.Hashtable();
            di2.put(new Short((short)110), new Integer(-1));
            di2.put(new Short((short)111), new Integer(-100));
            di2.put(new Short((short)1101), new Integer(0));

            Test.ShortIntDHolder _do = new Test.ShortIntDHolder();
            java.util.Hashtable ro = p.opShortIntD(di1, di2, _do);

            test(IceUtil.Hashtable.equals(_do.value, di1));
            test(ro.size() == 4);
            test(((Integer)ro.get(new Short((short)110))).intValue() == -1);
            test(((Integer)ro.get(new Short((short)111))).intValue() == -100);
            test(((Integer)ro.get(new Short((short)1100))).intValue() == 123123);
            test(((Integer)ro.get(new Short((short)1101))).intValue() == 0);
        }

        {
            java.util.Hashtable di1 = new java.util.Hashtable();
            di1.put(new Long(999999110L), new Float(-1.1f));
            di1.put(new Long(999999111L), new Float(123123.2f));
            java.util.Hashtable di2 = new java.util.Hashtable();
            di2.put(new Long(999999110L), new Float(-1.1f));
            di2.put(new Long(999999120L), new Float(-100.4f));
            di2.put(new Long(999999130L), new Float(0.5f));

            Test.LongFloatDHolder _do = new Test.LongFloatDHolder();
            java.util.Hashtable ro = p.opLongFloatD(di1, di2, _do);

            test(IceUtil.Hashtable.equals(_do.value, di1));
            test(ro.size() == 4);
            test(((Float)ro.get(new Long(999999110L))).floatValue() == -1.1f);
            test(((Float)ro.get(new Long(999999120L))).floatValue() == -100.4f);
            test(((Float)ro.get(new Long(999999111L))).floatValue() == 123123.2f);
            test(((Float)ro.get(new Long(999999130L))).floatValue() == 0.5f);
        }

        {
            java.util.Hashtable di1 = new java.util.Hashtable();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            java.util.Hashtable di2 = new java.util.Hashtable();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");

            Test.StringStringDHolder _do = new Test.StringStringDHolder();
            java.util.Hashtable ro = p.opStringStringD(di1, di2, _do);

            test(IceUtil.Hashtable.equals(_do.value, di1));
            test(ro.size() == 4);
            test(((String)ro.get("foo")).equals("abc -1.1"));
            test(((String)ro.get("FOO")).equals("abc -100.4"));
            test(((String)ro.get("bar")).equals("abc 123123.2"));
            test(((String)ro.get("BAR")).equals("abc 0.5"));
        }

        {
            java.util.Hashtable di1 = new java.util.Hashtable();
            di1.put("abc", Test.MyEnum.enum1);
            di1.put("", Test.MyEnum.enum2);
            java.util.Hashtable di2 = new java.util.Hashtable();
            di2.put("abc", Test.MyEnum.enum1);
            di2.put("qwerty", Test.MyEnum.enum3);
            di2.put("Hello!!", Test.MyEnum.enum2);

            Test.StringMyEnumDHolder _do = new Test.StringMyEnumDHolder();
            java.util.Hashtable ro = p.opStringMyEnumD(di1, di2, _do);

            test(IceUtil.Hashtable.equals(_do.value, di1));
            test(ro.size() == 4);
            test(((Test.MyEnum)ro.get("abc")) == Test.MyEnum.enum1);
            test(((Test.MyEnum)ro.get("qwerty")) == Test.MyEnum.enum3);
            test(((Test.MyEnum)ro.get("")) == Test.MyEnum.enum2);
            test(((Test.MyEnum)ro.get("Hello!!")) == Test.MyEnum.enum2);
        }

        {
	    int[] lengths = { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

	    for(int l = 0; l < lengths.length; ++l)
	    {
	        int[] s = new int[lengths[l]];
		for(int i = 0; i < lengths[l]; ++i)
		{
		    s[i] = i;
		}
		int[] r = p.opIntS(s);
		test(r.length == lengths[l]);
		for(int j = 0; j < r.length; ++j)
		{
		    test(r[j] == -j);
		}
	    }
        }

	{
	    java.util.Hashtable ctx = new java.util.Hashtable();
	    ctx.put("one", "ONE");
	    ctx.put("two", "TWO");
	    ctx.put("three", "THREE");
	    {
		test(p.ice_getContext().isEmpty());
		java.util.Hashtable r = p.opContext();
		test(!IceUtil.Hashtable.equals(r, ctx));
	    }
	    {
		java.util.Hashtable r = p.opContext(ctx);
		test(p.ice_getContext().isEmpty());
		test(IceUtil.Hashtable.equals(r, ctx));
	    }
	    {
		Test.MyClassPrx p2 = Test.MyClassPrxHelper.checkedCast(p.ice_context(ctx));
		test(IceUtil.Hashtable.equals(p2.ice_getContext(), ctx));
		java.util.Hashtable r = p2.opContext();
		test(IceUtil.Hashtable.equals(r, ctx));
		r = p2.opContext(ctx);
		test(IceUtil.Hashtable.equals(r, ctx));
	    }
	    {
		//
		// Test proxy contexts
		String ref = communicator.getProperties().getPropertyWithDefault("Test.Proxy", 
		    "test:default -p 12010 -t 10000");
		Test.MyClassPrx c = Test.MyClassPrxHelper.checkedCast(communicator.stringToProxy(ref));

		java.util.Hashtable dflt = new java.util.Hashtable();
		dflt.put("a", "b");
		Test.MyClassPrx c2 = Test.MyClassPrxHelper.uncheckedCast(c.ice_context(dflt));
		test(c2.opContext().get("a").equals("b"));

		dflt.clear();
		Test.MyClassPrx c3 = Test.MyClassPrxHelper.uncheckedCast(c2.ice_context(dflt));
		test(c3.opContext().get("a") == null);
	    }
	}
	
	{
	    double d = 1278312346.0 / 13.0;
	    double[] ds = new double[5];
	    for(int i = 0; i < 5; i++)
	    {
		ds[i] = d;
	    }
	    p.opDoubleMarshaling(d, ds);
	}
    }
}
