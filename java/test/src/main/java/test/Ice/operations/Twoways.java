// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import Ice.*;
import Ice.Object;
import test.Ice.operations.Test.*;
import test.Util.Application;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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

    static class PerThreadContextInvokeThread extends Thread
    {
        public PerThreadContextInvokeThread(MyClassPrx proxy)
        {
            _proxy = proxy;
        }

        @Override
        public void
        run()
        {
            java.util.Map<String, String> ctx = _proxy.ice_getCommunicator().getImplicitContext().getContext();
            test(ctx.isEmpty());
            ctx.put("one", "ONE");
            _proxy.ice_getCommunicator().getImplicitContext().setContext(ctx);
            test(_proxy.opContext().equals(ctx));
        }

        final private MyClassPrx _proxy;
    }

    static void
    twoways(Application app, MyClassPrx p)
    {
        Communicator communicator = app.communicator();
        
        String[] literals = p.opStringLiterals();
        
        test(s0.value.equals("\\") &&
             s0.value.equals(sw0.value) &&
             s0.value.equals(literals[0]) &&
             s0.value.equals(literals[11]));

        test(s1.value.equals("A") &&
             s1.value.equals(sw1.value) &&
             s1.value.equals(literals[1]) &&
             s1.value.equals(literals[12]));

        test(s2.value.equals("Ice") &&
             s2.value.equals(sw2.value) &&
             s2.value.equals(literals[2]) &&
             s2.value.equals(literals[13]));
        
        test(s3.value.equals("A21") && 
             s3.value.equals(sw3.value) &&
             s3.value.equals(literals[3]) &&
             s3.value.equals(literals[14]));

        test(s4.value.equals("\\u0041 \\U00000041") &&
             s4.value.equals(sw4.value) &&
             s4.value.equals(literals[4]) &&
             s4.value.equals(literals[15]));

        test(s5.value.equals("\u00FF") &&
             s5.value.equals(sw5.value) &&
             s5.value.equals(literals[5]) &&
             s5.value.equals(literals[16]));

        test(s6.value.equals("\u03FF") &&
             s6.value.equals(sw6.value) &&
             s6.value.equals(literals[6]) &&
             s6.value.equals(literals[17]));

        test(s7.value.equals("\u05F0") &&
             s7.value.equals(sw7.value) &&
             s7.value.equals(literals[7]) &&
             s7.value.equals(literals[18]));

        test(s8.value.equals("\uD800\uDC00") &&
             s8.value.equals(sw8.value) &&
             s8.value.equals(literals[8]) &&
             s8.value.equals(literals[19]));
        
        test(s9.value.equals("\uD83C\uDF4C") &&
             s9.value.equals(sw9.value) &&
             s9.value.equals(literals[9]) &&
             s9.value.equals(literals[20]));

        test(s10.value.equals("\u0DA7") &&
             s10.value.equals(sw10.value) &&
             s10.value.equals(literals[10]) &&
             s10.value.equals(literals[21]));
    
        test(ss0.value.equals("\'\"\u003f\\\u0007\b\f\n\r\t\u000b") &&
             ss0.value.equals(ss1.value) &&
             ss0.value.equals(ss2.value) &&
             ss0.value.equals(literals[22]) &&
             ss0.value.equals(literals[23]) &&
             ss0.value.equals(literals[24]));
        
        test(ss3.value.equals("\\\\U\\u\\") &&
             ss3.value.equals(literals[25]));

        test(ss4.value.equals("\\A\\") &&
             ss4.value.equals(literals[26]));

        test(ss5.value.equals("\\u0041\\") &&
             ss5.value.equals(literals[27]));
             
        test(su0.value.equals(su1.value) &&
             su0.value.equals(su2.value) &&
             su0.value.equals(literals[28]) &&
             su0.value.equals(literals[29]) &&
             su0.value.equals(literals[30]));

        p.ice_ping();

        test(p.ice_isA(MyClass.ice_staticId()));

        test(p.ice_id().equals(MyDerivedClass.ice_staticId()));

        test(MyDerivedClassPrxHelper.ice_staticId().equals(MyDerivedClass.ice_staticId()));
        test(ObjectPrxHelper.ice_staticId().equals(Object.ice_staticId));
        test(LocatorPrxHelper.ice_staticId().equals(Locator.ice_staticId));

        {
            String[] ids = p.ice_ids();
            test(ids.length == 3);
            test(ids[0].equals("::Ice::Object"));
            test(ids[1].equals("::Test::MyClass"));
            test(ids[2].equals("::Test::MyDerivedClass"));
        }

        {
            p.opVoid();
        }

        {
            ByteHolder b = new ByteHolder();
            byte r;

            r = p.opByte((byte) 0xff, (byte) 0x0f, b);
            test(b.value == (byte) 0xf0);
            test(r == (byte) 0xff);
        }

        {
            BooleanHolder b = new BooleanHolder();
            boolean r;

            r = p.opBool(true, false, b);
            test(b.value);
            test(!r);
        }

        {
            ShortHolder s = new ShortHolder();
            IntHolder i = new IntHolder();
            LongHolder l = new LongHolder();
            long r;

            r = p.opShortIntLong((short) 10, 11, 12L, s, i, l);
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
            FloatHolder f = new FloatHolder();
            DoubleHolder d = new DoubleHolder();
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
            StringHolder s = new StringHolder();
            String r;

            r = p.opString("hello", "world", s);
            test(s.value.equals("world hello"));
            test(r.equals("hello world"));
        }

        {
            MyEnumHolder e = new MyEnumHolder();
            MyEnum r;

            r = p.opMyEnum(MyEnum.enum2, e);
            test(e.value == MyEnum.enum2);
            test(r == MyEnum.enum3);

            //
            // Test marshalling of null enum (firt enum value is
            // marshalled in this case).
            //
            r = p.opMyEnum(null, e);
            test(e.value == MyEnum.enum1);
            test(r == MyEnum.enum3);
        }

        {
            MyClassPrxHolder c1 = new MyClassPrxHolder();
            MyClassPrxHolder c2 = new MyClassPrxHolder();
            MyClassPrx r;

            r = p.opMyClass(p, c1, c2);
            test(Util.proxyIdentityAndFacetCompare(c1.value, p) == 0);
            test(Util.proxyIdentityAndFacetCompare(c2.value, p) != 0);
            test(Util.proxyIdentityAndFacetCompare(r, p) == 0);
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
            catch(ObjectNotExistException ex)
            {
            }

            r = p.opMyClass(null, c1, c2);
            test(c1.value == null);
            test(c2.value != null);
            test(Util.proxyIdentityAndFacetCompare(r, p) == 0);
            r.opVoid();
        }

        {
            Structure si1 = new Structure();
            si1.p = p;
            si1.e = MyEnum.enum3;
            si1.s = new AnotherStruct();
            si1.s.s = "abc";
            Structure si2 = new Structure();
            si2.p = null;
            si2.e = MyEnum.enum2;
            si2.s = new AnotherStruct();
            si2.s.s = "def";

            StructureHolder so = new StructureHolder();
            Structure rso = p.opStruct(si1, si2, so);
            test(rso.p == null);
            test(rso.e == MyEnum.enum2);
            test(rso.s.s.equals("def"));
            test(so.value.p.equals(p));
            test(so.value.e == MyEnum.enum3);
            test(so.value.s.s.equals("a new string"));
            so.value.p.opVoid();

            //
            // Test marshalling of null structs and structs with null members.
            //
            si1 = new Structure();
            si2 = null;

            so = new StructureHolder();
            rso = p.opStruct(si1, si2, so);
            test(rso.p == null);
            test(rso.e == MyEnum.enum1);
            test(rso.s.s.equals(""));
            test(so.value.p == null);
            test(so.value.e == MyEnum.enum1);
            test(so.value.s.s.equals("a new string"));
        }

        {
            final byte[] bsi1 =
                    {
                            (byte) 0x01,
                            (byte) 0x11,
                            (byte) 0x12,
                            (byte) 0x22
                    };
            final byte[] bsi2 =
                    {
                            (byte) 0xf1,
                            (byte) 0xf2,
                            (byte) 0xf3,
                            (byte) 0xf4
                    };

            ByteSHolder bso = new ByteSHolder();
            byte[] rso;

            rso = p.opByteS(bsi1, bsi2, bso);
            test(bso.value.length == 4);
            test(bso.value[0] == (byte) 0x22);
            test(bso.value[1] == (byte) 0x12);
            test(bso.value[2] == (byte) 0x11);
            test(bso.value[3] == (byte) 0x01);
            test(rso.length == 8);
            test(rso[0] == (byte) 0x01);
            test(rso[1] == (byte) 0x11);
            test(rso[2] == (byte) 0x12);
            test(rso[3] == (byte) 0x22);
            test(rso[4] == (byte) 0xf1);
            test(rso[5] == (byte) 0xf2);
            test(rso[6] == (byte) 0xf3);
            test(rso[7] == (byte) 0xf4);
        }

        {
            final boolean[] bsi1 = {true, true, false};
            final boolean[] bsi2 = {false};

            BoolSHolder bso = new BoolSHolder();
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
            final short[] ssi = {1, 2, 3};
            final int[] isi = {5, 6, 7, 8};
            final long[] lsi = {10, 30, 20};

            ShortSHolder sso = new ShortSHolder();
            IntSHolder iso = new IntSHolder();
            LongSHolder lso = new LongSHolder();
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
            final float[] fsi = {3.14f, 1.11f};
            final double[] dsi = {1.1E10, 1.2E10, 1.3E10};

            FloatSHolder fso = new FloatSHolder();
            DoubleSHolder dso = new DoubleSHolder();
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
            test((float) rso[3] == 3.14f);
            test((float) rso[4] == 1.11f);
        }

        {
            final String[] ssi1 = {"abc", "de", "fghi"};
            final String[] ssi2 = {"xyz"};

            StringSHolder sso = new StringSHolder();
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
                            {(byte) 0x01, (byte) 0x11, (byte) 0x12},
                            {(byte) 0xff}
                    };
            final byte[][] bsi2 =
                    {
                            {(byte) 0x0e},
                            {(byte) 0xf2, (byte) 0xf1}
                    };

            ByteSSHolder bso = new ByteSSHolder();
            byte[][] rso;

            rso = p.opByteSS(bsi1, bsi2, bso);
            test(bso.value.length == 2);
            test(bso.value[0].length == 1);
            test(bso.value[0][0] == (byte) 0xff);
            test(bso.value[1].length == 3);
            test(bso.value[1][0] == (byte) 0x01);
            test(bso.value[1][1] == (byte) 0x11);
            test(bso.value[1][2] == (byte) 0x12);
            test(rso.length == 4);
            test(rso[0].length == 3);
            test(rso[0][0] == (byte) 0x01);
            test(rso[0][1] == (byte) 0x11);
            test(rso[0][2] == (byte) 0x12);
            test(rso[1].length == 1);
            test(rso[1][0] == (byte) 0xff);
            test(rso[2].length == 1);
            test(rso[2][0] == (byte) 0x0e);
            test(rso[3].length == 2);
            test(rso[3][0] == (byte) 0xf2);
            test(rso[3][1] == (byte) 0xf1);
        }

        {
            final boolean[][] bsi1 =
                    {
                            {true},
                            {false},
                            {true, true}
                    };

            final boolean[][] bsi2 =
                    {
                            {false, false, true}
                    };

            BoolSSHolder bso = new BoolSSHolder();
            boolean[][] rso;

            rso = p.opBoolSS(bsi1, bsi2, bso);
            test(bso.value.length == 4);
            test(bso.value[0].length == 1);
            test(bso.value[0][0]);
            test(bso.value[1].length == 1);
            test(!bso.value[1][0]);
            test(bso.value[2].length == 2);
            test(bso.value[2][0]);
            test(bso.value[2][1]);
            test(bso.value[3].length == 3);
            test(!bso.value[3][0]);
            test(!bso.value[3][1]);
            test(bso.value[3][2]);
            test(rso.length == 3);
            test(rso[0].length == 2);
            test(rso[0][0]);
            test(rso[0][1]);
            test(rso[1].length == 1);
            test(!rso[1][0]);
            test(rso[2].length == 1);
            test(rso[2][0]);
        }

        {
            final short[][] ssi =
                    {
                            {1, 2, 5},
                            {13},
                            {}
                    };
            final int[][] isi =
                    {
                            {24, 98},
                            {42}
                    };
            final long[][] lsi =
                    {
                            {496, 1729},
                    };

            ShortSSHolder sso = new ShortSSHolder();
            IntSSHolder iso = new IntSSHolder();
            LongSSHolder lso = new LongSSHolder();
            long[][] rso;

            rso = p.opShortIntLongSS(ssi, isi, lsi, sso, iso, lso);
            test(rso.length == 1);
            test(rso[0].length == 2);
            test(rso[0][0] == 496);
            test(rso[0][1] == 1729);
            test(sso.value.length == 3);
            test(sso.value[0].length == 3);
            test(sso.value[0][0] == 1);
            test(sso.value[0][1] == 2);
            test(sso.value[0][2] == 5);
            test(sso.value[1].length == 1);
            test(sso.value[1][0] == 13);
            test(sso.value[2].length == 0);
            test(iso.value.length == 2);
            test(iso.value[0].length == 1);
            test(iso.value[0][0] == 42);
            test(iso.value[1].length == 2);
            test(iso.value[1][0] == 24);
            test(iso.value[1][1] == 98);
            test(lso.value.length == 2);
            test(lso.value[0].length == 2);
            test(lso.value[0][0] == 496);
            test(lso.value[0][1] == 1729);
            test(lso.value[1].length == 2);
            test(lso.value[1][0] == 496);
            test(lso.value[1][1] == 1729);
        }

        {
            final float[][] fsi =
                    {
                            {3.14f},
                            {1.11f},
                            {},
                    };
            final double[][] dsi =
                    {
                            {1.1E10, 1.2E10, 1.3E10}
                    };

            FloatSSHolder fso = new FloatSSHolder();
            DoubleSSHolder dso = new DoubleSSHolder();
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
                            {"abc"},
                            {"de", "fghi"}
                    };
            final String[][] ssi2 =
                    {
                            {},
                            {},
                            {"xyz"}
                    };

            StringSSHolder sso = new StringSSHolder();
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

            StringSSSHolder ssso = new StringSSSHolder();
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
            Map<Byte, Boolean> di1 = new HashMap<>();
            di1.put((byte) 10, Boolean.TRUE);
            di1.put((byte) 100, Boolean.FALSE);
            Map<Byte, Boolean> di2 = new HashMap<>();
            di2.put((byte) 10, Boolean.TRUE);
            di2.put((byte) 11, Boolean.FALSE);
            di2.put((byte) 101, Boolean.TRUE);

            ByteBoolDHolder _do = new ByteBoolDHolder();
            Map<Byte, Boolean> ro = p.opByteBoolD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get((byte) 10));
            test(!ro.get((byte) 11));
            test(!ro.get((byte) 100));
            test(ro.get((byte) 101));
        }

        {
            Map<Short, Integer> di1 = new HashMap<>();
            di1.put((short) 110, -1);
            di1.put((short) 1100, 123123);
            Map<Short, Integer> di2 = new HashMap<>();
            di2.put((short) 110, -1);
            di2.put((short) 111, -100);
            di2.put((short) 1101, 0);

            ShortIntDHolder _do = new ShortIntDHolder();
            Map<Short, Integer> ro = p.opShortIntD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get((short) 110) == -1);
            test(ro.get((short) 111) == -100);
            test(ro.get((short) 1100) == 123123);
            test(ro.get((short) 1101) == 0);
        }

        {
            Map<Long, Float> di1 = new HashMap<>();
            di1.put(999999110L, -1.1f);
            di1.put(999999111L, 123123.2f);
            Map<Long, Float> di2 = new HashMap<>();
            di2.put(999999110L, -1.1f);
            di2.put(999999120L, -100.4f);
            di2.put(999999130L, 0.5f);

            LongFloatDHolder _do = new LongFloatDHolder();
            Map<Long, Float> ro = p.opLongFloatD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get(999999110L) == -1.1f);
            test(ro.get(999999120L) == -100.4f);
            test(ro.get(999999111L) == 123123.2f);
            test(ro.get(999999130L) == 0.5f);
        }

        {
            Map<String, String> di1 = new HashMap<>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            Map<String, String> di2 = new HashMap<>();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");

            StringStringDHolder _do = new StringStringDHolder();
            Map<String, String> ro = p.opStringStringD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get("foo").equals("abc -1.1"));
            test(ro.get("FOO").equals("abc -100.4"));
            test(ro.get("bar").equals("abc 123123.2"));
            test(ro.get("BAR").equals("abc 0.5"));
        }

        {
            Map<String, MyEnum> di1 = new HashMap<>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            Map<String, MyEnum> di2 = new HashMap<>();
            di2.put("abc", MyEnum.enum1);
            di2.put("qwerty", MyEnum.enum3);
            di2.put("Hello!!", MyEnum.enum2);

            StringMyEnumDHolder _do = new StringMyEnumDHolder();
            Map<String, MyEnum> ro = p.opStringMyEnumD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get("abc") == MyEnum.enum1);
            test(ro.get("qwerty") == MyEnum.enum3);
            test(ro.get("") == MyEnum.enum2);
            test(ro.get("Hello!!") == MyEnum.enum2);
        }

        {
            Map<MyEnum, String> di1 = new HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            Map<MyEnum, String> di2 = new HashMap<>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            MyEnumStringDHolder _do = new MyEnumStringDHolder();
            Map<MyEnum, String> ro = p.opMyEnumStringD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 3);
            test(ro.get(MyEnum.enum1).equals("abc"));
            test(ro.get(MyEnum.enum2).equals("Hello!!"));
            test(ro.get(MyEnum.enum3).equals("qwerty"));
        }

        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            Map<MyStruct, MyEnum> di1 = new HashMap<>();
            di1.put(s11, MyEnum.enum1);
            di1.put(s12, MyEnum.enum2);

            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);
            Map<MyStruct, MyEnum> di2 = new HashMap<>();
            di2.put(s11, MyEnum.enum1);
            di2.put(s22, MyEnum.enum3);
            di2.put(s23, MyEnum.enum2);

            MyStructMyEnumDHolder _do = new MyStructMyEnumDHolder();
            Map<MyStruct, MyEnum> ro = p.opMyStructMyEnumD(di1, di2, _do);

            test(_do.value.equals(di1));
            test(ro.size() == 4);
            test(ro.get(s11) == MyEnum.enum1);
            test(ro.get(s12) == MyEnum.enum2);
            test(ro.get(s22) == MyEnum.enum3);
            test(ro.get(s23) == MyEnum.enum2);
        }

        {
            List<Map<Byte, Boolean>> dsi1 = new ArrayList<>();
            List<Map<Byte, Boolean>> dsi2 = new ArrayList<>();

            Map<Byte, Boolean> di1 = new HashMap<>();
            di1.put((byte) 10, Boolean.TRUE);
            di1.put((byte) 100, Boolean.FALSE);
            Map<Byte, Boolean> di2 = new HashMap<>();
            di2.put((byte) 10, Boolean.TRUE);
            di2.put((byte) 11, Boolean.FALSE);
            di2.put((byte) 101, Boolean.TRUE);
            Map<Byte, Boolean> di3 = new HashMap<>();
            di3.put((byte) 100, Boolean.FALSE);
            di3.put((byte) 101, Boolean.FALSE);

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            ByteBoolDSHolder _do = new ByteBoolDSHolder();
            List<Map<Byte, Boolean>> ro = p.opByteBoolDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get((byte) 10));
            test(!ro.get(0).get((byte) 11));
            test(ro.get(0).get((byte) 101));
            test(ro.get(1).size() == 2);
            test(ro.get(1).get((byte) 10));
            test(!ro.get(1).get((byte) 100));

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 2);
            test(!_do.value.get(0).get((byte) 100));
            test(!_do.value.get(0).get((byte) 101));
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get((byte) 10));
            test(!_do.value.get(1).get((byte) 100));
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get((byte) 10));
            test(!_do.value.get(2).get((byte) 11));
            test(_do.value.get(2).get((byte) 101));
        }

        {
            List<Map<Short, Integer>> dsi1 = new ArrayList<>();
            List<Map<Short, Integer>> dsi2 = new ArrayList<>();

            Map<Short, Integer> di1 = new HashMap<>();
            di1.put((short) 110, -1);
            di1.put((short) 1100, 123123);
            Map<Short, Integer> di2 = new HashMap<>();
            di2.put((short) 110, -1);
            di2.put((short) 111, -100);
            di2.put((short) 1101, 0);
            Map<Short, Integer> di3 = new HashMap<>();
            di3.put((short) 100, -1001);

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            ShortIntDSHolder _do = new ShortIntDSHolder();
            List<Map<Short, Integer>> ro = p.opShortIntDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get((short) 110) == -1);
            test(ro.get(0).get((short) 111) == -100);
            test(ro.get(0).get((short) 1101) == 0);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get((short) 110) == -1);
            test(ro.get(1).get((short) 1100) == 123123);

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get((short) 100) == -1001);
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get((short) 110) == -1);
            test(_do.value.get(1).get((short) 1100) == 123123);
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get((short) 110) == -1);
            test(_do.value.get(2).get((short) 111) == -100);
            test(_do.value.get(2).get((short) 1101) == 0);
        }

        {
            List<Map<Long, Float>> dsi1 = new ArrayList<>();
            List<Map<Long, Float>> dsi2 = new ArrayList<>();

            Map<Long, Float> di1 = new HashMap<>();
            di1.put(999999110L, new Float(-1.1));
            di1.put(999999111L, new Float(123123.2));
            Map<Long, Float> di2 = new HashMap<>();
            di2.put(999999110L, new Float(-1.1));
            di2.put(999999120L, new Float(-100.4));
            di2.put(999999130L, new Float(0.5));
            Map<Long, Float> di3 = new HashMap<>();
            di3.put(999999140L, new Float(3.14));

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            LongFloatDSHolder _do = new LongFloatDSHolder();
            List<Map<Long, Float>> ro = p.opLongFloatDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get(999999110L) == -1.1f);
            test(ro.get(0).get(999999120L) == -100.4f);
            test(ro.get(0).get(999999130L) == 0.5f);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get(999999110L) == -1.1f);
            test(ro.get(1).get(999999111L) == 123123.2f);

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get(999999140L) == 3.14f);
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get(999999110L) == -1.1f);
            test(_do.value.get(1).get(999999111L) == 123123.2f);
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get(999999110L) == -1.1f);
            test(_do.value.get(2).get(999999120L) == -100.4f);
            test(_do.value.get(2).get(999999130L) == 0.5f);
        }

        {
            List<Map<String, String>> dsi1 = new ArrayList<>();
            List<Map<String, String>> dsi2 = new ArrayList<>();

            java.util.Map<String, String> di1 = new HashMap<>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            java.util.Map<String, String> di2 = new HashMap<>();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");
            java.util.Map<String, String> di3 = new HashMap<>();
            di3.put("f00", "ABC -3.14");

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            StringStringDSHolder _do = new StringStringDSHolder();
            List<Map<String, String>> ro = p.opStringStringDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get("foo").equals("abc -1.1"));
            test(ro.get(0).get("FOO").equals("abc -100.4"));
            test(ro.get(0).get("BAR").equals("abc 0.5"));
            test(ro.get(1).size() == 2);
            test(ro.get(1).get("foo").equals("abc -1.1"));
            test(ro.get(1).get("bar").equals("abc 123123.2"));

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get("f00").equals("ABC -3.14"));
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get("foo").equals("abc -1.1"));
            test(_do.value.get(1).get("bar").equals("abc 123123.2"));
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get("foo").equals("abc -1.1"));
            test(_do.value.get(2).get("FOO").equals("abc -100.4"));
            test(_do.value.get(2).get("BAR").equals("abc 0.5"));
        }

        {
            List<Map<String, MyEnum>> dsi1 = new ArrayList<>();
            List<Map<String, MyEnum>> dsi2 = new ArrayList<>();

            java.util.Map<String, MyEnum> di1 = new HashMap<>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            java.util.Map<String, MyEnum> di2 = new HashMap<>();
            di2.put("abc", MyEnum.enum1);
            di2.put("qwerty", MyEnum.enum3);
            di2.put("Hello!!", MyEnum.enum2);
            java.util.Map<String, MyEnum> di3 = new HashMap<>();
            di3.put("Goodbye", MyEnum.enum1);

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            StringMyEnumDSHolder _do = new StringMyEnumDSHolder();
            List<Map<String, MyEnum>> ro = p.opStringMyEnumDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get("abc") == MyEnum.enum1);
            test(ro.get(0).get("qwerty") == MyEnum.enum3);
            test(ro.get(0).get("Hello!!") == MyEnum.enum2);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get("abc") == MyEnum.enum1);
            test(ro.get(1).get("") == MyEnum.enum2);

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get("Goodbye") == MyEnum.enum1);
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get("abc") == MyEnum.enum1);
            test(_do.value.get(1).get("") == MyEnum.enum2);
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get("abc") == MyEnum.enum1);
            test(_do.value.get(2).get("qwerty") == MyEnum.enum3);
            test(_do.value.get(2).get("Hello!!") == MyEnum.enum2);
        }

        {
            List<Map<MyEnum, String>> dsi1 = new ArrayList<>();
            List<Map<MyEnum, String>> dsi2 = new ArrayList<>();

            java.util.Map<MyEnum, String> di1 = new HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            java.util.Map<MyEnum, String> di2 = new HashMap<>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");
            java.util.Map<MyEnum, String> di3 = new HashMap<>();
            di3.put(MyEnum.enum1, "Goodbye");

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            MyEnumStringDSHolder _do = new MyEnumStringDSHolder();
            List<Map<MyEnum, String>> ro = p.opMyEnumStringDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 2);
            test(ro.get(0).get(MyEnum.enum2).equals("Hello!!"));
            test(ro.get(0).get(MyEnum.enum3).equals("qwerty"));
            test(ro.get(1).size() == 1);
            test(ro.get(1).get(MyEnum.enum1).equals("abc"));

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get(MyEnum.enum1).equals("Goodbye"));
            test(_do.value.get(1).size() == 1);
            test(_do.value.get(1).get(MyEnum.enum1).equals("abc"));
            test(_do.value.get(2).size() == 2);
            test(_do.value.get(2).get(MyEnum.enum2).equals("Hello!!"));
            test(_do.value.get(2).get(MyEnum.enum3).equals("qwerty"));
        }

        {
            List<Map<MyStruct, MyEnum>> dsi1 = new ArrayList<>();
            List<Map<MyStruct, MyEnum>> dsi2 = new ArrayList<>();

            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            java.util.Map<MyStruct, MyEnum> di1 = new HashMap<>();
            di1.put(s11, MyEnum.enum1);
            di1.put(s12, MyEnum.enum2);

            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);
            java.util.Map<MyStruct, MyEnum> di2 = new HashMap<>();
            di2.put(s11, MyEnum.enum1);
            di2.put(s22, MyEnum.enum3);
            di2.put(s23, MyEnum.enum2);

            java.util.Map<MyStruct, MyEnum> di3 = new HashMap<>();
            di3.put(s23, MyEnum.enum3);

            dsi1.add(di1);
            dsi1.add(di2);
            dsi2.add(di3);

            MyStructMyEnumDSHolder _do = new MyStructMyEnumDSHolder();
            List<Map<MyStruct, MyEnum>> ro = p.opMyStructMyEnumDS(dsi1, dsi2, _do);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get(s11) == MyEnum.enum1);
            test(ro.get(0).get(s22) == MyEnum.enum3);
            test(ro.get(0).get(s23) == MyEnum.enum2);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get(s11) == MyEnum.enum1);
            test(ro.get(1).get(s12) == MyEnum.enum2);

            test(_do.value.size() == 3);
            test(_do.value.get(0).size() == 1);
            test(_do.value.get(0).get(s23) == MyEnum.enum3);
            test(_do.value.get(1).size() == 2);
            test(_do.value.get(1).get(s11) == MyEnum.enum1);
            test(_do.value.get(1).get(s12) == MyEnum.enum2);
            test(_do.value.get(2).size() == 3);
            test(_do.value.get(2).get(s11) == MyEnum.enum1);
            test(_do.value.get(2).get(s22) == MyEnum.enum3);
            test(_do.value.get(2).get(s23) == MyEnum.enum2);
        }

        {
            java.util.Map<Byte, byte[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Byte, byte[]> sdi2 = new java.util.HashMap<>();

            final byte[] si1 = {(byte) 0x01, (byte) 0x11};
            final byte[] si2 = {(byte) 0x12};
            final byte[] si3 = {(byte) 0xf2, (byte) 0xf3};

            sdi1.put((byte) 0x01, si1);
            sdi1.put((byte) 0x22, si2);
            sdi2.put((byte) 0xf1, si3);

            ByteByteSDHolder _do = new ByteByteSDHolder();
            java.util.Map<Byte, byte[]> ro = p.opByteByteSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get((byte) 0xf1).length == 2);
            test(_do.value.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(_do.value.get((byte) 0xf1)[1] == (byte) 0xf3);
            test(ro.size() == 3);
            test(ro.get((byte) 0x01).length == 2);
            test(ro.get((byte) 0x01)[0] == (byte) 0x01);
            test(ro.get((byte) 0x01)[1] == (byte) 0x11);
            test(ro.get((byte) 0x22).length == 1);
            test(ro.get((byte) 0x22)[0] == (byte) 0x12);
            test(ro.get((byte) 0xf1).length == 2);
            test(ro.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(ro.get((byte) 0xf1)[1] == (byte) 0xf3);
        }

        {
            java.util.Map<Boolean, boolean[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Boolean, boolean[]> sdi2 = new java.util.HashMap<>();

            final boolean[] si1 = {true, false};
            final boolean[] si2 = {false, true, true};

            sdi1.put(false, si1);
            sdi1.put(true, si2);
            sdi2.put(false, si1);

            BoolBoolSDHolder _do = new BoolBoolSDHolder();
            java.util.Map<Boolean, boolean[]> ro = p.opBoolBoolSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get(false).length == 2);
            test(_do.value.get(false)[0]);
            test(!_do.value.get(false)[1]);
            test(ro.size() == 2);
            test(ro.get(false).length == 2);
            test(ro.get(false)[0]);
            test(!ro.get(false)[1]);
            test(ro.get(true).length == 3);
            test(!ro.get(true)[0]);
            test(ro.get(true)[1]);
            test(ro.get(true)[2]);
        }

        {
            java.util.Map<Short, short[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Short, short[]> sdi2 = new java.util.HashMap<>();

            final short[] si1 = {1, 2, 3};
            final short[] si2 = {4, 5};
            final short[] si3 = {6, 7};

            sdi1.put((short) 1, si1);
            sdi1.put((short) 2, si2);
            sdi2.put((short) 4, si3);

            ShortShortSDHolder _do = new ShortShortSDHolder();
            java.util.Map<Short, short[]> ro = p.opShortShortSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get((short) 4).length == 2);
            test(_do.value.get((short) 4)[0] == 6);
            test(_do.value.get((short) 4)[1] == 7);
            test(ro.size() == 3);
            test(ro.get((short) 1).length == 3);
            test(ro.get((short) 1)[0] == 1);
            test(ro.get((short) 1)[1] == 2);
            test(ro.get((short) 1)[2] == 3);
            test(ro.get((short) 2).length == 2);
            test(ro.get((short) 2)[0] == 4);
            test(ro.get((short) 2)[1] == 5);
            test(ro.get((short) 4).length == 2);
            test(ro.get((short) 4)[0] == 6);
            test(ro.get((short) 4)[1] == 7);
        }

        {
            java.util.Map<Integer, int[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Integer, int[]> sdi2 = new java.util.HashMap<>();

            final int[] si1 = {100, 200, 300};
            final int[] si2 = {400, 500};
            final int[] si3 = {600, 700};

            sdi1.put(100, si1);
            sdi1.put(200, si2);
            sdi2.put(400, si3);

            IntIntSDHolder _do = new IntIntSDHolder();
            java.util.Map<Integer, int[]> ro = p.opIntIntSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get(400).length == 2);
            test(_do.value.get(400)[0] == 600);
            test(_do.value.get(400)[1] == 700);
            test(ro.size() == 3);
            test(ro.get(100).length == 3);
            test(ro.get(100)[0] == 100);
            test(ro.get(100)[1] == 200);
            test(ro.get(100)[2] == 300);
            test(ro.get(200).length == 2);
            test(ro.get(200)[0] == 400);
            test(ro.get(200)[1] == 500);
            test(ro.get(400).length == 2);
            test(ro.get(400)[0] == 600);
            test(ro.get(400)[1] == 700);
        }

        {
            java.util.Map<Long, long[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Long, long[]> sdi2 = new java.util.HashMap<>();

            final long[] si1 = {999999110L, 999999111L, 999999110L};
            final long[] si2 = {999999120L, 999999130L};
            final long[] si3 = {999999110L, 999999120L};

            sdi1.put(999999990L, si1);
            sdi1.put(999999991L, si2);
            sdi2.put(999999992L, si3);

            LongLongSDHolder _do = new LongLongSDHolder();
            java.util.Map<Long, long[]> ro = p.opLongLongSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get(999999992L).length == 2);
            test(_do.value.get(999999992L)[0] == 999999110L);
            test(_do.value.get(999999992L)[1] == 999999120L);
            test(ro.size() == 3);
            test(ro.get(999999990L).length == 3);
            test(ro.get(999999990L)[0] == 999999110L);
            test(ro.get(999999990L)[1] == 999999111L);
            test(ro.get(999999990L)[2] == 999999110L);
            test(ro.get(999999991L).length == 2);
            test(ro.get(999999991L)[0] == 999999120L);
            test(ro.get(999999991L)[1] == 999999130L);
            test(ro.get(999999992L).length == 2);
            test(ro.get(999999992L)[0] == 999999110L);
            test(ro.get(999999992L)[1] == 999999120L);
        }

        {
            java.util.Map<String, float[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<String, float[]> sdi2 = new java.util.HashMap<>();

            final float[] si1 = {-1.1f, 123123.2f, 100.0f};
            final float[] si2 = {42.24f, -1.61f};
            final float[] si3 = {-3.14f, 3.14f};

            sdi1.put("abc", si1);
            sdi1.put("ABC", si2);
            sdi2.put("aBc", si3);

            StringFloatSDHolder _do = new StringFloatSDHolder();
            java.util.Map<String, float[]> ro = p.opStringFloatSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get("aBc").length == 2);
            test(_do.value.get("aBc")[0] == -3.14f);
            test(_do.value.get("aBc")[1] == 3.14f);
            test(ro.size() == 3);
            test(ro.get("abc").length == 3);
            test(ro.get("abc")[0] == -1.1f);
            test(ro.get("abc")[1] == 123123.2f);
            test(ro.get("abc")[2] == 100.0f);
            test(ro.get("ABC").length == 2);
            test(ro.get("ABC")[0] == 42.24f);
            test(ro.get("ABC")[1] == -1.61f);
            test(ro.get("aBc").length == 2);
            test(ro.get("aBc")[0] == -3.14f);
            test(ro.get("aBc")[1] == 3.14f);
        }

        {
            java.util.Map<String, double[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<String, double[]> sdi2 = new java.util.HashMap<>();

            double[] si1 = new double[] { 1.1E10, 1.2E10, 1.3E10 };
            double[] si2 = new double[] { 1.4E10, 1.5E10 };
            double[] si3 = new double[] { 1.6E10, 1.7E10 };

            sdi1.put("Hello!!", si1);
            sdi1.put("Goodbye",  si2);
            sdi2.put("", si3);

            StringDoubleSDHolder _do = new StringDoubleSDHolder();
            java.util.Map<String, double[]> ro = p.opStringDoubleSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get("").length == 2);
            test(_do.value.get("")[0] == 1.6E10);
            test(_do.value.get("")[1] == 1.7E10);
            test(ro.size()== 3);
            test(ro.get("Hello!!").length == 3);
            test(ro.get("Hello!!")[0] == 1.1E10);
            test(ro.get("Hello!!")[1] == 1.2E10);
            test(ro.get("Hello!!")[2] == 1.3E10);
            test(ro.get("Goodbye").length == 2);
            test(ro.get("Goodbye")[0] == 1.4E10);
            test(ro.get("Goodbye")[1] == 1.5E10);
            test(ro.get("").length== 2);
            test(ro.get("")[0] == 1.6E10);
            test(ro.get("")[1] == 1.7E10);
        }

        {
            java.util.Map<String, String[]> sdi1 = new java.util.HashMap<String, String[]>();
            java.util.Map<String, String[]> sdi2 = new java.util.HashMap<String, String[]>();

            String[] si1 = new String[] { "abc", "de", "fghi" };
            String[] si2 = new String[] { "xyz", "or" };
            String[] si3 = new String[] { "and", "xor" };

            sdi1.put("abc", si1);
            sdi1.put("def", si2);
            sdi2.put("ghi", si3);

            StringStringSDHolder _do = new StringStringSDHolder();
            java.util.Map<String, String[]> ro = p.opStringStringSD(sdi1, sdi2, _do);


            test(_do.value.size() == 1);
            test(_do.value.get("ghi").length== 2);
            test(_do.value.get("ghi")[0].equals("and"));
            test(_do.value.get("ghi")[1].equals("xor"));
            test(ro.size()== 3);
            test(ro.get("abc").length == 3);
            test(ro.get("abc")[0].equals("abc"));
            test(ro.get("abc")[1].equals("de"));
            test(ro.get("abc")[2].equals("fghi"));
            test(ro.get("def").length == 2);
            test(ro.get("def")[0].equals("xyz"));
            test(ro.get("def")[1].equals("or"));
            test(ro.get("ghi").length == 2);
            test(ro.get("ghi")[0].equals("and"));
            test(ro.get("ghi")[1].equals("xor"));
        }

        {
            java.util.Map<MyEnum, MyEnum[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<MyEnum, MyEnum[]> sdi2 = new java.util.HashMap<>();

            final MyEnum[] si1 = new MyEnum[] { MyEnum.enum1, MyEnum.enum1, MyEnum.enum2 };
            final MyEnum[] si2 = new MyEnum[] { MyEnum.enum1, MyEnum.enum2 };
            final MyEnum[] si3 = new MyEnum[] { MyEnum.enum3, MyEnum.enum3 };

            sdi1.put(MyEnum.enum3, si1);
            sdi1.put(MyEnum.enum2, si2);
            sdi2.put(MyEnum.enum1, si3);

            MyEnumMyEnumSDHolder _do = new MyEnumMyEnumSDHolder();
            java.util.Map<MyEnum, MyEnum[]> ro = p.opMyEnumMyEnumSD(sdi1, sdi2, _do);

            test(_do.value.size() == 1);
            test(_do.value.get(MyEnum.enum1).length == 2);
            test(_do.value.get(MyEnum.enum1)[0] == MyEnum.enum3);
            test(_do.value.get(MyEnum.enum1)[1] == MyEnum.enum3);
            test(ro.size()== 3);
            test(ro.get(MyEnum.enum3).length == 3);
            test(ro.get(MyEnum.enum3)[0] == MyEnum.enum1);
            test(ro.get(MyEnum.enum3)[1] == MyEnum.enum1);
            test(ro.get(MyEnum.enum3)[2] == MyEnum.enum2);
            test(ro.get(MyEnum.enum2).length == 2);
            test(ro.get(MyEnum.enum2)[0] == MyEnum.enum1);
            test(ro.get(MyEnum.enum2)[1] == MyEnum.enum2);
            test(ro.get(MyEnum.enum1).length == 2);
            test(ro.get(MyEnum.enum1)[0] == MyEnum.enum3);
            test(ro.get(MyEnum.enum1)[1] == MyEnum.enum3);
        }

        {
            int[] lengths = {0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000};

            for(int l : lengths)
            {
                int[] s = new int[l];
                for(int i = 0; i < l; ++i)
                {
                    s[i] = i;
                }
                int[] r = p.opIntS(s);
                test(r.length == l);
                for(int j = 0; j < r.length; ++j)
                {
                    test(r[j] == -j);
                }
            }
        }

        {
            Map<String, String> ctx = new HashMap<>();
            ctx.put("one", "ONE");
            ctx.put("two", "TWO");
            ctx.put("three", "THREE");
            {
                test(p.ice_getContext().isEmpty());
                Map<String, String> r = p.opContext();
                test(!r.equals(ctx));
            }
            {
                Map<String, String> r = p.opContext(ctx);
                test(p.ice_getContext().isEmpty());
                test(r.equals(ctx));
            }
            {
                MyClassPrx p2 = MyClassPrxHelper.checkedCast(p.ice_context(ctx));
                test(p2.ice_getContext().equals(ctx));
                Map<String, String> r = p2.opContext();
                test(r.equals(ctx));
                r = p2.opContext(ctx);
                test(r.equals(ctx));
            }
        }

        if(p.ice_getConnection() != null)
        {
            //
            // Test implicit context propagation
            //

            String[] impls = {"Shared", "PerThread"};
            for(int i = 0; i < 2; i++)
            {
                InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                Communicator ic = app.initialize(initData);

                Map<String, String> ctx = new HashMap<>();
                ctx.put("one", "ONE");
                ctx.put("two", "TWO");
                ctx.put("three", "THREE");

                MyClassPrx p3 = MyClassPrxHelper.uncheckedCast(ic.stringToProxy("test:default -p 12010"));

                ic.getImplicitContext().setContext(ctx);
                test(ic.getImplicitContext().getContext().equals(ctx));
                test(p3.opContext().equals(ctx));

                test(!ic.getImplicitContext().containsKey("zero"));
                String r = ic.getImplicitContext().put("zero", "ZERO");
                test(r.equals(""));
                test(ic.getImplicitContext().containsKey("zero"));
                test(ic.getImplicitContext().get("zero").equals("ZERO"));

                ctx = ic.getImplicitContext().getContext();
                test(p3.opContext().equals(ctx));

                Map<String, String> prxContext = new HashMap<>();
                prxContext.put("one", "UN");
                prxContext.put("four", "QUATRE");

                Map<String, String> combined = new HashMap<>(ctx);
                combined.putAll(prxContext);
                test(combined.get("one").equals("UN"));

                p3 = MyClassPrxHelper.uncheckedCast(p3.ice_context(prxContext));

                ic.getImplicitContext().setContext(null);
                test(p3.opContext().equals(prxContext));

                ic.getImplicitContext().setContext(ctx);
                test(p3.opContext().equals(combined));

                test(ic.getImplicitContext().remove("one").equals("ONE"));

                if(impls[i].equals("PerThread"))
                {
                    Thread thread = new PerThreadContextInvokeThread(
                            MyClassPrxHelper.uncheckedCast(p3.ice_context(null)));
                    thread.start();
                    try
                    {
                        thread.join();
                    }
                    catch(InterruptedException ex)
                    {
                    }
                }

                ic.destroy();
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

        p.opIdempotent();

        p.opNonmutating();
        
        test(p.opByte1((byte)0xFF) == (byte)0xFF);
        test(p.opShort1((short)0x7FFF) == (short)0x7FFF);
        test(p.opInt1(0x7FFFFFFF) == 0x7FFFFFFF);
        //test(p.opLong1(0x7FFFFFFFFFFFFFFF) == 0x7FFFFFFFFFFFFFFF);
        test(p.opFloat1(1.0f) == 1.0f);
        test(p.opDouble1(1.0d) == 1.0d);
        test(p.opString1("opString1").equals("opString1"));
        test(p.opStringS1(null).length == 0);
        test(p.opByteBoolD1(null).size() == 0);
        test(p.opStringS2(null).length == 0);
        test(p.opByteBoolD2(null).size() == 0);
        
        
        MyDerivedClassPrx d = MyDerivedClassPrxHelper.uncheckedCast(p);
        MyStruct1 s = new MyStruct1();
        s.tesT = "Test.MyStruct1.s";
        s.myClass = null;
        s.myStruct1 = "Test.MyStruct1.myStruct1";
        s = d.opMyStruct1(s);
        test(s.tesT.equals("Test.MyStruct1.s"));
        test(s.myClass == null);
        test(s.myStruct1.equals("Test.MyStruct1.myStruct1"));
        MyClass1 c = new MyClass1();
        c.tesT = "Test.MyClass1.testT";
        c.myClass = null;
        c.myClass1 = "Test.MyClass1.myClass1";
        c = d.opMyClass1(c);
        test(c.tesT.equals("Test.MyClass1.testT"));
        test(c.myClass == null);
        test(c.myClass1.equals("Test.MyClass1.myClass1"));
    }
}
