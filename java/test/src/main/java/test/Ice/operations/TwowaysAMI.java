// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.LocalException;

import test.Ice.operations.Test.*;

class TwowaysAMI
{
    private static void test(boolean b)
    {
        if(!b)
        {
            new Throwable().printStackTrace();
            //
            // Exceptions raised by callbacks are swallowed by CompletableFuture.
            //
            throw new RuntimeException();
        }
    }

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void check()
        {
            while(!_called)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
            }

            _called = false;
        }

        public synchronized void called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    static void twowaysAMI(test.Util.Application app, MyClassPrx p)
    {
        com.zeroc.Ice.Communicator communicator = app.communicator();

        {
            Callback cb = new Callback();
            p.ice_pingAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.ice_isAAsync(MyClass.ice_staticId()).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.ice_idAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.equals(MyDerivedClass.ice_staticId()));
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.ice_idsAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.length == 3);
                    cb.called();
                });
            cb.check();
        }

        {
            p.opVoidAsync().join();
        }

        {
            Callback cb = new Callback();
            p.opVoidAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opByteAsync((byte)0xff, (byte)0x0f).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3 == (byte)0xf0);
                    test(result.returnValue == (byte)0xff);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opBoolAsync(true, false).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3);
                    test(!result.returnValue);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opShortIntLongAsync((short)10, 11, 12L).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p4 == 10);
                    test(result.p5 == 11);
                    test(result.p6 == 12);
                    test(result.returnValue == 12);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opFloatDoubleAsync(3.14f, 1.1E10).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3 == 3.14f);
                    test(result.p4 == 1.1E10);
                    test(result.returnValue == 1.1E10);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opStringAsync("hello", "world").whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals("world hello"));
                    test(result.returnValue.equals("hello world"));
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opMyEnumAsync(MyEnum.enum2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p2 == MyEnum.enum2);
                    test(result.returnValue == MyEnum.enum3);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opMyClassAsync(p).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p2.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("test")));
                    test(result.p3.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("noSuchIdentity")));
                    test(result.returnValue.ice_getIdentity().equals(com.zeroc.Ice.Util.stringToIdentity("test")));
                    // We can't do the callbacks below in connection serialization mode.
                    if(communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
                    {
                        result.returnValue.opVoid();
                        result.p2.opVoid();
                        try
                        {
                            result.p3.opVoid();
                            test(false);
                        }
                        catch(com.zeroc.Ice.ObjectNotExistException e)
                        {
                        }
                    }
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStructAsync(si1, si2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.p == null);
                    test(result.returnValue.e == MyEnum.enum2);
                    test(result.returnValue.s.s.equals("def"));
                    test(result.p3.e == MyEnum.enum3);
                    test(result.p3.s.s.equals("a new string"));
                    // We can't do the callbacks below in connection serialization mode.
                    if(communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
                    {
                        result.p3.p.opVoid();
                    }
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opByteSAsync(bsi1, bsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 4);
                    test(result.p3[0] == (byte)0x22);
                    test(result.p3[1] == (byte)0x12);
                    test(result.p3[2] == (byte)0x11);
                    test(result.p3[3] == (byte)0x01);
                    test(result.returnValue.length == 8);
                    test(result.returnValue[0] == (byte)0x01);
                    test(result.returnValue[1] == (byte)0x11);
                    test(result.returnValue[2] == (byte)0x12);
                    test(result.returnValue[3] == (byte)0x22);
                    test(result.returnValue[4] == (byte)0xf1);
                    test(result.returnValue[5] == (byte)0xf2);
                    test(result.returnValue[6] == (byte)0xf3);
                    test(result.returnValue[7] == (byte)0xf4);
                    cb.called();
                });
            cb.check();
        }

        {
            final boolean[] bsi1 = { true, true, false };
            final boolean[] bsi2 = { false };

            Callback cb = new Callback();
            p.opBoolSAsync(bsi1, bsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 4);
                    test(result.p3[0]);
                    test(result.p3[1]);
                    test(!result.p3[2]);
                    test(!result.p3[3]);
                    test(result.returnValue.length == 3);
                    test(!result.returnValue[0]);
                    test(result.returnValue[1]);
                    test(result.returnValue[2]);
                    cb.called();
                });
            cb.check();
        }

        {
            final short[] ssi = { 1, 2, 3 };
            final int[] isi = { 5, 6, 7, 8 };
            final long[] lsi = { 10, 30, 20 };

            Callback cb = new Callback();
            p.opShortIntLongSAsync(ssi, isi, lsi).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p4.length == 3);
                    test(result.p4[0] == 1);
                    test(result.p4[1] == 2);
                    test(result.p4[2] == 3);
                    test(result.p5.length == 4);
                    test(result.p5[0] == 8);
                    test(result.p5[1] == 7);
                    test(result.p5[2] == 6);
                    test(result.p5[3] == 5);
                    test(result.p6.length == 6);
                    test(result.p6[0] == 10);
                    test(result.p6[1] == 30);
                    test(result.p6[2] == 20);
                    test(result.p6[3] == 10);
                    test(result.p6[4] == 30);
                    test(result.p6[5] == 20);
                    test(result.returnValue.length == 3);
                    test(result.returnValue[0] == 10);
                    test(result.returnValue[1] == 30);
                    test(result.returnValue[2] == 20);
                    cb.called();
                });
            cb.check();
        }

        {
            final float[] fsi = { 3.14f, 1.11f };
            final double[] dsi = { 1.1E10, 1.2E10, 1.3E10 };

            Callback cb = new Callback();
            p.opFloatDoubleSAsync(fsi, dsi).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 2);
                    test(result.p3[0] == 3.14f);
                    test(result.p3[1] == 1.11f);
                    test(result.p4.length == 3);
                    test(result.p4[0] == 1.3E10);
                    test(result.p4[1] == 1.2E10);
                    test(result.p4[2] == 1.1E10);
                    test(result.returnValue.length == 5);
                    test(result.returnValue[0] == 1.1E10);
                    test(result.returnValue[1] == 1.2E10);
                    test(result.returnValue[2] == 1.3E10);
                    test((float)result.returnValue[3] == 3.14f);
                    test((float)result.returnValue[4] == 1.11f);
                    cb.called();
                });
            cb.check();
        }

        {
            final String[] ssi1 = { "abc", "de", "fghi" };
            final String[] ssi2 = { "xyz" };

            Callback cb = new Callback();
            p.opStringSAsync(ssi1, ssi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 4);
                    test(result.p3[0].equals("abc"));
                    test(result.p3[1].equals("de"));
                    test(result.p3[2].equals("fghi"));
                    test(result.p3[3].equals("xyz"));
                    test(result.returnValue.length == 3);
                    test(result.returnValue[0].equals("fghi"));
                    test(result.returnValue[1].equals("de"));
                    test(result.returnValue[2].equals("abc"));
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opByteSSAsync(bsi1, bsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 2);
                    test(result.p3[0].length == 1);
                    test(result.p3[0][0] == (byte)0xff);
                    test(result.p3[1].length == 3);
                    test(result.p3[1][0] == (byte)0x01);
                    test(result.p3[1][1] == (byte)0x11);
                    test(result.p3[1][2] == (byte)0x12);
                    test(result.returnValue.length == 4);
                    test(result.returnValue[0].length == 3);
                    test(result.returnValue[0][0] == (byte)0x01);
                    test(result.returnValue[0][1] == (byte)0x11);
                    test(result.returnValue[0][2] == (byte)0x12);
                    test(result.returnValue[1].length == 1);
                    test(result.returnValue[1][0] == (byte)0xff);
                    test(result.returnValue[2].length == 1);
                    test(result.returnValue[2][0] == (byte)0x0e);
                    test(result.returnValue[3].length == 2);
                    test(result.returnValue[3][0] == (byte)0xf2);
                    test(result.returnValue[3][1] == (byte)0xf1);
                    cb.called();
                });
            cb.check();
        }

        {
            final boolean[][] bsi1 =
                {
                    { true },
                    { false },
                    { true, true}
                };

            final boolean[][] bsi2 =
                {
                    { false, false, true }
                };

            Callback cb = new Callback();
            p.opBoolSSAsync(bsi1, bsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 4);
                    test(result.p3[0].length == 1);
                    test(result.p3[0][0]);
                    test(result.p3[1].length == 1);
                    test(!result.p3[1][0]);
                    test(result.p3[2].length == 2);
                    test(result.p3[2][0]);
                    test(result.p3[2][1]);
                    test(result.p3[3].length == 3);
                    test(!result.p3[3][0]);
                    test(!result.p3[3][1]);
                    test(result.p3[3][2]);
                    test(result.returnValue.length == 3);
                    test(result.returnValue[0].length == 2);
                    test(result.returnValue[0][0]);
                    test(result.returnValue[0][1]);
                    test(result.returnValue[1].length == 1);
                    test(!result.returnValue[1][0]);
                    test(result.returnValue[2].length == 1);
                    test(result.returnValue[2][0]);
                    cb.called();
                });
            cb.check();
        }

        {
            final short[][] ssi=
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

            Callback cb = new Callback();
            p.opShortIntLongSSAsync(ssi, isi, lsi).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.length == 1);
                    test(result.returnValue[0].length == 2);
                    test(result.returnValue[0][0] == 496);
                    test(result.returnValue[0][1] == 1729);
                    test(result.p4.length == 3);
                    test(result.p4[0].length == 3);
                    test(result.p4[0][0] == 1);
                    test(result.p4[0][1] == 2);
                    test(result.p4[0][2] == 5);
                    test(result.p4[1].length == 1);
                    test(result.p4[1][0] == 13);
                    test(result.p4[2].length == 0);
                    test(result.p5.length == 2);
                    test(result.p5[0].length == 1);
                    test(result.p5[0][0] == 42);
                    test(result.p5[1].length == 2);
                    test(result.p5[1][0] == 24);
                    test(result.p5[1][1] == 98);
                    test(result.p6.length == 2);
                    test(result.p6[0].length == 2);
                    test(result.p6[0][0] == 496);
                    test(result.p6[0][1] == 1729);
                    test(result.p6[1].length == 2);
                    test(result.p6[1][0] == 496);
                    test(result.p6[1][1] == 1729);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opFloatDoubleSSAsync(fsi, dsi).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 3);
                    test(result.p3[0].length == 1);
                    test(result.p3[0][0] == 3.14f);
                    test(result.p3[1].length == 1);
                    test(result.p3[1][0] == 1.11f);
                    test(result.p3[2].length == 0);
                    test(result.p4.length == 1);
                    test(result.p4[0].length == 3);
                    test(result.p4[0][0] == 1.1E10);
                    test(result.p4[0][1] == 1.2E10);
                    test(result.p4[0][2] == 1.3E10);
                    test(result.returnValue.length == 2);
                    test(result.returnValue[0].length == 3);
                    test(result.returnValue[0][0] == 1.1E10);
                    test(result.returnValue[0][1] == 1.2E10);
                    test(result.returnValue[0][2] == 1.3E10);
                    test(result.returnValue[1].length == 3);
                    test(result.returnValue[1][0] == 1.1E10);
                    test(result.returnValue[1][1] == 1.2E10);
                    test(result.returnValue[1][2] == 1.3E10);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStringSSAsync(ssi1, ssi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 5);
                    test(result.p3[0].length == 1);
                    test(result.p3[0][0].equals("abc"));
                    test(result.p3[1].length == 2);
                    test(result.p3[1][0].equals("de"));
                    test(result.p3[1][1].equals("fghi"));
                    test(result.p3[2].length == 0);
                    test(result.p3[3].length == 0);
                    test(result.p3[4].length == 1);
                    test(result.p3[4][0].equals("xyz"));
                    test(result.returnValue.length == 3);
                    test(result.returnValue[0].length == 1);
                    test(result.returnValue[0][0].equals("xyz"));
                    test(result.returnValue[1].length == 0);
                    test(result.returnValue[2].length == 0);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStringSSSAsync(sssi1, sssi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.length == 5);
                    test(result.p3[0].length == 2);
                    test(result.p3[0][0].length == 2);
                    test(result.p3[0][1].length == 1);
                    test(result.p3[1].length == 1);
                    test(result.p3[1][0].length == 1);
                    test(result.p3[2].length == 2);
                    test(result.p3[2][0].length == 2);
                    test(result.p3[2][1].length == 1);
                    test(result.p3[3].length == 1);
                    test(result.p3[3][0].length == 1);
                    test(result.p3[4].length == 0);
                    test(result.p3[0][0][0].equals("abc"));
                    test(result.p3[0][0][1].equals("de"));
                    test(result.p3[0][1][0].equals("xyz"));
                    test(result.p3[1][0][0].equals("hello"));
                    test(result.p3[2][0][0].equals(""));
                    test(result.p3[2][0][1].equals(""));
                    test(result.p3[2][1][0].equals("abcd"));
                    test(result.p3[3][0][0].equals(""));

                    test(result.returnValue.length == 3);
                    test(result.returnValue[0].length == 0);
                    test(result.returnValue[1].length == 1);
                    test(result.returnValue[1][0].length == 1);
                    test(result.returnValue[2].length == 2);
                    test(result.returnValue[2][0].length == 2);
                    test(result.returnValue[2][1].length == 1);
                    test(result.returnValue[1][0][0].equals(""));
                    test(result.returnValue[2][0][0].equals(""));
                    test(result.returnValue[2][0][1].equals(""));
                    test(result.returnValue[2][1][0].equals("abcd"));
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<Byte, Boolean> di1 = new java.util.HashMap<>();
            di1.put((byte)10, Boolean.TRUE);
            di1.put((byte)100, Boolean.FALSE);
            java.util.Map<Byte, Boolean> di2 = new java.util.HashMap<>();
            di2.put((byte)10, Boolean.TRUE);
            di2.put((byte)11, Boolean.FALSE);
            di2.put((byte)101, Boolean.TRUE);

            Callback cb = new Callback();
            p.opByteBoolDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get((byte) 10));
                    test(!result.returnValue.get((byte) 11));
                    test(!result.returnValue.get((byte) 100));
                    test(result.returnValue.get((byte) 101));
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<Short, Integer> di1 = new java.util.HashMap<>();
            di1.put((short)110, -1);
            di1.put((short)1100, 123123);
            java.util.Map<Short, Integer> di2 = new java.util.HashMap<>();
            di2.put((short)110, -1);
            di2.put((short)111, -100);
            di2.put((short)1101, 0);

            Callback cb = new Callback();
            p.opShortIntDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get((short) 110) == -1);
                    test(result.returnValue.get((short) 111) == -100);
                    test(result.returnValue.get((short) 1100) == 123123);
                    test(result.returnValue.get((short) 1101) == 0);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<>();
            di1.put(999999110L, -1.1f);
            di1.put(999999111L, 123123.2f);
            java.util.Map<Long, Float> di2 = new java.util.HashMap<>();
            di2.put(999999110L, -1.1f);
            di2.put(999999120L, -100.4f);
            di2.put(999999130L, 0.5f);

            Callback cb = new Callback();
            p.opLongFloatDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get(999999110L) == -1.1f);
                    test(result.returnValue.get(999999120L) == -100.4f);
                    test(result.returnValue.get(999999111L) == 123123.2f);
                    test(result.returnValue.get(999999130L) == 0.5f);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<String, String> di1 = new java.util.HashMap<>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            java.util.Map<String, String> di2 = new java.util.HashMap<>();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");

            Callback cb = new Callback();
            p.opStringStringDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get("foo").equals("abc -1.1"));
                    test(result.returnValue.get("FOO").equals("abc -100.4"));
                    test(result.returnValue.get("bar").equals("abc 123123.2"));
                    test(result.returnValue.get("BAR").equals("abc 0.5"));
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<String, MyEnum> di1 = new java.util.HashMap<>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            java.util.Map<String, MyEnum> di2 = new java.util.HashMap<>();
            di2.put("abc", MyEnum.enum1);
            di2.put("qwerty", MyEnum.enum3);
            di2.put("Hello!!", MyEnum.enum2);

            Callback cb = new Callback();
            p.opStringMyEnumDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get("abc") == MyEnum.enum1);
                    test(result.returnValue.get("qwerty") == MyEnum.enum3);
                    test(result.returnValue.get("") == MyEnum.enum2);
                    test(result.returnValue.get("Hello!!") == MyEnum.enum2);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            java.util.Map<MyEnum, String> di2 = new java.util.HashMap<>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            Callback cb = new Callback();
            p.opMyEnumStringDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get(MyEnum.enum1).equals("abc"));
                    test(result.returnValue.get(MyEnum.enum2).equals("Hello!!"));
                    test(result.returnValue.get(MyEnum.enum3).equals("qwerty"));
                    cb.called();
                });
            cb.check();
        }

        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            java.util.Map<MyStruct, MyEnum> di1 = new java.util.HashMap<>();
            di1.put(s11, MyEnum.enum1);
            di1.put(s12, MyEnum.enum2);
            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);
            java.util.Map<MyStruct, MyEnum> di2 = new java.util.HashMap<>();
            di2.put(s11, MyEnum.enum1);
            di2.put(s22, MyEnum.enum3);
            di2.put(s23, MyEnum.enum2);

            Callback cb = new Callback();
            p.opMyStructMyEnumDAsync(di1, di2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.equals(di1));
                    test(result.returnValue.size() == 4);
                    test(result.returnValue.get(s11) == MyEnum.enum1);
                    test(result.returnValue.get(s12) == MyEnum.enum2);
                    test(result.returnValue.get(s22) == MyEnum.enum3);
                    test(result.returnValue.get(s23) == MyEnum.enum2);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opByteBoolDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get((byte) 10));
                    test(!result.returnValue.get(0).get((byte) 11));
                    test(result.returnValue.get(0).get((byte) 101));
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get((byte) 10));
                    test(!result.returnValue.get(1).get((byte) 100));

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 2);
                    test(!result.p3.get(0).get((byte) 100));
                    test(!result.p3.get(0).get((byte) 101));
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get((byte) 10));
                    test(!result.p3.get(1).get((byte) 100));
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get((byte) 10));
                    test(!result.p3.get(2).get((byte) 11));
                    test(result.p3.get(2).get((byte) 101));
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opShortIntDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get((short) 110) == -1);
                    test(result.returnValue.get(0).get((short) 111) == -100);
                    test(result.returnValue.get(0).get((short) 1101) == 0);
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get((short) 110) == -1);
                    test(result.returnValue.get(1).get((short) 1100) == 123123);

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get((short) 100) == -1001);
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get((short) 110) == -1);
                    test(result.p3.get(1).get((short) 1100) == 123123);
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get((short) 110) == -1);
                    test(result.p3.get(2).get((short) 111) == -100);
                    test(result.p3.get(2).get((short) 1101) == 0);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opLongFloatDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get(999999110L) == -1.1f);
                    test(result.returnValue.get(0).get(999999120L) == -100.4f);
                    test(result.returnValue.get(0).get(999999130L) == 0.5f);
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get(999999110L) == -1.1f);
                    test(result.returnValue.get(1).get(999999111L) == 123123.2f);

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get(999999140L) == 3.14f);
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get(999999110L) == -1.1f);
                    test(result.p3.get(1).get(999999111L) == 123123.2f);
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get(999999110L) == -1.1f);
                    test(result.p3.get(2).get(999999120L) == -100.4f);
                    test(result.p3.get(2).get(999999130L) == 0.5f);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStringStringDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get("foo").equals("abc -1.1"));
                    test(result.returnValue.get(0).get("FOO").equals("abc -100.4"));
                    test(result.returnValue.get(0).get("BAR").equals("abc 0.5"));
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get("foo").equals("abc -1.1"));
                    test(result.returnValue.get(1).get("bar").equals("abc 123123.2"));

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get("f00").equals("ABC -3.14"));
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get("foo").equals("abc -1.1"));
                    test(result.p3.get(1).get("bar").equals("abc 123123.2"));
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get("foo").equals("abc -1.1"));
                    test(result.p3.get(2).get("FOO").equals("abc -100.4"));
                    test(result.p3.get(2).get("BAR").equals("abc 0.5"));
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStringMyEnumDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get("abc") == MyEnum.enum1);
                    test(result.returnValue.get(0).get("qwerty") == MyEnum.enum3);
                    test(result.returnValue.get(0).get("Hello!!") == MyEnum.enum2);
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get("abc") == MyEnum.enum1);
                    test(result.returnValue.get(1).get("") == MyEnum.enum2);

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get("Goodbye") == MyEnum.enum1);
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get("abc") == MyEnum.enum1);
                    test(result.p3.get(1).get("") == MyEnum.enum2);
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get("abc") == MyEnum.enum1);
                    test(result.p3.get(2).get("qwerty") == MyEnum.enum3);
                    test(result.p3.get(2).get("Hello!!") == MyEnum.enum2);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opMyEnumStringDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 2);
                    test(result.returnValue.get(0).get(MyEnum.enum2).equals("Hello!!"));
                    test(result.returnValue.get(0).get(MyEnum.enum3).equals("qwerty"));
                    test(result.returnValue.get(1).size() == 1);
                    test(result.returnValue.get(1).get(MyEnum.enum1).equals("abc"));

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get(MyEnum.enum1).equals("Goodbye"));
                    test(result.p3.get(1).size() == 1);
                    test(result.p3.get(1).get(MyEnum.enum1).equals("abc"));
                    test(result.p3.get(2).size() == 2);
                    test(result.p3.get(2).get(MyEnum.enum2).equals("Hello!!"));
                    test(result.p3.get(2).get(MyEnum.enum3).equals("qwerty"));
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opMyStructMyEnumDSAsync(dsi1, dsi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(0).size() == 3);
                    test(result.returnValue.get(0).get(s11) == MyEnum.enum1);
                    test(result.returnValue.get(0).get(s22) == MyEnum.enum3);
                    test(result.returnValue.get(0).get(s23) == MyEnum.enum2);
                    test(result.returnValue.get(1).size() == 2);
                    test(result.returnValue.get(1).get(s11) == MyEnum.enum1);
                    test(result.returnValue.get(1).get(s12) == MyEnum.enum2);

                    test(result.p3.size() == 3);
                    test(result.p3.get(0).size() == 1);
                    test(result.p3.get(0).get(s23) == MyEnum.enum3);
                    test(result.p3.get(1).size() == 2);
                    test(result.p3.get(1).get(s11) == MyEnum.enum1);
                    test(result.p3.get(1).get(s12) == MyEnum.enum2);
                    test(result.p3.get(2).size() == 3);
                    test(result.p3.get(2).get(s11) == MyEnum.enum1);
                    test(result.p3.get(2).get(s22) == MyEnum.enum3);
                    test(result.p3.get(2).get(s23) == MyEnum.enum2);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opByteByteSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get((byte) 0xf1).length == 2);
                    test(result.p3.get((byte) 0xf1)[0] == (byte) 0xf2);
                    test(result.p3.get((byte) 0xf1)[1] == (byte) 0xf3);
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get((byte) 0x01).length == 2);
                    test(result.returnValue.get((byte) 0x01)[0] == (byte) 0x01);
                    test(result.returnValue.get((byte) 0x01)[1] == (byte) 0x11);
                    test(result.returnValue.get((byte) 0x22).length == 1);
                    test(result.returnValue.get((byte) 0x22)[0] == (byte) 0x12);
                    test(result.returnValue.get((byte) 0xf1).length == 2);
                    test(result.returnValue.get((byte) 0xf1)[0] == (byte) 0xf2);
                    test(result.returnValue.get((byte) 0xf1)[1] == (byte) 0xf3);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<Boolean, boolean[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Boolean, boolean[]> sdi2 = new java.util.HashMap<>();

            final boolean[] si1 = {true, false};
            final boolean[] si2 = {false, true, true};

            sdi1.put(false, si1);
            sdi1.put(true, si2);
            sdi2.put(false, si1);

            Callback cb = new Callback();
            p.opBoolBoolSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get(false).length == 2);
                    test(result.p3.get(false)[0]);
                    test(!result.p3.get(false)[1]);
                    test(result.returnValue.size() == 2);
                    test(result.returnValue.get(false).length == 2);
                    test(result.returnValue.get(false)[0]);
                    test(!result.returnValue.get(false)[1]);
                    test(result.returnValue.get(true).length == 3);
                    test(!result.returnValue.get(true)[0]);
                    test(result.returnValue.get(true)[1]);
                    test(result.returnValue.get(true)[2]);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opShortShortSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get((short) 4).length == 2);
                    test(result.p3.get((short) 4)[0] == 6);
                    test(result.p3.get((short) 4)[1] == 7);
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get((short) 1).length == 3);
                    test(result.returnValue.get((short) 1)[0] == 1);
                    test(result.returnValue.get((short) 1)[1] == 2);
                    test(result.returnValue.get((short) 1)[2] == 3);
                    test(result.returnValue.get((short) 2).length == 2);
                    test(result.returnValue.get((short) 2)[0] == 4);
                    test(result.returnValue.get((short) 2)[1] == 5);
                    test(result.returnValue.get((short) 4).length == 2);
                    test(result.returnValue.get((short) 4)[0] == 6);
                    test(result.returnValue.get((short) 4)[1] == 7);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opIntIntSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get(400).length == 2);
                    test(result.p3.get(400)[0] == 600);
                    test(result.p3.get(400)[1] == 700);
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get(100).length == 3);
                    test(result.returnValue.get(100)[0] == 100);
                    test(result.returnValue.get(100)[1] == 200);
                    test(result.returnValue.get(100)[2] == 300);
                    test(result.returnValue.get(200).length == 2);
                    test(result.returnValue.get(200)[0] == 400);
                    test(result.returnValue.get(200)[1] == 500);
                    test(result.returnValue.get(400).length == 2);
                    test(result.returnValue.get(400)[0] == 600);
                    test(result.returnValue.get(400)[1] == 700);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<Long, long[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<Long, long[]> sdi2 = new java.util.HashMap<>();

            final long[] si1 = {999999110L, 999999111L, 999999110L};
            final long[] si2 = {999999120L, 999999130L};
            final long[] si3 = {999999110L, 999999120};

            sdi1.put(999999990L, si1);
            sdi1.put(999999991L, si2);
            sdi2.put(999999992L, si3);

            Callback cb = new Callback();
            p.opLongLongSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get(999999992L).length == 2);
                    test(result.p3.get(999999992L)[0] == 999999110L);
                    test(result.p3.get(999999992L)[1] == 999999120);
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get(999999990L).length == 3);
                    test(result.returnValue.get(999999990L)[0] == 999999110);
                    test(result.returnValue.get(999999990L)[1] == 999999111);
                    test(result.returnValue.get(999999990L)[2] == 999999110);
                    test(result.returnValue.get(999999991L).length == 2);
                    test(result.returnValue.get(999999991L)[0] == 999999120);
                    test(result.returnValue.get(999999991L)[1] == 999999130);
                    test(result.returnValue.get(999999992L).length == 2);
                    test(result.returnValue.get(999999992L)[0] == 999999110);
                    test(result.returnValue.get(999999992L)[1] == 999999120);
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opStringFloatSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get("aBc").length == 2);
                    test(result.p3.get("aBc")[0] == -3.14f);
                    test(result.p3.get("aBc")[1] == 3.14f);
                    test(result.returnValue.size() == 3);
                    test(result.returnValue.get("abc").length == 3);
                    test(result.returnValue.get("abc")[0] == -1.1f);
                    test(result.returnValue.get("abc")[1] == 123123.2f);
                    test(result.returnValue.get("abc")[2] == 100.0f);
                    test(result.returnValue.get("ABC").length == 2);
                    test(result.returnValue.get("ABC")[0] == 42.24f);
                    test(result.returnValue.get("ABC")[1] == -1.61f);
                    test(result.returnValue.get("aBc").length == 2);
                    test(result.returnValue.get("aBc")[0] == -3.14f);
                    test(result.returnValue.get("aBc")[1] == 3.14f);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<String, double[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<String, double[]> sdi2 = new java.util.HashMap<>();

            double[] si1 = new double[]{1.1E10, 1.2E10, 1.3E10};
            double[] si2 = new double[]{1.4E10, 1.5E10};
            double[] si3 = new double[]{1.6E10, 1.7E10};

            sdi1.put("Hello!!", si1);
            sdi1.put("Goodbye", si2);
            sdi2.put("", si3);

            Callback cb = new Callback();
            p.opStringDoubleSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get("").length == 2);
                    test(result.p3.get("")[0] == 1.6E10);
                    test(result.p3.get("")[1] == 1.7E10);
                    test(result.returnValue.size()== 3);
                    test(result.returnValue.get("Hello!!").length == 3);
                    test(result.returnValue.get("Hello!!")[0] == 1.1E10);
                    test(result.returnValue.get("Hello!!")[1] == 1.2E10);
                    test(result.returnValue.get("Hello!!")[2] == 1.3E10);
                    test(result.returnValue.get("Goodbye").length == 2);
                    test(result.returnValue.get("Goodbye")[0] == 1.4E10);
                    test(result.returnValue.get("Goodbye")[1] == 1.5E10);
                    test(result.returnValue.get("").length== 2);
                    test(result.returnValue.get("")[0] == 1.6E10);
                    test(result.returnValue.get("")[1] == 1.7E10);
                    cb.called();
                });
            cb.check();
        }

        {
            java.util.Map<String, String[]> sdi1 = new java.util.HashMap<>();
            java.util.Map<String, String[]> sdi2 = new java.util.HashMap<>();

            String[] si1 = new String[] { "abc", "de", "fghi" };
            String[] si2 = new String[] { "xyz", "or" };
            String[] si3 = new String[] { "and", "xor" };

            sdi1.put("abc", si1);
            sdi1.put("def", si2);
            sdi2.put("ghi", si3);

            Callback cb = new Callback();
            p.opStringStringSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get("ghi").length== 2);
                    test(result.p3.get("ghi")[0].equals("and"));
                    test(result.p3.get("ghi")[1].equals("xor"));
                    test(result.returnValue.size()== 3);
                    test(result.returnValue.get("abc").length == 3);
                    test(result.returnValue.get("abc")[0].equals("abc"));
                    test(result.returnValue.get("abc")[1].equals("de"));
                    test(result.returnValue.get("abc")[2].equals("fghi"));
                    test(result.returnValue.get("def").length == 2);
                    test(result.returnValue.get("def")[0].equals("xyz"));
                    test(result.returnValue.get("def")[1].equals("or"));
                    test(result.returnValue.get("ghi").length == 2);
                    test(result.returnValue.get("ghi")[0].equals("and"));
                    test(result.returnValue.get("ghi")[1].equals("xor"));
                    cb.called();
                });
            cb.check();
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

            Callback cb = new Callback();
            p.opMyEnumMyEnumSDAsync(sdi1, sdi2).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.p3.size() == 1);
                    test(result.p3.get(MyEnum.enum1).length == 2);
                    test(result.p3.get(MyEnum.enum1)[0] == MyEnum.enum3);
                    test(result.p3.get(MyEnum.enum1)[1] == MyEnum.enum3);
                    test(result.returnValue.size()== 3);
                    test(result.returnValue.get(MyEnum.enum3).length == 3);
                    test(result.returnValue.get(MyEnum.enum3)[0] == MyEnum.enum1);
                    test(result.returnValue.get(MyEnum.enum3)[1] == MyEnum.enum1);
                    test(result.returnValue.get(MyEnum.enum3)[2] == MyEnum.enum2);
                    test(result.returnValue.get(MyEnum.enum2).length == 2);
                    test(result.returnValue.get(MyEnum.enum2)[0] == MyEnum.enum1);
                    test(result.returnValue.get(MyEnum.enum2)[1] == MyEnum.enum2);
                    test(result.returnValue.get(MyEnum.enum1).length == 2);
                    test(result.returnValue.get(MyEnum.enum1)[0] == MyEnum.enum3);
                    test(result.returnValue.get(MyEnum.enum1)[1] == MyEnum.enum3);
                    cb.called();
                });
            cb.check();
        }

        {
            int[] lengths = { 0, 1, 2, 126, 127, 128, 129, 253, 254, 255, 256, 257, 1000 };

            for(int l : lengths)
            {
                int[] s = new int[l];
                for(int i = 0; i < s.length; ++i)
                {
                    s[i] = i;
                }
                Callback cb = new Callback();
                p.opIntSAsync(s).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.length == l);
                    for(int j = 0; j < result.length; ++j)
                    {
                        test(result[j] == -j);
                    }
                    cb.called();
                });
                cb.check();
            }
        }

        {
            java.util.Map<String, String> ctx = new java.util.HashMap<>();
            ctx.put("one", "ONE");
            ctx.put("two", "TWO");
            ctx.put("three", "THREE");
            {
                test(p.ice_getContext().isEmpty());
                java.util.Map<String, String> c = p.opContextAsync().join();
                test(!c.equals(ctx));
            }
            {
                test(p.ice_getContext().isEmpty());
                java.util.Map<String, String> c = p.opContextAsync(ctx).join();
                test(c.equals(ctx));
            }
            MyClassPrx p2 = MyClassPrx.checkedCast(p.ice_context(ctx));
            test(p2.ice_getContext().equals(ctx));
            {
                java.util.Map<String, String> c = p2.opContextAsync().join();
                test(c.equals(ctx));
            }
            {
                java.util.Map<String, String> c = p2.opContextAsync(ctx).join();
                test(c.equals(ctx));
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
                com.zeroc.Ice.InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                com.zeroc.Ice.Communicator ic = app.initialize(initData);

                java.util.Map<String, String> ctx = new java.util.HashMap<>();
                ctx.put("one", "ONE");
                ctx.put("two", "TWO");
                ctx.put("three", "THREE");

                MyClassPrx p3 = MyClassPrx.uncheckedCast(ic.stringToProxy("test:" + app.getTestEndpoint(0)));

                ic.getImplicitContext().setContext(ctx);
                test(ic.getImplicitContext().getContext().equals(ctx));
                {
                    java.util.Map<String, String> c = p3.opContextAsync().join();
                    test(c.equals(ctx));
                }

                ic.getImplicitContext().put("zero", "ZERO");

                ctx = ic.getImplicitContext().getContext();
                {
                    java.util.Map<String, String> c = p3.opContextAsync().join();
                    test(c.equals(ctx));
                }

                java.util.Map<String, String> prxContext = new java.util.HashMap<>();
                prxContext.put("one", "UN");
                prxContext.put("four", "QUATRE");

                java.util.Map<String, String> combined = new java.util.HashMap<>(ctx);
                combined.putAll(prxContext);
                test(combined.get("one").equals("UN"));

                p3 = p3.ice_context(prxContext);

                ic.getImplicitContext().setContext(null);
                {
                    java.util.Map<String, String> c = p3.opContextAsync().join();
                    test(c.equals(prxContext));
                }

                ic.getImplicitContext().setContext(ctx);
                {
                    java.util.Map<String, String> c = p3.opContextAsync().join();
                    test(c.equals(combined));
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
            Callback cb = new Callback();
            p.opDoubleMarshalingAsync(d, ds).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opIdempotentAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opNonmutatingAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            MyDerivedClassPrx derived = MyDerivedClassPrx.checkedCast(p);
            test(derived != null);
            Callback cb = new Callback();
            derived.opDerivedAsync().whenComplete((result, ex) ->
                {
                    test(ex == null);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opByte1Async((byte)0xFF).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == (byte)0xFF);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opShort1Async((short)0x7FFF).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == 0x7FFF);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opInt1Async(0x7FFFFFFF).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == 0x7FFFFFFF);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opLong1Async(0x7FFFFFFF).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == 0x7FFFFFFF);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opFloat1Async(1.0f).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == 1.0f);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opDouble1Async(1.0).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result == 1.0);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opString1Async("opString1").whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.equals("opString1"));
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opStringS1Async(null).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.length == 0);
                    cb.called();
                });
            cb.check();
        }

        {
            Callback cb = new Callback();
            p.opByteBoolD1Async(null).whenComplete((result, ex) ->
                {
                    test(ex == null);
                    test(result.size() == 0);
                    cb.called();
                });
            cb.check();
        }
    }
}
