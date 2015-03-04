// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;
import test.Ice.operations.Test.AMI_MyClass_opBool;
import test.Ice.operations.Test.AMI_MyClass_opBoolS;
import test.Ice.operations.Test.AMI_MyClass_opBoolSS;
import test.Ice.operations.Test.AMI_MyClass_opByte;
import test.Ice.operations.Test.AMI_MyClass_opByteBoolD;
import test.Ice.operations.Test.AMI_MyClass_opByteS;
import test.Ice.operations.Test.AMI_MyClass_opByteSS;
import test.Ice.operations.Test.AMI_MyClass_opContext;
import test.Ice.operations.Test.AMI_MyClass_opDoubleMarshaling;
import test.Ice.operations.Test.AMI_MyClass_opFloatDouble;
import test.Ice.operations.Test.AMI_MyClass_opFloatDoubleS;
import test.Ice.operations.Test.AMI_MyClass_opFloatDoubleSS;
import test.Ice.operations.Test.AMI_MyClass_opIdempotent;
import test.Ice.operations.Test.AMI_MyClass_opIntS;
import test.Ice.operations.Test.AMI_MyClass_opLongFloatD;
import test.Ice.operations.Test.AMI_MyClass_opMyClass;
import test.Ice.operations.Test.AMI_MyClass_opMyEnum;
import test.Ice.operations.Test.AMI_MyClass_opNonmutating;
import test.Ice.operations.Test.AMI_MyClass_opShortIntD;
import test.Ice.operations.Test.AMI_MyClass_opShortIntLong;
import test.Ice.operations.Test.AMI_MyClass_opShortIntLongS;
import test.Ice.operations.Test.AMI_MyClass_opShortIntLongSS;
import test.Ice.operations.Test.AMI_MyClass_opString;
import test.Ice.operations.Test.AMI_MyClass_opStringMyEnumD;
import test.Ice.operations.Test.AMI_MyClass_opMyEnumStringD;
import test.Ice.operations.Test.AMI_MyClass_opStringS;
import test.Ice.operations.Test.AMI_MyClass_opStringSS;
import test.Ice.operations.Test.AMI_MyClass_opStringSSS;
import test.Ice.operations.Test.AMI_MyClass_opStringStringD;
import test.Ice.operations.Test.AMI_MyClass_opStruct;
import test.Ice.operations.Test.AMI_MyClass_opVoid;
import test.Ice.operations.Test.AMI_MyDerivedClass_opDerived;
import test.Ice.operations.Test.AMI_MyClass_opMyStructMyEnumD;
import test.Ice.operations.Test.AnotherStruct;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;
import test.Ice.operations.Test.MyEnum;
import test.Ice.operations.Test.Structure;
import test.Ice.operations.Test.MyStruct;

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

    private static class Callback
    {
        Callback()
        {
            _called = false;
        }

        public synchronized void
        check()
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

        public synchronized void
        called()
        {
            assert(!_called);
            _called = true;
            notify();
        }

        private boolean _called;
    }

    private static class AMI_MyClass_opVoidI extends AMI_MyClass_opVoid
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opVoidExI extends AMI_MyClass_opVoid
    {
        public void
        ice_response()
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteI extends AMI_MyClass_opByte
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteExI extends AMI_MyClass_opByte
    {
        public void
        ice_response(byte r, byte b)
        {
            test(false);
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(ex instanceof Ice.NoEndpointException);
            callback.called();
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opBoolI extends AMI_MyClass_opBool
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opShortIntLongI extends AMI_MyClass_opShortIntLong
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opFloatDoubleI extends AMI_MyClass_opFloatDouble
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringI extends AMI_MyClass_opString
    {
        public void
        ice_response(String r, String s)
        {
            test(s.equals("world hello"));
            test(r.equals("hello world"));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opMyEnumI extends AMI_MyClass_opMyEnum
    {
        public void
        ice_response(MyEnum r, MyEnum e)
        {
            test(e == MyEnum.enum2);
            test(r == MyEnum.enum3);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opMyClassI extends AMI_MyClass_opMyClass
    {
        AMI_MyClass_opMyClassI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void
        ice_response(MyClassPrx r, MyClassPrx c1, MyClassPrx c2)
        {
            test(c1.ice_getIdentity().equals(_communicator.stringToIdentity("test")));
            test(c2.ice_getIdentity().equals(_communicator.stringToIdentity("noSuchIdentity")));
            test(r.ice_getIdentity().equals(_communicator.stringToIdentity("test")));
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class AMI_MyClass_opStructI extends AMI_MyClass_opStruct
    {
        AMI_MyClass_opStructI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        public void
        ice_response(Structure rso, Structure so)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class AMI_MyClass_opByteSI extends AMI_MyClass_opByteS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opBoolSI extends AMI_MyClass_opBoolS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opShortIntLongSI extends AMI_MyClass_opShortIntLongS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opFloatDoubleSI extends AMI_MyClass_opFloatDoubleS
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
            test((float)rso[3] == 3.14f);
            test((float)rso[4] == 1.11f);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringSI extends AMI_MyClass_opStringS
    {
        public void
        ice_response(String[] rso, String[] sso)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteSSI extends AMI_MyClass_opByteSS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opBoolSSI extends AMI_MyClass_opBoolSS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opShortIntLongSSI extends AMI_MyClass_opShortIntLongSS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opFloatDoubleSSI extends AMI_MyClass_opFloatDoubleSS
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringSSI extends AMI_MyClass_opStringSS
    {
        public void
        ice_response(String[][] rso, String[][] sso)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringSSSI extends AMI_MyClass_opStringSSS
    {
        public void
        ice_response(String[][][] rsso, String[][][] ssso)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opByteBoolDI extends AMI_MyClass_opByteBoolD
    {
        public void
        ice_response(java.util.Map<Byte, Boolean> ro, java.util.Map<Byte, Boolean> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opShortIntDI extends AMI_MyClass_opShortIntD
    {
        public void
        ice_response(java.util.Map<Short, Integer> ro, java.util.Map<Short, Integer> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opLongFloatDI extends AMI_MyClass_opLongFloatD
    {
        public void
        ice_response(java.util.Map<Long, Float> ro, java.util.Map<Long, Float> _do)
        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<Long, Float>();
            di1.put(999999110L, new Float(-1.1f));
            di1.put(999999111L, new Float(123123.2f));
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get(999999110L).floatValue() == -1.1f);
            test(ro.get(999999120L).floatValue() == -100.4f);
            test(ro.get(999999111L).floatValue() == 123123.2f);
            test(ro.get(999999130L).floatValue() == 0.5f);
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringStringDI extends AMI_MyClass_opStringStringD
    {
        public void
        ice_response(java.util.Map<String, String> ro, java.util.Map<String, String> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opStringMyEnumDI extends AMI_MyClass_opStringMyEnumD
    {
        public void
        ice_response(java.util.Map<String, MyEnum> ro, java.util.Map<String, MyEnum> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opMyEnumStringDI extends AMI_MyClass_opMyEnumStringD
    {
        public void
        ice_response(java.util.Map<MyEnum, String> ro, java.util.Map<MyEnum, String> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opMyStructMyEnumDI extends AMI_MyClass_opMyStructMyEnumD
    {
        public void
        ice_response(java.util.Map<MyStruct, MyEnum> ro, java.util.Map<MyStruct, MyEnum> _do)
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

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opIntSI extends AMI_MyClass_opIntS
    {
        AMI_MyClass_opIntSI(int l)
        {
            _l = l;
        }

        public void
        ice_response(int[] r)
        {
            test(r.length == _l);
            for(int j = 0; j < r.length; ++j)
            {
                test(r[j] == -j);
            }
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private int _l;
        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opContextEqualI extends AMI_MyClass_opContext
    {
        AMI_MyClass_opContextEqualI(java.util.Map<String, String> d)
        {
            _d = d;
        }

        public void
        ice_response(java.util.Map r)
        {
            test(r.equals(_d));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private java.util.Map<String, String> _d;
        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opContextNotEqualI extends AMI_MyClass_opContext
    {
        AMI_MyClass_opContextNotEqualI(java.util.Map<String, String> d)
        {
            _d = d;
        }

        public void
        ice_response(java.util.Map r)
        {
            test(!r.equals(_d));
            callback.called();
        }

        public void
        ice_exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void
        check()
        {
            callback.check();
        }

        private java.util.Map<String, String> _d;
        private Callback callback = new Callback();
    }

    private static class AMI_MyDerivedClass_opDerivedI extends AMI_MyDerivedClass_opDerived
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opDoubleMarshalingI extends AMI_MyClass_opDoubleMarshaling
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opIdempotentI extends AMI_MyClass_opIdempotent
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class AMI_MyClass_opNonmutatingI extends AMI_MyClass_opNonmutating
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

        public void
        check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    static void
    twowaysAMI(test.Util.Application app, MyClassPrx p)
    {
        Ice.Communicator communicator = app.communicator();

        {
            // Check that a call to a void operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            MyClassPrx indirect = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opVoidExI cb = new AMI_MyClass_opVoidExI();
            try
            {
                test(!indirect.opVoid_async(cb));
            }
            catch(java.lang.Exception ex)
            {
                test(false);
            }
            cb.check();
        }

        {
            // Check that a call to a twoway operation raises NoEndpointException
            // in the ice_exception() callback instead of at the point of call.
            MyClassPrx indirect = MyClassPrxHelper.uncheckedCast(p.ice_adapterId("dummy"));
            AMI_MyClass_opByteExI cb = new AMI_MyClass_opByteExI();
            try
            {
                test(!(indirect.opByte_async(cb, (byte)0, (byte)0)));
            }
            catch(java.lang.Exception ex)
            {
                test(false);
            }
            cb.check();
        }

        {
            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            p.opVoid_async(cb);
            cb.check();
            // Let's check if we can reuse the same callback object for another call.
            p.opVoid_async(cb);
            cb.check();
        }

        {
            // Check that CommunicatorDestroyedException is raised directly.
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = communicator.getProperties()._clone();
            Ice.Communicator ic = app.initialize(initData);
            Ice.ObjectPrx obj = ic.stringToProxy(p.toString());
            MyClassPrx p2 = MyClassPrxHelper.checkedCast(obj);

            ic.destroy();

            AMI_MyClass_opVoidI cb = new AMI_MyClass_opVoidI();
            try
            {
                test(!p2.opVoid_async(cb));
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
                // Expected.
            }
        }


        {
            AMI_MyClass_opByteI cb = new AMI_MyClass_opByteI();
            p.opByte_async(cb, (byte)0xff, (byte)0x0f);
            cb.check();
        }

        {
            AMI_MyClass_opBoolI cb = new AMI_MyClass_opBoolI();
            p.opBool_async(cb, true, false);
            cb.check();
        }

        {
            AMI_MyClass_opShortIntLongI cb = new AMI_MyClass_opShortIntLongI();
            p.opShortIntLong_async(cb, (short)10, 11, 12L);
            cb.check();
        }

        {
            AMI_MyClass_opFloatDoubleI cb = new AMI_MyClass_opFloatDoubleI();
            p.opFloatDouble_async(cb, 3.14f, 1.1E10);
            cb.check();
            // Let's check if we can reuse the same callback object for another call.
            p.opFloatDouble_async(cb, 3.14f, 1.1E10);
            cb.check();
        }

        {
            AMI_MyClass_opStringI cb = new AMI_MyClass_opStringI();
            p.opString_async(cb, "hello", "world");
            cb.check();
        }

        {
            AMI_MyClass_opMyEnumI cb = new AMI_MyClass_opMyEnumI();
            p.opMyEnum_async(cb, MyEnum.enum2);
            cb.check();
        }

        {
            AMI_MyClass_opMyClassI cb = new AMI_MyClass_opMyClassI(communicator);
            p.opMyClass_async(cb, p);
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

            AMI_MyClass_opStructI cb = new AMI_MyClass_opStructI(communicator);
            p.opStruct_async(cb, si1, si2);
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

            AMI_MyClass_opByteSI cb = new AMI_MyClass_opByteSI();
            p.opByteS_async(cb, bsi1, bsi2);
            cb.check();
        }

        {
            final boolean[] bsi1 = { true, true, false };
            final boolean[] bsi2 = { false };

            AMI_MyClass_opBoolSI cb = new AMI_MyClass_opBoolSI();
            p.opBoolS_async(cb, bsi1, bsi2);
            cb.check();
        }

        {
            final short[] ssi = { 1, 2, 3 };
            final int[] isi = { 5, 6, 7, 8 };
            final long[] lsi = { 10, 30, 20 };

            AMI_MyClass_opShortIntLongSI cb = new AMI_MyClass_opShortIntLongSI();
            p.opShortIntLongS_async(cb, ssi, isi, lsi);
            cb.check();
        }

        {
            final float[] fsi = { 3.14f, 1.11f };
            final double[] dsi = { 1.1E10, 1.2E10, 1.3E10 };

            AMI_MyClass_opFloatDoubleSI cb = new AMI_MyClass_opFloatDoubleSI();
            p.opFloatDoubleS_async(cb, fsi, dsi);
            cb.check();
        }

        {
            final String[] ssi1 = { "abc", "de", "fghi" };
            final String[] ssi2 = { "xyz" };

            AMI_MyClass_opStringSI cb = new AMI_MyClass_opStringSI();
            p.opStringS_async(cb, ssi1, ssi2);
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

            AMI_MyClass_opByteSSI cb = new AMI_MyClass_opByteSSI();
            p.opByteSS_async(cb, bsi1, bsi2);
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

            AMI_MyClass_opFloatDoubleSSI cb = new AMI_MyClass_opFloatDoubleSSI();
            p.opFloatDoubleSS_async(cb, fsi, dsi);
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

            AMI_MyClass_opStringSSI cb = new AMI_MyClass_opStringSSI();
            p.opStringSS_async(cb, ssi1, ssi2);
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

            AMI_MyClass_opStringSSSI cb = new AMI_MyClass_opStringSSSI();
            p.opStringSSS_async(cb, sssi1, sssi2);
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

            AMI_MyClass_opByteBoolDI cb = new AMI_MyClass_opByteBoolDI();
            p.opByteBoolD_async(cb, di1, di2);
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

            AMI_MyClass_opShortIntDI cb = new AMI_MyClass_opShortIntDI();
            p.opShortIntD_async(cb, di1, di2);
            cb.check();
        }

        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<Long, Float>();
            di1.put(999999110L, new Float(-1.1f));
            di1.put(999999111L, new Float(123123.2f));
            java.util.Map<Long, Float> di2 = new java.util.HashMap<Long, Float>();
            di2.put(999999110L, new Float(-1.1f));
            di2.put(999999120L, new Float(-100.4f));
            di2.put(999999130L, new Float(0.5f));

            AMI_MyClass_opLongFloatDI cb = new AMI_MyClass_opLongFloatDI();
            p.opLongFloatD_async(cb, di1, di2);
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

            AMI_MyClass_opStringStringDI cb = new AMI_MyClass_opStringStringDI();
            p.opStringStringD_async(cb, di1, di2);
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

            AMI_MyClass_opStringMyEnumDI cb = new AMI_MyClass_opStringMyEnumDI();
            p.opStringMyEnumD_async(cb, di1, di2);
            cb.check();
        }

        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<MyEnum, String>();
            di1.put(MyEnum.enum1, "abc");
            java.util.Map<MyEnum, String> di2 = new java.util.HashMap<MyEnum, String>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            AMI_MyClass_opMyEnumStringDI cb = new AMI_MyClass_opMyEnumStringDI();
            p.opMyEnumStringD_async(cb, di1, di2);
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

            AMI_MyClass_opMyStructMyEnumDI cb = new AMI_MyClass_opMyStructMyEnumDI();
            p.opMyStructMyEnumD_async(cb, di1, di2);
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
                AMI_MyClass_opIntSI cb = new AMI_MyClass_opIntSI(l);
                p.opIntS_async(cb, s);
                cb.check();
            }
        }

        {
            java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
            ctx.put("one", "ONE");
            ctx.put("two", "TWO");
            ctx.put("three", "THREE");
            {
                test(p.ice_getContext().isEmpty());
                AMI_MyClass_opContextNotEqualI cb = new AMI_MyClass_opContextNotEqualI(ctx);
                p.opContext_async(cb);
                cb.check();
            }
            {
                test(p.ice_getContext().isEmpty());
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p.opContext_async(cb, ctx);
                cb.check();
            }
            MyClassPrx p2 = MyClassPrxHelper.checkedCast(p.ice_context(ctx));
            test(p2.ice_getContext().equals(ctx));
            {
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p2.opContext_async(cb);
                cb.check();
            }
            {
                AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                p2.opContext_async(cb, ctx);
                cb.check();
            }
        }

        {
            //
            // Test implicit context propagation
            //

            String[] impls = {"Shared", "PerThread"};
            for(int i = 0; i < 2; i++)
            {
                Ice.InitializationData initData = new Ice.InitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                Ice.Communicator ic = app.initialize(initData);

                java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
                ctx.put("one", "ONE");
                ctx.put("two", "TWO");
                ctx.put("three", "THREE");

                MyClassPrx p3 = MyClassPrxHelper.uncheckedCast(ic.stringToProxy("test:default -p 12010"));

                ic.getImplicitContext().setContext(ctx);
                test(ic.getImplicitContext().getContext().equals(ctx));
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                    p3.opContext_async(cb);
                    cb.check();
                }

                ic.getImplicitContext().put("zero", "ZERO");

                ctx = ic.getImplicitContext().getContext();
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(ctx);
                    p3.opContext_async(cb);
                    cb.check();
                }

                java.util.Map<String, String> prxContext = new java.util.HashMap<String, String>();
                prxContext.put("one", "UN");
                prxContext.put("four", "QUATRE");

                java.util.Map<String, String> combined = new java.util.HashMap<String, String>(ctx);
                combined.putAll(prxContext);
                test(combined.get("one").equals("UN"));

                p3 = MyClassPrxHelper.uncheckedCast(p3.ice_context(prxContext));

                ic.getImplicitContext().setContext(null);
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(prxContext);
                    p3.opContext_async(cb);
                    cb.check();
                }

                ic.getImplicitContext().setContext(ctx);
                {
                    AMI_MyClass_opContextEqualI cb = new AMI_MyClass_opContextEqualI(combined);
                    p3.opContext_async(cb);
                    cb.check();
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
            AMI_MyClass_opDoubleMarshalingI cb = new AMI_MyClass_opDoubleMarshalingI();
            p.opDoubleMarshaling_async(cb, d, ds);
            cb.check();
        }

        {
            AMI_MyClass_opIdempotentI cb = new AMI_MyClass_opIdempotentI();
            p.opIdempotent_async(cb);
            cb.check();
        }

        {
            AMI_MyClass_opNonmutatingI cb = new AMI_MyClass_opNonmutatingI();
            p.opNonmutating_async(cb);
            cb.check();
        }

        {
            MyDerivedClassPrx derived = MyDerivedClassPrxHelper.checkedCast(p);
            test(derived != null);
            AMI_MyDerivedClass_opDerivedI cb = new AMI_MyDerivedClass_opDerivedI();
            derived.opDerived_async(cb);
            cb.check();
        }
    }
}
