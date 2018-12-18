// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations.lambda;

import test.Ice.operations.Test.AnotherStruct;
import test.Ice.operations.Test._MyClassDisp;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test._MyDerivedClassDisp;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;
import test.Ice.operations.Test.MyEnum;
import test.Ice.operations.Test.Structure;
import test.Ice.operations.Test.MyStruct;

import java.util.Map;

public class TwowaysLambdaAMI
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
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

    private static class pingI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class isAI
    {
        public void response(boolean r)
        {
            test(r);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class idI
    {
        public void response(String id)
        {
            test(id.equals(_MyDerivedClassDisp.ice_staticId()));
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class idsI
    {
        public void response(String[] ids)
        {
            test(ids.length == 3);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opVoidI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteI
    {
        public void response(byte r, byte b)
        {
            test(b == (byte)0xf0);
            test(r == (byte)0xff);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolI
    {
        public void response(boolean r, boolean b)
        {
            test(b);
            test(!r);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntLongI
    {
        public void response(long r, short s, int i, long l)
        {
            test(s == 10);
            test(i == 11);
            test(l == 12);
            test(r == 12);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opFloatDoubleI
    {
        public void response(double r, float f, double d)
        {
            test(f == 3.14f);
            test(d == 1.1E10);
            test(r == 1.1E10);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringI
    {
        public void response(String r, String s)
        {
            test(s.equals("world hello"));
            test(r.equals("hello world"));
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyEnumI
    {
        public void response(MyEnum r, MyEnum e)
        {
            test(e == MyEnum.enum2);
            test(r == MyEnum.enum3);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyClassI
    {
        opMyClassI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void response(MyClassPrx r, MyClassPrx c1, MyClassPrx c2)
        {
            test(c1.ice_getIdentity().equals(Ice.Util.stringToIdentity("test")));
            test(c2.ice_getIdentity().equals(Ice.Util.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().equals(Ice.Util.stringToIdentity("test")));
            // We can't do the callbacks below in connection serialization mode.
            if(_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
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
            }
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class opStructI
    {
        opStructI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void response(Structure rso, Structure so)
        {
            test(rso.p == null);
            test(rso.e == MyEnum.enum2);
            test(rso.s.s.equals("def"));
            test(so.e == MyEnum.enum3);
            test(so.s.s.equals("a new string"));
            // We can't do the callbacks below in connection serialization mode.
            if(_communicator.getProperties().getPropertyAsInt("Ice.ThreadPool.Client.Serialize") == 0)
            {
                so.p.opVoid();
            }
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class opByteSI
    {
        public void response(byte[] rso, byte[] bso)
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

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolSI
    {
        public void response(boolean[] rso, boolean[] bso)
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

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntLongSI
    {
        public void response(long[] rso, short[] sso, int[] iso, long[] lso)
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

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opFloatDoubleSI
    {
        public void response(double[] rso, float[] fso, double[] dso)
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
            test((float)rso[3] == 3.14f);
            test((float)rso[4] == 1.11f);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSI
    {
        public void response(String[] rso, String[] sso)
        {
            test(sso.length == 4);
            test(sso[0].equals("abc"));
            test(sso[1].equals("de"));
            test(sso[2].equals("fghi"));
            test(sso[3].equals("xyz"));
            test(rso.length == 3);
            test(rso[0].equals("fghi"));
            test(rso[1].equals("de"));
            test(rso[2].equals("abc"));
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteSSI
    {
        public void response(byte[][] rso, byte[][] bso)
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

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolSSI
    {
        public void response(boolean[][] rso, boolean[][] bso)
        {
            test(bso.length == 4);
            test(bso[0].length == 1);
            test(bso[0][0]);
            test(bso[1].length == 1);
            test(!bso[1][0]);
            test(bso[2].length == 2);
            test(bso[2][0]);
            test(bso[2][1]);
            test(bso[3].length == 3);
            test(!bso[3][0]);
            test(!bso[3][1]);
            test(bso[3][2]);
            test(rso.length == 3);
            test(rso[0].length == 2);
            test(rso[0][0]);
            test(rso[0][1]);
            test(rso[1].length == 1);
            test(!rso[1][0]);
            test(rso[2].length == 1);
            test(rso[2][0]);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntLongSSI
    {
        public void response(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
        {
            test(rso.length == 1);
            test(rso[0].length == 2);
            test(rso[0][0] == 496);
            test(rso[0][1] == 1729);
            test(sso.length == 3);
            test(sso[0].length == 3);
            test(sso[0][0] == 1);
            test(sso[0][1] == 2);
            test(sso[0][2] == 5);
            test(sso[1].length == 1);
            test(sso[1][0] == 13);
            test(sso[2].length == 0);
            test(iso.length == 2);
            test(iso[0].length == 1);
            test(iso[0][0] == 42);
            test(iso[1].length == 2);
            test(iso[1][0] == 24);
            test(iso[1][1] == 98);
            test(lso.length == 2);
            test(lso[0].length == 2);
            test(lso[0][0] == 496);
            test(lso[0][1] == 1729);
            test(lso[1].length == 2);
            test(lso[1][0] == 496);
            test(lso[1][1] == 1729);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opFloatDoubleSSI
    {
        public void response(double[][] rso, float[][] fso, double[][] dso)
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

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSSI
    {
        public void response(String[][] rso, String[][] sso)
        {
            test(sso.length == 5);
            test(sso[0].length == 1);
            test(sso[0][0].equals("abc"));
            test(sso[1].length == 2);
            test(sso[1][0].equals("de"));
            test(sso[1][1].equals("fghi"));
            test(sso[2].length == 0);
            test(sso[3].length == 0);
            test(sso[4].length == 1);
            test(sso[4][0].equals("xyz"));
            test(rso.length == 3);
            test(rso[0].length == 1);
            test(rso[0][0].equals("xyz"));
            test(rso[1].length == 0);
            test(rso[2].length == 0);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSSSI
    {
        public void response(String[][][] rsso, String[][][] ssso)
        {
            test(ssso.length == 5);
            test(ssso[0].length == 2);
            test(ssso[0][0].length == 2);
            test(ssso[0][1].length == 1);
            test(ssso[1].length == 1);
            test(ssso[1][0].length == 1);
            test(ssso[2].length == 2);
            test(ssso[2][0].length == 2);
            test(ssso[2][1].length == 1);
            test(ssso[3].length == 1);
            test(ssso[3][0].length == 1);
            test(ssso[4].length == 0);
            test(ssso[0][0][0].equals("abc"));
            test(ssso[0][0][1].equals("de"));
            test(ssso[0][1][0].equals("xyz"));
            test(ssso[1][0][0].equals("hello"));
            test(ssso[2][0][0].equals(""));
            test(ssso[2][0][1].equals(""));
            test(ssso[2][1][0].equals("abcd"));
            test(ssso[3][0][0].equals(""));

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
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteBoolDI
    {
        public void response(java.util.Map<Byte, Boolean> ro, java.util.Map<Byte, Boolean> _do)
        {
            java.util.Map<Byte, Boolean> di1 = new java.util.HashMap<Byte, Boolean>();
            di1.put((byte)10, Boolean.TRUE);
            di1.put((byte)100, Boolean.FALSE);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get((byte)10).booleanValue() == true);
            test(ro.get((byte)11).booleanValue() == false);
            test(ro.get((byte)100).booleanValue() == false);
            test(ro.get((byte)101).booleanValue() == true);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntDI
    {
        public void response(java.util.Map<Short, Integer> ro, java.util.Map<Short, Integer> _do)
        {
            java.util.Map<Short, Integer> di1 = new java.util.HashMap<Short, Integer>();
            di1.put((short)110, -1);
            di1.put((short)1100, 123123);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get((short)110).intValue() == -1);
            test(ro.get((short)111).intValue() == -100);
            test(ro.get((short)1100).intValue() == 123123);
            test(ro.get((short)1101).intValue() == 0);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opLongFloatDI
    {
        public void response(java.util.Map<Long, Float> ro, java.util.Map<Long, Float> _do)
        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<Long, Float>();
            di1.put(999999110L, Float.valueOf((float)-1.1f));
            di1.put(999999111L, Float.valueOf((float)123123.2f));
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get(999999110L).floatValue() == -1.1f);
            test(ro.get(999999120L).floatValue() == -100.4f);
            test(ro.get(999999111L).floatValue() == 123123.2f);
            test(ro.get(999999130L).floatValue() == 0.5f);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringStringDI
    {
        public void response(java.util.Map<String, String> ro, java.util.Map<String, String> _do)
        {
            java.util.Map<String, String> di1 = new java.util.HashMap<String, String>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get("foo").equals("abc -1.1"));
            test(ro.get("FOO").equals("abc -100.4"));
            test(ro.get("bar").equals("abc 123123.2"));
            test(ro.get("BAR").equals("abc 0.5"));
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringMyEnumDI
    {
        public void response(java.util.Map<String, MyEnum> ro, java.util.Map<String, MyEnum> _do)
        {
            java.util.Map<String, MyEnum> di1 = new java.util.HashMap<String, MyEnum>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get("abc") == MyEnum.enum1);
            test(ro.get("qwerty") == MyEnum.enum3);
            test(ro.get("") == MyEnum.enum2);
            test(ro.get("Hello!!") == MyEnum.enum2);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyEnumStringDI
    {
        public void response(java.util.Map<MyEnum, String> ro, java.util.Map<MyEnum, String> _do)
        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<MyEnum, String>();
            di1.put(MyEnum.enum1, "abc");
            test(_do.equals(di1));
            test(ro.size() == 3);
            test(ro.get(MyEnum.enum1).equals("abc"));
            test(ro.get(MyEnum.enum2).equals("Hello!!"));
            test(ro.get(MyEnum.enum3).equals("qwerty"));
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyStructMyEnumDI
    {
        public void response(java.util.Map<MyStruct, MyEnum> ro, java.util.Map<MyStruct, MyEnum> _do)
        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            java.util.Map<MyStruct, MyEnum> di1 = new java.util.HashMap<MyStruct, MyEnum>();
            di1.put(s11, MyEnum.enum1);
            di1.put(s12, MyEnum.enum2);
            test(_do.equals(di1));
            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);
            test(ro.size() == 4);
            test(ro.get(s11) == MyEnum.enum1);
            test(ro.get(s12) == MyEnum.enum2);
            test(ro.get(s22) == MyEnum.enum3);
            test(ro.get(s23) == MyEnum.enum2);
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opIntSI
    {
        opIntSI(int l)
        {
            _l = l;
        }

        public void response(int[] r)
        {
            test(r.length == _l);
            for(int j = 0; j < r.length; ++j)
            {
                test(r[j] == -j);
            }
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private int _l;
        private Callback callback = new Callback();
    }

    private static class opDerivedI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opDoubleMarshalingI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opIdempotentI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opNonmutatingI
    {
        public void response()
        {
            callback.called();
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    public static void
    twowaysLambdaAMI(test.TestHelper helper, MyClassPrx p)
    {
        Ice.Communicator communicator = helper.communicator();

        {
            pingI cb = new pingI();
            p.begin_ice_ping(() -> cb.response(), (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            isAI cb = new isAI();
            p.begin_ice_isA(_MyClassDisp.ice_staticId(), (boolean r) -> cb.response(r), (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            idI cb = new idI();
            p.begin_ice_id((String id) -> cb.response(id), (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            idsI cb = new idsI();
            p.begin_ice_ids((String[] ids) -> cb.response(ids), (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opVoidI cb = new opVoidI();
            p.begin_opVoid(() -> cb.response(), (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opByteI cb = new opByteI();
            p.begin_opByte((byte)0xff, (byte)0x0f,
                (byte p1, byte p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opBoolI cb = new opBoolI();
            p.begin_opBool(true, false,
                (boolean p1, boolean p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opShortIntLongI cb = new opShortIntLongI();
            p.begin_opShortIntLong((short)10, 11, 12L,
                (long p1, short p2, int p3, long p4) -> cb.response(p1, p2, p3, p4),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opFloatDoubleI cb = new opFloatDoubleI();
            p.begin_opFloatDouble(3.14f, 1.1E10,
                (double p1, float p2, double p3) -> cb.response(p1, p2, p3),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opStringI cb = new opStringI();
            p.begin_opString("hello", "world",
                (String p1, String p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opMyEnumI cb = new opMyEnumI();
            p.begin_opMyEnum(MyEnum.enum2,
                (MyEnum p1, MyEnum p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opMyClassI cb = new opMyClassI(communicator);
            p.begin_opMyClass(p,
                (MyClassPrx p1, MyClassPrx p2, MyClassPrx p3) -> cb.response(p1, p2, p3),
                (Ice.Exception ex) -> test(false));
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

            opStructI cb = new opStructI(communicator);
            p.begin_opStruct(si1, si2,
                (Structure p1, Structure p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
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

            opByteSI cb = new opByteSI();
            p.begin_opByteS(bsi1, bsi2,
                (byte[] p1, byte[] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            final boolean[] bsi1 = { true, true, false };
            final boolean[] bsi2 = { false };

            opBoolSI cb = new opBoolSI();
            p.begin_opBoolS(bsi1, bsi2,
                (boolean[] p1, boolean[] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            final short[] ssi = { 1, 2, 3 };
            final int[] isi = { 5, 6, 7, 8 };
            final long[] lsi = { 10, 30, 20 };

            opShortIntLongSI cb = new opShortIntLongSI();
            p.begin_opShortIntLongS(ssi, isi, lsi,
                (long[] p1, short[] p2, int[] p3, long[] p4) -> cb.response(p1, p2, p3, p4),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            final float[] fsi = { 3.14f, 1.11f };
            final double[] dsi = { 1.1E10, 1.2E10, 1.3E10 };

            opFloatDoubleSI cb = new opFloatDoubleSI();
            p.begin_opFloatDoubleS(fsi, dsi,
                (double[] p1, float[] p2, double[] p3) -> cb.response(p1, p2, p3),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            final String[] ssi1 = { "abc", "de", "fghi" };
            final String[] ssi2 = { "xyz" };

            opStringSI cb = new opStringSI();
            p.begin_opStringS(ssi1, ssi2,
                (String[] p1, String[] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
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

            opByteSSI cb = new opByteSSI();
            p.begin_opByteSS(bsi1, bsi2,
                (byte[][] p1, byte[][] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
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

            opBoolSSI cb = new opBoolSSI();
            p.begin_opBoolSS(bsi1, bsi2,
                    (boolean[][] rso, boolean[][] bso) -> cb.response(rso, bso),
                    (Ice.Exception ex) -> test(false));
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

            opShortIntLongSSI cb = new opShortIntLongSSI();
            p.begin_opShortIntLongSS(ssi, isi, lsi,
                    (long[][] rso, short[][] sso, int[][] iso, long[][] lso) -> cb.response(rso, sso, iso, lso),
                    (Ice.Exception ex) -> test(false));
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

            opFloatDoubleSSI cb = new opFloatDoubleSSI();
            p.begin_opFloatDoubleSS(fsi, dsi,
                (double[][] p1, float[][] p2, double[][] p3) -> cb.response(p1, p2, p3),
                (Ice.Exception ex) -> test(false));
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

            opStringSSI cb = new opStringSSI();
            p.begin_opStringSS(ssi1, ssi2,
                (String[][] p1, String[][] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
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

            opStringSSSI cb = new opStringSSSI();
            p.begin_opStringSSS(sssi1, sssi2,
                (String[][][] p1, String[][][] p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<Byte, Boolean> di1 = new java.util.HashMap<Byte, Boolean>();
            di1.put((byte)10, Boolean.TRUE);
            di1.put((byte)100, Boolean.FALSE);
            java.util.Map<Byte, Boolean> di2 = new java.util.HashMap<Byte, Boolean>();
            di2.put((byte)10, Boolean.TRUE);
            di2.put((byte)11, Boolean.FALSE);
            di2.put((byte)101, Boolean.TRUE);

            opByteBoolDI cb = new opByteBoolDI();
            p.begin_opByteBoolD(di1, di2,
                (Map<Byte, Boolean> p1, Map<Byte, Boolean> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<Short, Integer> di1 = new java.util.HashMap<Short, Integer>();
            di1.put((short)110, -1);
            di1.put((short)1100, 123123);
            java.util.Map<Short, Integer> di2 = new java.util.HashMap<Short, Integer>();
            di2.put((short)110, -1);
            di2.put((short)111, -100);
            di2.put((short)1101, 0);

            opShortIntDI cb = new opShortIntDI();
            p.begin_opShortIntD(di1, di2,
                (Map<Short, Integer> p1, Map<Short, Integer> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<Long, Float>();
            di1.put(999999110L, Float.valueOf((float)-1.1f));
            di1.put(999999111L, Float.valueOf((float)123123.2f));
            java.util.Map<Long, Float> di2 = new java.util.HashMap<Long, Float>();
            di2.put(999999110L, Float.valueOf((float)-1.1f));
            di2.put(999999120L, Float.valueOf((float)-100.4f));
            di2.put(999999130L, Float.valueOf((float)0.5f));

            opLongFloatDI cb = new opLongFloatDI();
            p.begin_opLongFloatD(di1, di2,
                (Map<Long, Float> p1, Map<Long, Float> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<String, String> di1 = new java.util.HashMap<String, String>();
            di1.put("foo", "abc -1.1");
            di1.put("bar", "abc 123123.2");
            java.util.Map<String, String> di2 = new java.util.HashMap<String, String>();
            di2.put("foo", "abc -1.1");
            di2.put("FOO", "abc -100.4");
            di2.put("BAR", "abc 0.5");

            opStringStringDI cb = new opStringStringDI();
            p.begin_opStringStringD(di1, di2,
                (Map<String, String> p1, Map<String, String> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<String, MyEnum> di1 = new java.util.HashMap<String, MyEnum>();
            di1.put("abc", MyEnum.enum1);
            di1.put("", MyEnum.enum2);
            java.util.Map<String, MyEnum> di2 = new java.util.HashMap<String, MyEnum>();
            di2.put("abc", MyEnum.enum1);
            di2.put("qwerty", MyEnum.enum3);
            di2.put("Hello!!", MyEnum.enum2);

            opStringMyEnumDI cb = new opStringMyEnumDI();
            p.begin_opStringMyEnumD(di1, di2,
                (Map<String, MyEnum> p1, Map<String, MyEnum> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<MyEnum, String>();
            di1.put(MyEnum.enum1, "abc");
            java.util.Map<MyEnum, String> di2 = new java.util.HashMap<MyEnum, String>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            opMyEnumStringDI cb = new opMyEnumStringDI();
            p.begin_opMyEnumStringD(di1, di2,
                (Map<MyEnum, String> p1, Map<MyEnum, String> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            java.util.Map<MyStruct, MyEnum> di1 = new java.util.HashMap<MyStruct, MyEnum>();
            di1.put(s11, MyEnum.enum1);
            di1.put(s12, MyEnum.enum2);
            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);
            java.util.Map<MyStruct, MyEnum> di2 = new java.util.HashMap<MyStruct, MyEnum>();
            di2.put(s11, MyEnum.enum1);
            di2.put(s22, MyEnum.enum3);
            di2.put(s23, MyEnum.enum2);

            opMyStructMyEnumDI cb = new opMyStructMyEnumDI();
            p.begin_opMyStructMyEnumD(di1, di2,
                (Map<MyStruct, MyEnum> p1, Map<MyStruct, MyEnum> p2) -> cb.response(p1, p2),
                (Ice.Exception ex) -> test(false));
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
                opIntSI cb = new opIntSI(l);
                p.begin_opIntS(s,
                    (int[] p1) -> cb.response(p1),
                    (Ice.Exception ex) -> test(false));
                cb.check();
            }
        }

        {
            double d = 1278312346.0 / 13.0;
            double[] ds = new double[5];
            for(int i = 0; i < 5; i++)
            {
                ds[i] = d;
            }
            opDoubleMarshalingI cb = new opDoubleMarshalingI();
            p.begin_opDoubleMarshaling(d, ds,
                () -> cb.response(),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opIdempotentI cb = new opIdempotentI();
            p.begin_opIdempotent(
                () -> cb.response(),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            opNonmutatingI cb = new opNonmutatingI();
            p.begin_opNonmutating(
                () -> cb.response(),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }

        {
            MyDerivedClassPrx derived = MyDerivedClassPrxHelper.checkedCast(p);
            test(derived != null);
            opDerivedI cb = new opDerivedI();
            derived.begin_opDerived(
                () -> cb.response(),
                (Ice.Exception ex) -> test(false));
            cb.check();
        }
    }
}
