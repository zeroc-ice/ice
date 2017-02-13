// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.zeroc.Ice.*;

import test.Ice.operations.Test.*;
import test.Util.Application;

class Twoways
{
    private static void test(boolean b)
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
        public void run()
        {
            java.util.Map<String, String> ctx = _proxy.ice_getCommunicator().getImplicitContext().getContext();
            test(ctx.isEmpty());
            ctx.put("one", "ONE");
            _proxy.ice_getCommunicator().getImplicitContext().setContext(ctx);
            test(_proxy.opContext().equals(ctx));
        }

        final private MyClassPrx _proxy;
    }

    static void twoways(Application app, MyClassPrx p)
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

        test(ss0.value.equals("\'\"\u003f\\\u0007\b\f\n\r\t\u000b\6") &&
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

        test(MyDerivedClassPrx.ice_staticId().equals(MyDerivedClass.ice_staticId()));
        test(ObjectPrx.ice_staticId().equals(com.zeroc.Ice.Object.ice_staticId()));
        test(LocatorPrx.ice_staticId().equals(Locator.ice_staticId()));

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
            MyClass.OpByteResult r = p.opByte((byte) 0xff, (byte) 0x0f);
            test(r.p3 == (byte) 0xf0);
            test(r.returnValue == (byte) 0xff);
        }

        {
            MyClass.OpBoolResult r = p.opBool(true, false);
            test(r.p3);
            test(!r.returnValue);
        }

        {
            MyClass.OpShortIntLongResult r = p.opShortIntLong((short) 10, 11, 12L);
            test(r.p4 == 10);
            test(r.p5 == 11);
            test(r.p6 == 12);
            test(r.returnValue == 12L);

            r = p.opShortIntLong(Short.MIN_VALUE, Integer.MIN_VALUE, Long.MIN_VALUE);
            test(r.p4 == Short.MIN_VALUE);
            test(r.p5 == Integer.MIN_VALUE);
            test(r.p6 == Long.MIN_VALUE);
            test(r.returnValue == Long.MIN_VALUE);

            r = p.opShortIntLong(Short.MAX_VALUE, Integer.MAX_VALUE, Long.MAX_VALUE);
            test(r.p4 == Short.MAX_VALUE);
            test(r.p5 == Integer.MAX_VALUE);
            test(r.p6 == Long.MAX_VALUE);
            test(r.returnValue == Long.MAX_VALUE);
        }

        {
            MyClass.OpFloatDoubleResult r = p.opFloatDouble(3.14f, 1.1E10);
            test(r.p3 == 3.14f);
            test(r.p4 == 1.1E10);
            test(r.returnValue == 1.1E10);

            r = p.opFloatDouble(Float.MIN_VALUE, Double.MIN_VALUE);
            test(r.p3 == Float.MIN_VALUE);
            test(r.p4 == Double.MIN_VALUE);
            test(r.returnValue == Double.MIN_VALUE);

            r = p.opFloatDouble(Float.MAX_VALUE, Double.MAX_VALUE);
            test(r.p3 == Float.MAX_VALUE);
            test(r.p4 == Double.MAX_VALUE);
            test(r.returnValue == Double.MAX_VALUE);
        }

        {
            MyClass.OpStringResult r = p.opString("hello", "world");
            test(r.p3.equals("world hello"));
            test(r.returnValue.equals("hello world"));
        }

        {
            MyClass.OpMyEnumResult r = p.opMyEnum(MyEnum.enum2);
            test(r.p2 == MyEnum.enum2);
            test(r.returnValue == MyEnum.enum3);

            //
            // Test marshalling of null enum (first enum value is
            // marshalled in this case).
            //
            r = p.opMyEnum(null);
            test(r.p2 == MyEnum.enum1);
            test(r.returnValue == MyEnum.enum3);
        }

        {
            MyClass.OpMyClassResult r = p.opMyClass(p);
            test(Util.proxyIdentityAndFacetCompare(r.p2, p) == 0);
            test(Util.proxyIdentityAndFacetCompare(r.p3, p) != 0);
            test(Util.proxyIdentityAndFacetCompare(r.returnValue, p) == 0);
            test(r.p2.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("test")));
            test(r.p3.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("noSuchIdentity")));
            test(r.returnValue.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("test")));
            r.returnValue.opVoid();
            r.p2.opVoid();
            try
            {
                r.p3.opVoid();
                test(false);
            }
            catch(ObjectNotExistException ex)
            {
            }

            r = p.opMyClass(null);
            test(r.p2 == null);
            test(r.p3 != null);
            test(Util.proxyIdentityAndFacetCompare(r.returnValue, p) == 0);
            r.returnValue.opVoid();
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

            MyClass.OpStructResult r = p.opStruct(si1, si2);
            test(r.returnValue.p == null);
            test(r.returnValue.e == MyEnum.enum2);
            test(r.returnValue.s.s.equals("def"));
            test(r.p3.p.equals(p));
            test(r.p3.e == MyEnum.enum3);
            test(r.p3.s.s.equals("a new string"));
            r.p3.p.opVoid();

            //
            // Test marshalling of null structs and structs with default member values.
            //
            si1 = new Structure();
            si2 = null;

            r = p.opStruct(si1, si2);
            test(r.returnValue.p == null);
            test(r.returnValue.e == MyEnum.enum1);
            test(r.returnValue.s.s.equals(""));
            test(r.p3.p == null);
            test(r.p3.e == MyEnum.enum1);
            test(r.p3.s.s.equals("a new string"));
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

            MyClass.OpByteSResult r = p.opByteS(bsi1, bsi2);
            test(r.p3.length == 4);
            test(r.p3[0] == (byte) 0x22);
            test(r.p3[1] == (byte) 0x12);
            test(r.p3[2] == (byte) 0x11);
            test(r.p3[3] == (byte) 0x01);
            test(r.returnValue.length == 8);
            test(r.returnValue[0] == (byte) 0x01);
            test(r.returnValue[1] == (byte) 0x11);
            test(r.returnValue[2] == (byte) 0x12);
            test(r.returnValue[3] == (byte) 0x22);
            test(r.returnValue[4] == (byte) 0xf1);
            test(r.returnValue[5] == (byte) 0xf2);
            test(r.returnValue[6] == (byte) 0xf3);
            test(r.returnValue[7] == (byte) 0xf4);
        }

        {
            final boolean[] bsi1 = {true, true, false};
            final boolean[] bsi2 = {false};

            MyClass.OpBoolSResult r = p.opBoolS(bsi1, bsi2);
            test(r.p3.length == 4);
            test(r.p3[0]);
            test(r.p3[1]);
            test(!r.p3[2]);
            test(!r.p3[3]);
            test(r.returnValue.length == 3);
            test(!r.returnValue[0]);
            test(r.returnValue[1]);
            test(r.returnValue[2]);
        }

        {
            final short[] ssi = {1, 2, 3};
            final int[] isi = {5, 6, 7, 8};
            final long[] lsi = {10, 30, 20};

            MyClass.OpShortIntLongSResult r = p.opShortIntLongS(ssi, isi, lsi);
            test(r.p4.length == 3);
            test(r.p4[0] == 1);
            test(r.p4[1] == 2);
            test(r.p4[2] == 3);
            test(r.p5.length == 4);
            test(r.p5[0] == 8);
            test(r.p5[1] == 7);
            test(r.p5[2] == 6);
            test(r.p5[3] == 5);
            test(r.p6.length == 6);
            test(r.p6[0] == 10);
            test(r.p6[1] == 30);
            test(r.p6[2] == 20);
            test(r.p6[3] == 10);
            test(r.p6[4] == 30);
            test(r.p6[5] == 20);
            test(r.returnValue.length == 3);
            test(r.returnValue[0] == 10);
            test(r.returnValue[1] == 30);
            test(r.returnValue[2] == 20);
        }

        {
            final float[] fsi = {3.14f, 1.11f};
            final double[] dsi = {1.1E10, 1.2E10, 1.3E10};

            MyClass.OpFloatDoubleSResult r = p.opFloatDoubleS(fsi, dsi);
            test(r.p3.length == 2);
            test(r.p3[0] == 3.14f);
            test(r.p3[1] == 1.11f);
            test(r.p4.length == 3);
            test(r.p4[0] == 1.3E10);
            test(r.p4[1] == 1.2E10);
            test(r.p4[2] == 1.1E10);
            test(r.returnValue.length == 5);
            test(r.returnValue[0] == 1.1E10);
            test(r.returnValue[1] == 1.2E10);
            test(r.returnValue[2] == 1.3E10);
            test((float) r.returnValue[3] == 3.14f);
            test((float) r.returnValue[4] == 1.11f);
        }

        {
            final String[] ssi1 = {"abc", "de", "fghi"};
            final String[] ssi2 = {"xyz"};

            MyClass.OpStringSResult r = p.opStringS(ssi1, ssi2);
            test(r.p3.length == 4);
            test(r.p3[0].equals("abc"));
            test(r.p3[1].equals("de"));
            test(r.p3[2].equals("fghi"));
            test(r.p3[3].equals("xyz"));
            test(r.returnValue.length == 3);
            test(r.returnValue[0].equals("fghi"));
            test(r.returnValue[1].equals("de"));
            test(r.returnValue[2].equals("abc"));
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

            MyClass.OpByteSSResult r = p.opByteSS(bsi1, bsi2);
            test(r.p3.length == 2);
            test(r.p3[0].length == 1);
            test(r.p3[0][0] == (byte) 0xff);
            test(r.p3[1].length == 3);
            test(r.p3[1][0] == (byte) 0x01);
            test(r.p3[1][1] == (byte) 0x11);
            test(r.p3[1][2] == (byte) 0x12);
            test(r.returnValue.length == 4);
            test(r.returnValue[0].length == 3);
            test(r.returnValue[0][0] == (byte) 0x01);
            test(r.returnValue[0][1] == (byte) 0x11);
            test(r.returnValue[0][2] == (byte) 0x12);
            test(r.returnValue[1].length == 1);
            test(r.returnValue[1][0] == (byte) 0xff);
            test(r.returnValue[2].length == 1);
            test(r.returnValue[2][0] == (byte) 0x0e);
            test(r.returnValue[3].length == 2);
            test(r.returnValue[3][0] == (byte) 0xf2);
            test(r.returnValue[3][1] == (byte) 0xf1);
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

            MyClass.OpBoolSSResult r = p.opBoolSS(bsi1, bsi2);
            test(r.p3.length == 4);
            test(r.p3[0].length == 1);
            test(r.p3[0][0]);
            test(r.p3[1].length == 1);
            test(!r.p3[1][0]);
            test(r.p3[2].length == 2);
            test(r.p3[2][0]);
            test(r.p3[2][1]);
            test(r.p3[3].length == 3);
            test(!r.p3[3][0]);
            test(!r.p3[3][1]);
            test(r.p3[3][2]);
            test(r.returnValue.length == 3);
            test(r.returnValue[0].length == 2);
            test(r.returnValue[0][0]);
            test(r.returnValue[0][1]);
            test(r.returnValue[1].length == 1);
            test(!r.returnValue[1][0]);
            test(r.returnValue[2].length == 1);
            test(r.returnValue[2][0]);
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

            MyClass.OpShortIntLongSSResult r = p.opShortIntLongSS(ssi, isi, lsi);
            test(r.returnValue.length == 1);
            test(r.returnValue[0].length == 2);
            test(r.returnValue[0][0] == 496);
            test(r.returnValue[0][1] == 1729);
            test(r.p4.length == 3);
            test(r.p4[0].length == 3);
            test(r.p4[0][0] == 1);
            test(r.p4[0][1] == 2);
            test(r.p4[0][2] == 5);
            test(r.p4[1].length == 1);
            test(r.p4[1][0] == 13);
            test(r.p4[2].length == 0);
            test(r.p5.length == 2);
            test(r.p5[0].length == 1);
            test(r.p5[0][0] == 42);
            test(r.p5[1].length == 2);
            test(r.p5[1][0] == 24);
            test(r.p5[1][1] == 98);
            test(r.p6.length == 2);
            test(r.p6[0].length == 2);
            test(r.p6[0][0] == 496);
            test(r.p6[0][1] == 1729);
            test(r.p6[1].length == 2);
            test(r.p6[1][0] == 496);
            test(r.p6[1][1] == 1729);
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

            MyClass.OpFloatDoubleSSResult r = p.opFloatDoubleSS(fsi, dsi);
            test(r.p3.length == 3);
            test(r.p3[0].length == 1);
            test(r.p3[0][0] == 3.14f);
            test(r.p3[1].length == 1);
            test(r.p3[1][0] == 1.11f);
            test(r.p3[2].length == 0);
            test(r.p4.length == 1);
            test(r.p4[0].length == 3);
            test(r.p4[0][0] == 1.1E10);
            test(r.p4[0][1] == 1.2E10);
            test(r.p4[0][2] == 1.3E10);
            test(r.returnValue.length == 2);
            test(r.returnValue[0].length == 3);
            test(r.returnValue[0][0] == 1.1E10);
            test(r.returnValue[0][1] == 1.2E10);
            test(r.returnValue[0][2] == 1.3E10);
            test(r.returnValue[1].length == 3);
            test(r.returnValue[1][0] == 1.1E10);
            test(r.returnValue[1][1] == 1.2E10);
            test(r.returnValue[1][2] == 1.3E10);
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

            MyClass.OpStringSSResult r = p.opStringSS(ssi1, ssi2);
            test(r.p3.length == 5);
            test(r.p3[0].length == 1);
            test(r.p3[0][0].equals("abc"));
            test(r.p3[1].length == 2);
            test(r.p3[1][0].equals("de"));
            test(r.p3[1][1].equals("fghi"));
            test(r.p3[2].length == 0);
            test(r.p3[3].length == 0);
            test(r.p3[4].length == 1);
            test(r.p3[4][0].equals("xyz"));
            test(r.returnValue.length == 3);
            test(r.returnValue[0].length == 1);
            test(r.returnValue[0][0].equals("xyz"));
            test(r.returnValue[1].length == 0);
            test(r.returnValue[2].length == 0);
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

            MyClass.OpStringSSSResult r = p.opStringSSS(sssi1, sssi2);
            test(r.p3.length == 5);
            test(r.p3[0].length == 2);
            test(r.p3[0][0].length == 2);
            test(r.p3[0][1].length == 1);
            test(r.p3[1].length == 1);
            test(r.p3[1][0].length == 1);
            test(r.p3[2].length == 2);
            test(r.p3[2][0].length == 2);
            test(r.p3[2][1].length == 1);
            test(r.p3[3].length == 1);
            test(r.p3[3][0].length == 1);
            test(r.p3[4].length == 0);
            test(r.p3[0][0][0].equals("abc"));
            test(r.p3[0][0][1].equals("de"));
            test(r.p3[0][1][0].equals("xyz"));
            test(r.p3[1][0][0].equals("hello"));
            test(r.p3[2][0][0].equals(""));
            test(r.p3[2][0][1].equals(""));
            test(r.p3[2][1][0].equals("abcd"));
            test(r.p3[3][0][0].equals(""));

            test(r.returnValue.length == 3);
            test(r.returnValue[0].length == 0);
            test(r.returnValue[1].length == 1);
            test(r.returnValue[1][0].length == 1);
            test(r.returnValue[2].length == 2);
            test(r.returnValue[2][0].length == 2);
            test(r.returnValue[2][1].length == 1);
            test(r.returnValue[1][0][0].equals(""));
            test(r.returnValue[2][0][0].equals(""));
            test(r.returnValue[2][0][1].equals(""));
            test(r.returnValue[2][1][0].equals("abcd"));
        }

        {
            Map<Byte, Boolean> di1 = new HashMap<>();
            di1.put((byte) 10, Boolean.TRUE);
            di1.put((byte) 100, Boolean.FALSE);
            Map<Byte, Boolean> di2 = new HashMap<>();
            di2.put((byte) 10, Boolean.TRUE);
            di2.put((byte) 11, Boolean.FALSE);
            di2.put((byte) 101, Boolean.TRUE);

            MyClass.OpByteBoolDResult r = p.opByteBoolD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get((byte) 10));
            test(!r.returnValue.get((byte) 11));
            test(!r.returnValue.get((byte) 100));
            test(r.returnValue.get((byte) 101));
        }

        {
            Map<Short, Integer> di1 = new HashMap<>();
            di1.put((short) 110, -1);
            di1.put((short) 1100, 123123);
            Map<Short, Integer> di2 = new HashMap<>();
            di2.put((short) 110, -1);
            di2.put((short) 111, -100);
            di2.put((short) 1101, 0);

            MyClass.OpShortIntDResult r = p.opShortIntD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get((short) 110) == -1);
            test(r.returnValue.get((short) 111) == -100);
            test(r.returnValue.get((short) 1100) == 123123);
            test(r.returnValue.get((short) 1101) == 0);
        }

        {
            Map<Long, Float> di1 = new HashMap<>();
            di1.put(999999110L, -1.1f);
            di1.put(999999111L, 123123.2f);
            Map<Long, Float> di2 = new HashMap<>();
            di2.put(999999110L, -1.1f);
            di2.put(999999120L, -100.4f);
            di2.put(999999130L, 0.5f);

            MyClass.OpLongFloatDResult r = p.opLongFloatD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get(999999110L) == -1.1f);
            test(r.returnValue.get(999999120L) == -100.4f);
            test(r.returnValue.get(999999111L) == 123123.2f);
            test(r.returnValue.get(999999130L) == 0.5f);
        }

        {
            Map<String, String> di1 = new HashMap<>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            Map<String, String> di2 = new HashMap<>();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");

            MyClass.OpStringStringDResult r = p.opStringStringD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get("foo").equals("abc -1.1"));
            test(r.returnValue.get("FOO").equals("abc -100.4"));
            test(r.returnValue.get("bar").equals("abc 123123.2"));
            test(r.returnValue.get("BAR").equals("abc 0.5"));
        }

        {
            Map<String, MyEnum> di1 = new HashMap<>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            Map<String, MyEnum> di2 = new HashMap<>();
            di2.put("abc", MyEnum.enum1);
            di2.put("qwerty", MyEnum.enum3);
            di2.put("Hello!!", MyEnum.enum2);

            MyClass.OpStringMyEnumDResult r = p.opStringMyEnumD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get("abc") == MyEnum.enum1);
            test(r.returnValue.get("qwerty") == MyEnum.enum3);
            test(r.returnValue.get("") == MyEnum.enum2);
            test(r.returnValue.get("Hello!!") == MyEnum.enum2);
        }

        {
            Map<MyEnum, String> di1 = new HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            Map<MyEnum, String> di2 = new HashMap<>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            MyClass.OpMyEnumStringDResult r = p.opMyEnumStringD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 3);
            test(r.returnValue.get(MyEnum.enum1).equals("abc"));
            test(r.returnValue.get(MyEnum.enum2).equals("Hello!!"));
            test(r.returnValue.get(MyEnum.enum3).equals("qwerty"));
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

            MyClass.OpMyStructMyEnumDResult r = p.opMyStructMyEnumD(di1, di2);

            test(r.p3.equals(di1));
            test(r.returnValue.size() == 4);
            test(r.returnValue.get(s11) == MyEnum.enum1);
            test(r.returnValue.get(s12) == MyEnum.enum2);
            test(r.returnValue.get(s22) == MyEnum.enum3);
            test(r.returnValue.get(s23) == MyEnum.enum2);
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

            MyClass.OpByteBoolDSResult r = p.opByteBoolDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get((byte) 10));
            test(!r.returnValue.get(0).get((byte) 11));
            test(r.returnValue.get(0).get((byte) 101));
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get((byte) 10));
            test(!r.returnValue.get(1).get((byte) 100));

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 2);
            test(!r.p3.get(0).get((byte) 100));
            test(!r.p3.get(0).get((byte) 101));
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get((byte) 10));
            test(!r.p3.get(1).get((byte) 100));
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get((byte) 10));
            test(!r.p3.get(2).get((byte) 11));
            test(r.p3.get(2).get((byte) 101));
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

            MyClass.OpShortIntDSResult r = p.opShortIntDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get((short) 110) == -1);
            test(r.returnValue.get(0).get((short) 111) == -100);
            test(r.returnValue.get(0).get((short) 1101) == 0);
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get((short) 110) == -1);
            test(r.returnValue.get(1).get((short) 1100) == 123123);

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get((short) 100) == -1001);
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get((short) 110) == -1);
            test(r.p3.get(1).get((short) 1100) == 123123);
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get((short) 110) == -1);
            test(r.p3.get(2).get((short) 111) == -100);
            test(r.p3.get(2).get((short) 1101) == 0);
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

            MyClass.OpLongFloatDSResult r = p.opLongFloatDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get(999999110L) == -1.1f);
            test(r.returnValue.get(0).get(999999120L) == -100.4f);
            test(r.returnValue.get(0).get(999999130L) == 0.5f);
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get(999999110L) == -1.1f);
            test(r.returnValue.get(1).get(999999111L) == 123123.2f);

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get(999999140L) == 3.14f);
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get(999999110L) == -1.1f);
            test(r.p3.get(1).get(999999111L) == 123123.2f);
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get(999999110L) == -1.1f);
            test(r.p3.get(2).get(999999120L) == -100.4f);
            test(r.p3.get(2).get(999999130L) == 0.5f);
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

            MyClass.OpStringStringDSResult r = p.opStringStringDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get("foo").equals("abc -1.1"));
            test(r.returnValue.get(0).get("FOO").equals("abc -100.4"));
            test(r.returnValue.get(0).get("BAR").equals("abc 0.5"));
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get("foo").equals("abc -1.1"));
            test(r.returnValue.get(1).get("bar").equals("abc 123123.2"));

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get("f00").equals("ABC -3.14"));
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get("foo").equals("abc -1.1"));
            test(r.p3.get(1).get("bar").equals("abc 123123.2"));
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get("foo").equals("abc -1.1"));
            test(r.p3.get(2).get("FOO").equals("abc -100.4"));
            test(r.p3.get(2).get("BAR").equals("abc 0.5"));
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

            MyClass.OpStringMyEnumDSResult r = p.opStringMyEnumDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get("abc") == MyEnum.enum1);
            test(r.returnValue.get(0).get("qwerty") == MyEnum.enum3);
            test(r.returnValue.get(0).get("Hello!!") == MyEnum.enum2);
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get("abc") == MyEnum.enum1);
            test(r.returnValue.get(1).get("") == MyEnum.enum2);

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get("Goodbye") == MyEnum.enum1);
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get("abc") == MyEnum.enum1);
            test(r.p3.get(1).get("") == MyEnum.enum2);
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get("abc") == MyEnum.enum1);
            test(r.p3.get(2).get("qwerty") == MyEnum.enum3);
            test(r.p3.get(2).get("Hello!!") == MyEnum.enum2);
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

            MyClass.OpMyEnumStringDSResult r = p.opMyEnumStringDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 2);
            test(r.returnValue.get(0).get(MyEnum.enum2).equals("Hello!!"));
            test(r.returnValue.get(0).get(MyEnum.enum3).equals("qwerty"));
            test(r.returnValue.get(1).size() == 1);
            test(r.returnValue.get(1).get(MyEnum.enum1).equals("abc"));

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get(MyEnum.enum1).equals("Goodbye"));
            test(r.p3.get(1).size() == 1);
            test(r.p3.get(1).get(MyEnum.enum1).equals("abc"));
            test(r.p3.get(2).size() == 2);
            test(r.p3.get(2).get(MyEnum.enum2).equals("Hello!!"));
            test(r.p3.get(2).get(MyEnum.enum3).equals("qwerty"));
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

            MyClass.OpMyStructMyEnumDSResult r = p.opMyStructMyEnumDS(dsi1, dsi2);

            test(r.returnValue.size() == 2);
            test(r.returnValue.get(0).size() == 3);
            test(r.returnValue.get(0).get(s11) == MyEnum.enum1);
            test(r.returnValue.get(0).get(s22) == MyEnum.enum3);
            test(r.returnValue.get(0).get(s23) == MyEnum.enum2);
            test(r.returnValue.get(1).size() == 2);
            test(r.returnValue.get(1).get(s11) == MyEnum.enum1);
            test(r.returnValue.get(1).get(s12) == MyEnum.enum2);

            test(r.p3.size() == 3);
            test(r.p3.get(0).size() == 1);
            test(r.p3.get(0).get(s23) == MyEnum.enum3);
            test(r.p3.get(1).size() == 2);
            test(r.p3.get(1).get(s11) == MyEnum.enum1);
            test(r.p3.get(1).get(s12) == MyEnum.enum2);
            test(r.p3.get(2).size() == 3);
            test(r.p3.get(2).get(s11) == MyEnum.enum1);
            test(r.p3.get(2).get(s22) == MyEnum.enum3);
            test(r.p3.get(2).get(s23) == MyEnum.enum2);
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

            MyClass.OpByteByteSDResult r = p.opByteByteSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get((byte) 0xf1).length == 2);
            test(r.p3.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(r.p3.get((byte) 0xf1)[1] == (byte) 0xf3);
            test(r.returnValue.size() == 3);
            test(r.returnValue.get((byte) 0x01).length == 2);
            test(r.returnValue.get((byte) 0x01)[0] == (byte) 0x01);
            test(r.returnValue.get((byte) 0x01)[1] == (byte) 0x11);
            test(r.returnValue.get((byte) 0x22).length == 1);
            test(r.returnValue.get((byte) 0x22)[0] == (byte) 0x12);
            test(r.returnValue.get((byte) 0xf1).length == 2);
            test(r.returnValue.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(r.returnValue.get((byte) 0xf1)[1] == (byte) 0xf3);
        }

        {
            java.util.Map<Boolean, boolean[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Boolean, boolean[]> sdi2 = new java.util.HashMap<>();

            final boolean[] si1 = {true, false};
            final boolean[] si2 = {false, true, true};

            sdi1.put(false, si1);
            sdi1.put(true, si2);
            sdi2.put(false, si1);

            MyClass.OpBoolBoolSDResult r = p.opBoolBoolSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get(false).length == 2);
            test(r.p3.get(false)[0]);
            test(!r.p3.get(false)[1]);
            test(r.returnValue.size() == 2);
            test(r.returnValue.get(false).length == 2);
            test(r.returnValue.get(false)[0]);
            test(!r.returnValue.get(false)[1]);
            test(r.returnValue.get(true).length == 3);
            test(!r.returnValue.get(true)[0]);
            test(r.returnValue.get(true)[1]);
            test(r.returnValue.get(true)[2]);
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

            MyClass.OpShortShortSDResult r = p.opShortShortSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get((short) 4).length == 2);
            test(r.p3.get((short) 4)[0] == 6);
            test(r.p3.get((short) 4)[1] == 7);
            test(r.returnValue.size() == 3);
            test(r.returnValue.get((short) 1).length == 3);
            test(r.returnValue.get((short) 1)[0] == 1);
            test(r.returnValue.get((short) 1)[1] == 2);
            test(r.returnValue.get((short) 1)[2] == 3);
            test(r.returnValue.get((short) 2).length == 2);
            test(r.returnValue.get((short) 2)[0] == 4);
            test(r.returnValue.get((short) 2)[1] == 5);
            test(r.returnValue.get((short) 4).length == 2);
            test(r.returnValue.get((short) 4)[0] == 6);
            test(r.returnValue.get((short) 4)[1] == 7);
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

            MyClass.OpIntIntSDResult r = p.opIntIntSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get(400).length == 2);
            test(r.p3.get(400)[0] == 600);
            test(r.p3.get(400)[1] == 700);
            test(r.returnValue.size() == 3);
            test(r.returnValue.get(100).length == 3);
            test(r.returnValue.get(100)[0] == 100);
            test(r.returnValue.get(100)[1] == 200);
            test(r.returnValue.get(100)[2] == 300);
            test(r.returnValue.get(200).length == 2);
            test(r.returnValue.get(200)[0] == 400);
            test(r.returnValue.get(200)[1] == 500);
            test(r.returnValue.get(400).length == 2);
            test(r.returnValue.get(400)[0] == 600);
            test(r.returnValue.get(400)[1] == 700);
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

            MyClass.OpLongLongSDResult r = p.opLongLongSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get(999999992L).length == 2);
            test(r.p3.get(999999992L)[0] == 999999110L);
            test(r.p3.get(999999992L)[1] == 999999120L);
            test(r.returnValue.size() == 3);
            test(r.returnValue.get(999999990L).length == 3);
            test(r.returnValue.get(999999990L)[0] == 999999110L);
            test(r.returnValue.get(999999990L)[1] == 999999111L);
            test(r.returnValue.get(999999990L)[2] == 999999110L);
            test(r.returnValue.get(999999991L).length == 2);
            test(r.returnValue.get(999999991L)[0] == 999999120L);
            test(r.returnValue.get(999999991L)[1] == 999999130L);
            test(r.returnValue.get(999999992L).length == 2);
            test(r.returnValue.get(999999992L)[0] == 999999110L);
            test(r.returnValue.get(999999992L)[1] == 999999120L);
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

            MyClass.OpStringFloatSDResult r = p.opStringFloatSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get("aBc").length == 2);
            test(r.p3.get("aBc")[0] == -3.14f);
            test(r.p3.get("aBc")[1] == 3.14f);
            test(r.returnValue.size() == 3);
            test(r.returnValue.get("abc").length == 3);
            test(r.returnValue.get("abc")[0] == -1.1f);
            test(r.returnValue.get("abc")[1] == 123123.2f);
            test(r.returnValue.get("abc")[2] == 100.0f);
            test(r.returnValue.get("ABC").length == 2);
            test(r.returnValue.get("ABC")[0] == 42.24f);
            test(r.returnValue.get("ABC")[1] == -1.61f);
            test(r.returnValue.get("aBc").length == 2);
            test(r.returnValue.get("aBc")[0] == -3.14f);
            test(r.returnValue.get("aBc")[1] == 3.14f);
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

            MyClass.OpStringDoubleSDResult r = p.opStringDoubleSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get("").length == 2);
            test(r.p3.get("")[0] == 1.6E10);
            test(r.p3.get("")[1] == 1.7E10);
            test(r.returnValue.size()== 3);
            test(r.returnValue.get("Hello!!").length == 3);
            test(r.returnValue.get("Hello!!")[0] == 1.1E10);
            test(r.returnValue.get("Hello!!")[1] == 1.2E10);
            test(r.returnValue.get("Hello!!")[2] == 1.3E10);
            test(r.returnValue.get("Goodbye").length == 2);
            test(r.returnValue.get("Goodbye")[0] == 1.4E10);
            test(r.returnValue.get("Goodbye")[1] == 1.5E10);
            test(r.returnValue.get("").length== 2);
            test(r.returnValue.get("")[0] == 1.6E10);
            test(r.returnValue.get("")[1] == 1.7E10);
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

            MyClass.OpStringStringSDResult r = p.opStringStringSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get("ghi").length== 2);
            test(r.p3.get("ghi")[0].equals("and"));
            test(r.p3.get("ghi")[1].equals("xor"));
            test(r.returnValue.size()== 3);
            test(r.returnValue.get("abc").length == 3);
            test(r.returnValue.get("abc")[0].equals("abc"));
            test(r.returnValue.get("abc")[1].equals("de"));
            test(r.returnValue.get("abc")[2].equals("fghi"));
            test(r.returnValue.get("def").length == 2);
            test(r.returnValue.get("def")[0].equals("xyz"));
            test(r.returnValue.get("def")[1].equals("or"));
            test(r.returnValue.get("ghi").length == 2);
            test(r.returnValue.get("ghi")[0].equals("and"));
            test(r.returnValue.get("ghi")[1].equals("xor"));
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

            MyClass.OpMyEnumMyEnumSDResult r = p.opMyEnumMyEnumSD(sdi1, sdi2);

            test(r.p3.size() == 1);
            test(r.p3.get(MyEnum.enum1).length == 2);
            test(r.p3.get(MyEnum.enum1)[0] == MyEnum.enum3);
            test(r.p3.get(MyEnum.enum1)[1] == MyEnum.enum3);
            test(r.returnValue.size()== 3);
            test(r.returnValue.get(MyEnum.enum3).length == 3);
            test(r.returnValue.get(MyEnum.enum3)[0] == MyEnum.enum1);
            test(r.returnValue.get(MyEnum.enum3)[1] == MyEnum.enum1);
            test(r.returnValue.get(MyEnum.enum3)[2] == MyEnum.enum2);
            test(r.returnValue.get(MyEnum.enum2).length == 2);
            test(r.returnValue.get(MyEnum.enum2)[0] == MyEnum.enum1);
            test(r.returnValue.get(MyEnum.enum2)[1] == MyEnum.enum2);
            test(r.returnValue.get(MyEnum.enum1).length == 2);
            test(r.returnValue.get(MyEnum.enum1)[0] == MyEnum.enum3);
            test(r.returnValue.get(MyEnum.enum1)[1] == MyEnum.enum3);
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
                MyClassPrx p2 = MyClassPrx.checkedCast(p.ice_context(ctx));
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

                MyClassPrx p3 = MyClassPrx.uncheckedCast(ic.stringToProxy("test:" + app.getTestEndpoint(0)));

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

                p3 = p3.ice_context(prxContext);

                ic.getImplicitContext().setContext(null);
                test(p3.opContext().equals(prxContext));

                ic.getImplicitContext().setContext(ctx);
                test(p3.opContext().equals(combined));

                test(ic.getImplicitContext().remove("one").equals("ONE"));

                if(impls[i].equals("PerThread"))
                {
                    Thread thread = new PerThreadContextInvokeThread(p3.ice_context(null));
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

        MyDerivedClassPrx d = MyDerivedClassPrx.uncheckedCast(p);
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

        {
            Structure p1 = p.opMStruct1();
            p1.e = MyEnum.enum3;
            MyClass.OpMStruct2Result r = p.opMStruct2(p1);
            test(r.p2.e == p1.e && r.returnValue.e == p1.e);
        }

        {
            p.opMSeq1();

            String[] p1 = new String[1];
            p1[0] = "test";
            MyClass.OpMSeq2Result r = p.opMSeq2(p1);
            test(java.util.Arrays.equals(r.p2, p1) && java.util.Arrays.equals(r.returnValue, p1));
        }

        {
            p.opMDict1();

            java.util.Map<String, String> p1 = new java.util.HashMap<>();
            p1.put("test", "test");
            MyClass.OpMDict2Result r = p.opMDict2(p1);
            test(r.p2.equals(p1) && r.returnValue.equals(p1));
        }
    }
}
