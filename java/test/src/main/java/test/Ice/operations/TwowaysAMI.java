// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.operations;

import Ice.LocalException;
import test.Ice.operations.AMD.Test.*;
import test.Ice.operations.Test.*;
import test.Ice.operations.Test.AnotherStruct;
import test.Ice.operations.Test.ByteBoolDSHolder;
import test.Ice.operations.Test.Callback_MyClass_opBool;
import test.Ice.operations.Test.Callback_MyClass_opBoolBoolSD;
import test.Ice.operations.Test.Callback_MyClass_opBoolS;
import test.Ice.operations.Test.Callback_MyClass_opBoolSS;
import test.Ice.operations.Test.Callback_MyClass_opByte;
import test.Ice.operations.Test.Callback_MyClass_opByteBoolD;
import test.Ice.operations.Test.Callback_MyClass_opByteBoolDS;
import test.Ice.operations.Test.Callback_MyClass_opByteByteSD;
import test.Ice.operations.Test.Callback_MyClass_opByteS;
import test.Ice.operations.Test.Callback_MyClass_opByteSS;
import test.Ice.operations.Test.Callback_MyClass_opDoubleMarshaling;
import test.Ice.operations.Test.Callback_MyClass_opFloatDouble;
import test.Ice.operations.Test.Callback_MyClass_opFloatDoubleS;
import test.Ice.operations.Test.Callback_MyClass_opFloatDoubleSS;
import test.Ice.operations.Test.Callback_MyClass_opIdempotent;
import test.Ice.operations.Test.Callback_MyClass_opIntIntSD;
import test.Ice.operations.Test.Callback_MyClass_opIntS;
import test.Ice.operations.Test.Callback_MyClass_opLongFloatD;
import test.Ice.operations.Test.Callback_MyClass_opLongFloatDS;
import test.Ice.operations.Test.Callback_MyClass_opLongLongSD;
import test.Ice.operations.Test.Callback_MyClass_opMyClass;
import test.Ice.operations.Test.Callback_MyClass_opMyEnum;
import test.Ice.operations.Test.Callback_MyClass_opMyEnumMyEnumSD;
import test.Ice.operations.Test.Callback_MyClass_opMyEnumStringD;
import test.Ice.operations.Test.Callback_MyClass_opMyEnumStringDS;
import test.Ice.operations.Test.Callback_MyClass_opMyStructMyEnumD;
import test.Ice.operations.Test.Callback_MyClass_opMyStructMyEnumDS;
import test.Ice.operations.Test.Callback_MyClass_opNonmutating;
import test.Ice.operations.Test.Callback_MyClass_opShortIntD;
import test.Ice.operations.Test.Callback_MyClass_opShortIntDS;
import test.Ice.operations.Test.Callback_MyClass_opShortIntLong;
import test.Ice.operations.Test.Callback_MyClass_opShortIntLongS;
import test.Ice.operations.Test.Callback_MyClass_opShortIntLongSS;
import test.Ice.operations.Test.Callback_MyClass_opShortShortSD;
import test.Ice.operations.Test.Callback_MyClass_opString;
import test.Ice.operations.Test.Callback_MyClass_opStringDoubleSD;
import test.Ice.operations.Test.Callback_MyClass_opStringFloatSD;
import test.Ice.operations.Test.Callback_MyClass_opStringMyEnumD;
import test.Ice.operations.Test.Callback_MyClass_opStringMyEnumDS;
import test.Ice.operations.Test.Callback_MyClass_opStringS;
import test.Ice.operations.Test.Callback_MyClass_opStringSS;
import test.Ice.operations.Test.Callback_MyClass_opStringSSS;
import test.Ice.operations.Test.Callback_MyClass_opStringStringD;
import test.Ice.operations.Test.Callback_MyClass_opStringStringDS;
import test.Ice.operations.Test.Callback_MyClass_opStringStringSD;
import test.Ice.operations.Test.Callback_MyClass_opStruct;
import test.Ice.operations.Test.Callback_MyClass_opVoid;
import test.Ice.operations.Test.Callback_MyDerivedClass_opDerived;
import test.Ice.operations.Test.MyClass;
import test.Ice.operations.Test.MyClassPrx;
import test.Ice.operations.Test.MyClassPrxHelper;
import test.Ice.operations.Test.MyDerivedClass;
import test.Ice.operations.Test.MyDerivedClassPrx;
import test.Ice.operations.Test.MyDerivedClassPrxHelper;
import test.Ice.operations.Test.MyEnum;
import test.Ice.operations.Test.MyStruct;
import test.Ice.operations.Test.Structure;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

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
    
    private static class GenericCallback<T> extends Callback
    {
        public GenericCallback(T value)
        {
            _value = value;
        }
        
        public void response(T value)
        {
            _value = value;
            _succeeded = true;
            called();
        }
        
        public void exception(Ice.LocalException ex)
        {
            _succeeded = false;
            called();
        }
        
        public boolean succeeded()
        {
            check();
            return _succeeded;
        }
        
        public T value()
        {
            return _value;
        }
        
        private T _value;
        private boolean _succeeded = false;
    }

    private static class pingI extends Ice.Callback_Object_ice_ping
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class isAI extends Ice.Callback_Object_ice_isA
    {
        @Override
        public void response(boolean r)
        {
            test(r);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class idI extends Ice.Callback_Object_ice_id
    {
        @Override
        public void response(String id)
        {
            test(id.equals(MyDerivedClass.ice_staticId()));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class idsI extends Ice.Callback_Object_ice_ids
    {
        @Override
        public void response(String[] ids)
        {
            test(ids.length == 3);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opVoidI extends Callback_MyClass_opVoid
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteI extends Callback_MyClass_opByte
    {
        @Override
        public void response(byte r, byte b)
        {
            test(b == (byte)0xf0);
            test(r == (byte)0xff);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolI extends Callback_MyClass_opBool
    {
        @Override
        public void response(boolean r, boolean b)
        {
            test(b);
            test(!r);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntLongI extends Callback_MyClass_opShortIntLong
    {
        @Override
        public void response(long r, short s, int i, long l)
        {
            test(s == 10);
            test(i == 11);
            test(l == 12);
            test(r == 12);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opFloatDoubleI extends Callback_MyClass_opFloatDouble
    {
        @Override
        public void response(double r, float f, double d)
        {
            test(f == 3.14f);
            test(d == 1.1E10);
            test(r == 1.1E10);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringI extends Callback_MyClass_opString
    {
        @Override
        public void response(String r, String s)
        {
            test(s.equals("world hello"));
            test(r.equals("hello world"));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyEnumI extends Callback_MyClass_opMyEnum
    {
        @Override
        public void response(MyEnum r, MyEnum e)
        {
            test(e == MyEnum.enum2);
            test(r == MyEnum.enum3);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyClassI extends Callback_MyClass_opMyClass
    {
        opMyClassI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void response(MyClassPrx r, MyClassPrx c1, MyClassPrx c2)
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class opStructI extends Callback_MyClass_opStruct
    {
        opStructI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
        private Ice.Communicator _communicator;
    }

    private static class opByteSI extends Callback_MyClass_opByteS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolSI extends Callback_MyClass_opBoolS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntLongSI extends Callback_MyClass_opShortIntLongS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opFloatDoubleSI extends Callback_MyClass_opFloatDoubleS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSI extends Callback_MyClass_opStringS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteSSI extends Callback_MyClass_opByteSS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opBoolSSI extends Callback_MyClass_opBoolSS
    {
        @Override
        public void
        response(boolean[][] rso, boolean[][] bso)
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

        @Override
        public void
        exception(Ice.LocalException ex)
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

    private static class opShortIntLongSSI extends Callback_MyClass_opShortIntLongSS
    {
        @Override
        public void
        response(long[][] rso, short[][] sso, int[][] iso, long[][] lso)
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

        @Override
        public void
        exception(Ice.LocalException ex)
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

    private static class opFloatDoubleSSI extends Callback_MyClass_opFloatDoubleSS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSSI extends Callback_MyClass_opStringSS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringSSSI extends Callback_MyClass_opStringSSS
    {
        @Override
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteBoolDI extends Callback_MyClass_opByteBoolD
    {
        @Override
        public void response(java.util.Map<Byte, Boolean> ro, java.util.Map<Byte, Boolean> _do)
        {
            java.util.Map<Byte, Boolean> di1 = new java.util.HashMap<>();
            di1.put((byte)10, Boolean.TRUE);
            di1.put((byte)100, Boolean.FALSE);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get((byte) 10));
            test(!ro.get((byte) 11));
            test(!ro.get((byte) 100));
            test(ro.get((byte) 101));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntDI extends Callback_MyClass_opShortIntD
    {
        @Override
        public void response(java.util.Map<Short, Integer> ro, java.util.Map<Short, Integer> _do)
        {
            java.util.Map<Short, Integer> di1 = new java.util.HashMap<>();
            di1.put((short)110, -1);
            di1.put((short)1100, 123123);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get((short) 110) == -1);
            test(ro.get((short) 111) == -100);
            test(ro.get((short) 1100) == 123123);
            test(ro.get((short) 1101) == 0);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opLongFloatDI extends Callback_MyClass_opLongFloatD
    {
        @Override
        public void response(java.util.Map<Long, Float> ro, java.util.Map<Long, Float> _do)
        {
            java.util.Map<Long, Float> di1 = new java.util.HashMap<>();
            di1.put(999999110L, -1.1f);
            di1.put(999999111L, 123123.2f);
            test(_do.equals(di1));
            test(ro.size() == 4);
            test(ro.get(999999110L) == -1.1f);
            test(ro.get(999999120L) == -100.4f);
            test(ro.get(999999111L) == 123123.2f);
            test(ro.get(999999130L) == 0.5f);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringStringDI extends Callback_MyClass_opStringStringD
    {
        @Override
        public void response(java.util.Map<String, String> ro, java.util.Map<String, String> _do)
        {
            java.util.Map<String, String> di1 = new java.util.HashMap<>();
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringMyEnumDI extends Callback_MyClass_opStringMyEnumD
    {
        @Override
        public void response(java.util.Map<String, MyEnum> ro, java.util.Map<String, MyEnum> _do)
        {
            java.util.Map<String, MyEnum> di1 = new java.util.HashMap<>();
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyEnumStringDI extends Callback_MyClass_opMyEnumStringD
    {
        @Override
        public void response(java.util.Map<MyEnum, String> ro, java.util.Map<MyEnum, String> _do)
        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            test(_do.equals(di1));
            test(ro.size() == 3);
            test(ro.get(MyEnum.enum1).equals("abc"));
            test(ro.get(MyEnum.enum2).equals("Hello!!"));
            test(ro.get(MyEnum.enum3).equals("qwerty"));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyStructMyEnumDI extends Callback_MyClass_opMyStructMyEnumD
    {
        @Override
        public void response(java.util.Map<MyStruct, MyEnum> ro, java.util.Map<MyStruct, MyEnum> _do)
        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            java.util.Map<MyStruct, MyEnum> di1 = new java.util.HashMap<>();
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

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteBoolDS extends Callback_MyClass_opByteBoolDS
    {

        @Override
        public void response(List<Map<Byte, Boolean>> ro, List<Map<Byte, Boolean>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get((byte) 10));
            test(!ro.get(0).get((byte) 11));
            test(ro.get(0).get((byte) 101));
            test(ro.get(1).size() == 2);
            test(ro.get(1).get((byte) 10));
            test(!ro.get(1).get((byte) 100));

            test(_do.size() == 3);
            test(_do.get(0).size() == 2);
            test(!_do.get(0).get((byte) 100));
            test(!_do.get(0).get((byte) 101));
            test(_do.get(1).size() == 2);
            test(_do.get(1).get((byte) 10));
            test(!_do.get(1).get((byte) 100));
            test(_do.get(2).size() == 3);
            test(_do.get(2).get((byte) 10));
            test(!_do.get(2).get((byte) 11));
            test(_do.get(2).get((byte) 101));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opShortIntDS extends Callback_MyClass_opShortIntDS
    {

        @Override
        public void response(List<Map<Short, Integer>> ro, List<Map<Short, Integer>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get((short) 110) == -1);
            test(ro.get(0).get((short) 111) == -100);
            test(ro.get(0).get((short) 1101) == 0);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get((short) 110) == -1);
            test(ro.get(1).get((short) 1100) == 123123);

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get((short) 100) == -1001);
            test(_do.get(1).size() == 2);
            test(_do.get(1).get((short) 110) == -1);
            test(_do.get(1).get((short) 1100) == 123123);
            test(_do.get(2).size() == 3);
            test(_do.get(2).get((short) 110) == -1);
            test(_do.get(2).get((short) 111) == -100);
            test(_do.get(2).get((short) 1101) == 0);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opLongFloatDS extends Callback_MyClass_opLongFloatDS
    {

        @Override
        public void response(List<Map<Long, Float>> ro, List<Map<Long, Float>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get(999999110L) == -1.1f);
            test(ro.get(0).get(999999120L) == -100.4f);
            test(ro.get(0).get(999999130L) == 0.5f);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get(999999110L) == -1.1f);
            test(ro.get(1).get(999999111L) == 123123.2f);

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get(999999140L) == 3.14f);
            test(_do.get(1).size() == 2);
            test(_do.get(1).get(999999110L) == -1.1f);
            test(_do.get(1).get(999999111L) == 123123.2f);
            test(_do.get(2).size() == 3);
            test(_do.get(2).get(999999110L) == -1.1f);
            test(_do.get(2).get(999999120L) == -100.4f);
            test(_do.get(2).get(999999130L) == 0.5f);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringStringDS extends Callback_MyClass_opStringStringDS
    {

        @Override
        public void response(List<Map<String, String>> ro, List<Map<String, String>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get("foo").equals("abc -1.1"));
            test(ro.get(0).get("FOO").equals("abc -100.4"));
            test(ro.get(0).get("BAR").equals("abc 0.5"));
            test(ro.get(1).size() == 2);
            test(ro.get(1).get("foo").equals("abc -1.1"));
            test(ro.get(1).get("bar").equals("abc 123123.2"));

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get("f00").equals("ABC -3.14"));
            test(_do.get(1).size() == 2);
            test(_do.get(1).get("foo").equals("abc -1.1"));
            test(_do.get(1).get("bar").equals("abc 123123.2"));
            test(_do.get(2).size() == 3);
            test(_do.get(2).get("foo").equals("abc -1.1"));
            test(_do.get(2).get("FOO").equals("abc -100.4"));
            test(_do.get(2).get("BAR").equals("abc 0.5"));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opStringMyEnumDS extends Callback_MyClass_opStringMyEnumDS
    {

        @Override
        public void response(List<Map<String, MyEnum>> ro, List<Map<String, MyEnum>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get("abc") == MyEnum.enum1);
            test(ro.get(0).get("qwerty") == MyEnum.enum3);
            test(ro.get(0).get("Hello!!") == MyEnum.enum2);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get("abc") == MyEnum.enum1);
            test(ro.get(1).get("") == MyEnum.enum2);

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get("Goodbye") == MyEnum.enum1);
            test(_do.get(1).size() == 2);
            test(_do.get(1).get("abc") == MyEnum.enum1);
            test(_do.get(1).get("") == MyEnum.enum2);
            test(_do.get(2).size() == 3);
            test(_do.get(2).get("abc") == MyEnum.enum1);
            test(_do.get(2).get("qwerty") == MyEnum.enum3);
            test(_do.get(2).get("Hello!!") == MyEnum.enum2);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyEnumStringDS extends Callback_MyClass_opMyEnumStringDS
    {

        @Override
        public void response(List<Map<MyEnum, String>> ro, List<Map<MyEnum, String>> _do)
        {
            test(ro.size() == 2);
            test(ro.get(0).size() == 2);
            test(ro.get(0).get(MyEnum.enum2).equals("Hello!!"));
            test(ro.get(0).get(MyEnum.enum3).equals("qwerty"));
            test(ro.get(1).size() == 1);
            test(ro.get(1).get(MyEnum.enum1).equals("abc"));

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get(MyEnum.enum1).equals("Goodbye"));
            test(_do.get(1).size() == 1);
            test(_do.get(1).get(MyEnum.enum1).equals("abc"));
            test(_do.get(2).size() == 2);
            test(_do.get(2).get(MyEnum.enum2).equals("Hello!!"));
            test(_do.get(2).get(MyEnum.enum3).equals("qwerty"));
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opMyStructMyEnumDS extends Callback_MyClass_opMyStructMyEnumDS
    {

        @Override
        public void response(List<Map<MyStruct, MyEnum>> ro, List<Map<MyStruct, MyEnum>> _do)
        {
            MyStruct s11 = new MyStruct(1, 1);
            MyStruct s12 = new MyStruct(1, 2);
            MyStruct s22 = new MyStruct(2, 2);
            MyStruct s23 = new MyStruct(2, 3);

            test(ro.size() == 2);
            test(ro.get(0).size() == 3);
            test(ro.get(0).get(s11) == MyEnum.enum1);
            test(ro.get(0).get(s22) == MyEnum.enum3);
            test(ro.get(0).get(s23) == MyEnum.enum2);
            test(ro.get(1).size() == 2);
            test(ro.get(1).get(s11) == MyEnum.enum1);
            test(ro.get(1).get(s12) == MyEnum.enum2);

            test(_do.size() == 3);
            test(_do.get(0).size() == 1);
            test(_do.get(0).get(s23) == MyEnum.enum3);
            test(_do.get(1).size() == 2);
            test(_do.get(1).get(s11) == MyEnum.enum1);
            test(_do.get(1).get(s12) == MyEnum.enum2);
            test(_do.get(2).size() == 3);
            test(_do.get(2).get(s11) == MyEnum.enum1);
            test(_do.get(2).get(s22) == MyEnum.enum3);
            test(_do.get(2).get(s23) == MyEnum.enum2);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opByteByteSD extends Callback_MyClass_opByteByteSD
    {

        @Override
        public void response(Map<Byte, byte[]> ro, Map<Byte, byte[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get((byte) 0xf1).length == 2);
            test(_do.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(_do.get((byte) 0xf1)[1] == (byte) 0xf3);
            test(ro.size() == 3);
            test(ro.get((byte) 0x01).length == 2);
            test(ro.get((byte) 0x01)[0] == (byte) 0x01);
            test(ro.get((byte) 0x01)[1] == (byte) 0x11);
            test(ro.get((byte) 0x22).length == 1);
            test(ro.get((byte) 0x22)[0] == (byte) 0x12);
            test(ro.get((byte) 0xf1).length == 2);
            test(ro.get((byte) 0xf1)[0] == (byte) 0xf2);
            test(ro.get((byte) 0xf1)[1] == (byte) 0xf3);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opBoolBoolSD extends Callback_MyClass_opBoolBoolSD
    {

        @Override
        public void response(Map<Boolean, boolean[]> ro, Map<Boolean, boolean[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get(false).length == 2);
            test(_do.get(false)[0]);
            test(!_do.get(false)[1]);
            test(ro.size() == 2);
            test(ro.get(false).length == 2);
            test(ro.get(false)[0]);
            test(!ro.get(false)[1]);
            test(ro.get(true).length == 3);
            test(!ro.get(true)[0]);
            test(ro.get(true)[1]);
            test(ro.get(true)[2]);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opShortShortSD extends Callback_MyClass_opShortShortSD
    {

        @Override
        public void response(Map<Short, short[]> ro, Map<Short, short[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get((short) 4).length == 2);
            test(_do.get((short) 4)[0] == 6);
            test(_do.get((short) 4)[1] == 7);
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opIntIntSD extends Callback_MyClass_opIntIntSD
    {

        @Override
        public void response(Map<Integer, int[]> ro, Map<Integer, int[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get(400).length == 2);
            test(_do.get(400)[0] == 600);
            test(_do.get(400)[1] == 700);
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opLongLongSD extends Callback_MyClass_opLongLongSD
    {

        @Override
        public void response(Map<Long, long[]> ro, Map<Long, long[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get(999999992L).length == 2);
            test(_do.get(999999992L)[0] == 999999110L);
            test(_do.get(999999992L)[1] == 999999120);
            test(ro.size() == 3);
            test(ro.get(999999990L).length == 3);
            test(ro.get(999999990L)[0] == 999999110);
            test(ro.get(999999990L)[1] == 999999111);
            test(ro.get(999999990L)[2] == 999999110);
            test(ro.get(999999991L).length == 2);
            test(ro.get(999999991L)[0] == 999999120);
            test(ro.get(999999991L)[1] == 999999130);
            test(ro.get(999999992L).length == 2);
            test(ro.get(999999992L)[0] == 999999110);
            test(ro.get(999999992L)[1] == 999999120);
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opStringFloatSD extends Callback_MyClass_opStringFloatSD
    {

        @Override
        public void response(Map<String, float[]> ro, Map<String, float[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get("aBc").length == 2);
            test(_do.get("aBc")[0] == -3.14f);
            test(_do.get("aBc")[1] == 3.14f);
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opStringDoubleSD extends Callback_MyClass_opStringDoubleSD
    {

        @Override
        public void response(Map<String, double[]> ro, Map<String, double[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get("").length == 2);
            test(_do.get("")[0] == 1.6E10);
            test(_do.get("")[1] == 1.7E10);
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opStringStringSD extends Callback_MyClass_opStringStringSD
    {

        @Override
        public void response(Map<String, String[]> ro, Map<String, String[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get("ghi").length== 2);
            test(_do.get("ghi")[0].equals("and"));
            test(_do.get("ghi")[1].equals("xor"));
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }
    private static class opMyEnumMyEnumSD extends Callback_MyClass_opMyEnumMyEnumSD
    {

        @Override
        public void response(Map<MyEnum, MyEnum[]> ro, Map<MyEnum, MyEnum[]> _do)
        {
            test(_do.size() == 1);
            test(_do.get(MyEnum.enum1).length == 2);
            test(_do.get(MyEnum.enum1)[0] == MyEnum.enum3);
            test(_do.get(MyEnum.enum1)[1] == MyEnum.enum3);
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
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opIntSI extends Callback_MyClass_opIntS
    {
        opIntSI(int l)
        {
            _l = l;
        }

        @Override
        public void response(int[] r)
        {
            test(r.length == _l);
            for(int j = 0; j < r.length; ++j)
            {
                test(r[j] == -j);
            }
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private int _l;
        private Callback callback = new Callback();
    }

    private static class opDerivedI extends Callback_MyDerivedClass_opDerived
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opDoubleMarshalingI extends Callback_MyClass_opDoubleMarshaling
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opIdempotentI extends Callback_MyClass_opIdempotent
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
        {
            callback.check();
        }

        private Callback callback = new Callback();
    }

    private static class opNonmutatingI extends Callback_MyClass_opNonmutating
    {
        @Override
        public void response()
        {
            callback.called();
        }

        @Override
        public void exception(Ice.LocalException ex)
        {
            test(false);
        }

        public void check()
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
            pingI cb = new pingI();
            p.begin_ice_ping(cb);
            cb.check();
        }

        {
            isAI cb = new isAI();
            p.begin_ice_isA(MyClass.ice_staticId(), cb);
            cb.check();
        }

        {
            idI cb = new idI();
            p.begin_ice_id(cb);
            cb.check();
        }

        {
            idsI cb = new idsI();
            p.begin_ice_ids(cb);
            cb.check();
        }

        {
            Ice.AsyncResult r = p.begin_opVoid();
            p.end_opVoid(r);
        }

        {
            opVoidI cb = new opVoidI();
            p.begin_opVoid(cb);
            cb.check();
        }

        {
            Ice.AsyncResult r = p.begin_opByte((byte)0xff, (byte)0x0f);
            Ice.ByteHolder p3 = new Ice.ByteHolder();
            byte ret = p.end_opByte(p3, r);
            test(p3.value == (byte)0xf0);
            test(ret == (byte)0xff);
        }

        {
            opByteI cb = new opByteI();
            p.begin_opByte((byte)0xff, (byte)0x0f, cb);
            cb.check();
        }

        {
            opBoolI cb = new opBoolI();
            p.begin_opBool(true, false, cb);
            cb.check();
        }

        {
            opShortIntLongI cb = new opShortIntLongI();
            p.begin_opShortIntLong((short)10, 11, 12L, cb);
            cb.check();
        }

        {
            opFloatDoubleI cb = new opFloatDoubleI();
            p.begin_opFloatDouble(3.14f, 1.1E10, cb);
            cb.check();
        }

        {
            opStringI cb = new opStringI();
            p.begin_opString("hello", "world", cb);
            cb.check();
        }

        {
            opMyEnumI cb = new opMyEnumI();
            p.begin_opMyEnum(MyEnum.enum2, cb);
            cb.check();
        }

        {
            opMyClassI cb = new opMyClassI(communicator);
            p.begin_opMyClass(p, cb);
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
            p.begin_opStruct(si1, si2, cb);
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
            p.begin_opByteS(bsi1, bsi2, cb);
            cb.check();
        }

        {
            final boolean[] bsi1 = { true, true, false };
            final boolean[] bsi2 = { false };

            opBoolSI cb = new opBoolSI();
            p.begin_opBoolS(bsi1, bsi2, cb);
            cb.check();
        }

        {
            final short[] ssi = { 1, 2, 3 };
            final int[] isi = { 5, 6, 7, 8 };
            final long[] lsi = { 10, 30, 20 };

            opShortIntLongSI cb = new opShortIntLongSI();
            p.begin_opShortIntLongS(ssi, isi, lsi, cb);
            cb.check();
        }

        {
            final float[] fsi = { 3.14f, 1.11f };
            final double[] dsi = { 1.1E10, 1.2E10, 1.3E10 };

            opFloatDoubleSI cb = new opFloatDoubleSI();
            p.begin_opFloatDoubleS(fsi, dsi, cb);
            cb.check();
        }

        {
            final String[] ssi1 = { "abc", "de", "fghi" };
            final String[] ssi2 = { "xyz" };

            opStringSI cb = new opStringSI();
            p.begin_opStringS(ssi1, ssi2, cb);
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
            p.begin_opByteSS(bsi1, bsi2, cb);
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
            p.begin_opBoolSS(bsi1, bsi2, cb);
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
            p.begin_opShortIntLongSS(ssi, isi, lsi, cb);
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
            p.begin_opFloatDoubleSS(fsi, dsi, cb);
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
            p.begin_opStringSS(ssi1, ssi2, cb);
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
            p.begin_opStringSSS(sssi1, sssi2, cb);
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

            opByteBoolDI cb = new opByteBoolDI();
            p.begin_opByteBoolD(di1, di2, cb);
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

            opShortIntDI cb = new opShortIntDI();
            p.begin_opShortIntD(di1, di2, cb);
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

            opLongFloatDI cb = new opLongFloatDI();
            p.begin_opLongFloatD(di1, di2, cb);
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

            opStringStringDI cb = new opStringStringDI();
            p.begin_opStringStringD(di1, di2, cb);
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

            opStringMyEnumDI cb = new opStringMyEnumDI();
            p.begin_opStringMyEnumD(di1, di2, cb);
            cb.check();
        }

        {
            java.util.Map<MyEnum, String> di1 = new java.util.HashMap<>();
            di1.put(MyEnum.enum1, "abc");
            java.util.Map<MyEnum, String> di2 = new java.util.HashMap<>();
            di2.put(MyEnum.enum2, "Hello!!");
            di2.put(MyEnum.enum3, "qwerty");

            opMyEnumStringDI cb = new opMyEnumStringDI();
            p.begin_opMyEnumStringD(di1, di2, cb);
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

            opMyStructMyEnumDI cb = new opMyStructMyEnumDI();
            p.begin_opMyStructMyEnumD(di1, di2, cb);
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

            opByteBoolDS cb = new opByteBoolDS();
            p.begin_opByteBoolDS(dsi1, dsi2, cb);
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

            opShortIntDS cb = new opShortIntDS();
            p.begin_opShortIntDS(dsi1, dsi2, cb);
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

            opLongFloatDS cb = new opLongFloatDS();
            p.begin_opLongFloatDS(dsi1, dsi2, cb);
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

            opStringStringDS cb = new opStringStringDS();
            p.begin_opStringStringDS(dsi1, dsi2, cb);
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

            opStringMyEnumDS cb = new opStringMyEnumDS();
            p.begin_opStringMyEnumDS(dsi1, dsi2, cb);
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

            opMyEnumStringDS cb = new opMyEnumStringDS();
            p.begin_opMyEnumStringDS(dsi1, dsi2, cb);
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

            opMyStructMyEnumDS cb = new opMyStructMyEnumDS();
            p.begin_opMyStructMyEnumDS(dsi1, dsi2, cb);
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

            opByteByteSD cb = new opByteByteSD();
            p.begin_opByteByteSD(sdi1, sdi2, cb);
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

            opBoolBoolSD cb = new opBoolBoolSD();
            p.begin_opBoolBoolSD(sdi1, sdi2, cb);
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

            opShortShortSD cb = new opShortShortSD();
            p.begin_opShortShortSD(sdi1, sdi2, cb);
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

            opIntIntSD cb = new opIntIntSD();
            p.begin_opIntIntSD(sdi1, sdi2, cb);
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

            opLongLongSD cb = new opLongLongSD();
            p.begin_opLongLongSD(sdi1, sdi2, cb);
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

            opStringFloatSD cb = new opStringFloatSD();
            p.begin_opStringFloatSD(sdi1, sdi2, cb);
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

            opStringDoubleSD cb = new opStringDoubleSD();
            p.begin_opStringDoubleSD(sdi1, sdi2, cb);
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

            opStringStringSD cb = new opStringStringSD();
            p.begin_opStringStringSD(sdi1, sdi2, cb);
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

            opMyEnumMyEnumSD cb = new opMyEnumMyEnumSD();
            p.begin_opMyEnumMyEnumSD(sdi1, sdi2, cb);
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
                p.begin_opIntS(s, cb);
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
                Ice.AsyncResult r = p.begin_opContext();
                java.util.Map<String, String> c = p.end_opContext(r);
                test(!c.equals(ctx));
            }
            {
                test(p.ice_getContext().isEmpty());
                Ice.AsyncResult r = p.begin_opContext(ctx);
                java.util.Map<String, String> c = p.end_opContext(r);
                test(c.equals(ctx));
            }
            MyClassPrx p2 = MyClassPrxHelper.checkedCast(p.ice_context(ctx));
            test(p2.ice_getContext().equals(ctx));
            {
                Ice.AsyncResult r = p2.begin_opContext();
                java.util.Map<String, String> c = p2.end_opContext(r);
                test(c.equals(ctx));
            }
            {
                Ice.AsyncResult r = p2.begin_opContext(ctx);
                java.util.Map<String, String> c = p2.end_opContext(r);
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
                Ice.InitializationData initData = app.createInitializationData();
                initData.properties = communicator.getProperties()._clone();
                initData.properties.setProperty("Ice.ImplicitContext", impls[i]);

                Ice.Communicator ic = app.initialize(initData);

                java.util.Map<String, String> ctx = new java.util.HashMap<>();
                ctx.put("one", "ONE");
                ctx.put("two", "TWO");
                ctx.put("three", "THREE");

                MyClassPrx p3 = MyClassPrxHelper.uncheckedCast(ic.stringToProxy("test:default -p 12010"));

                ic.getImplicitContext().setContext(ctx);
                test(ic.getImplicitContext().getContext().equals(ctx));
                {
                    Ice.AsyncResult r = p3.begin_opContext();
                    java.util.Map<String, String> c = p3.end_opContext(r);
                    test(c.equals(ctx));
                }

                ic.getImplicitContext().put("zero", "ZERO");

                ctx = ic.getImplicitContext().getContext();
                {
                    Ice.AsyncResult r = p3.begin_opContext();
                    java.util.Map<String, String> c = p3.end_opContext(r);
                    test(c.equals(ctx));
                }

                java.util.Map<String, String> prxContext = new java.util.HashMap<>();
                prxContext.put("one", "UN");
                prxContext.put("four", "QUATRE");

                java.util.Map<String, String> combined = new java.util.HashMap<>(ctx);
                combined.putAll(prxContext);
                test(combined.get("one").equals("UN"));

                p3 = MyClassPrxHelper.uncheckedCast(p3.ice_context(prxContext));

                ic.getImplicitContext().setContext(null);
                {
                    Ice.AsyncResult r = p3.begin_opContext();
                    java.util.Map<String, String> c = p3.end_opContext(r);
                    test(c.equals(prxContext));
                }

                ic.getImplicitContext().setContext(ctx);
                {
                    Ice.AsyncResult r = p3.begin_opContext();
                    java.util.Map<String, String> c = p3.end_opContext(r);
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
            opDoubleMarshalingI cb = new opDoubleMarshalingI();
            p.begin_opDoubleMarshaling(d, ds, cb);
            cb.check();
        }

        {
            opIdempotentI cb = new opIdempotentI();
            p.begin_opIdempotent(cb);
            cb.check();
        }

        {
            opNonmutatingI cb = new opNonmutatingI();
            p.begin_opNonmutating(cb);
            cb.check();
        }

        {
            MyDerivedClassPrx derived = MyDerivedClassPrxHelper.checkedCast(p);
            test(derived != null);
            opDerivedI cb = new opDerivedI();
            derived.begin_opDerived(cb);
            cb.check();
        }
        
        {
            final GenericCallback<Byte> cb = new GenericCallback<Byte>((byte)0);
            p.begin_opByte1((byte)0xFF,
                new test.Ice.operations.Test.Callback_MyClass_opByte1()
                    {
                        public void response(byte value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == (byte)0xFF);
        }
        
        {
            final GenericCallback<Short> cb = new GenericCallback<Short>((short)0);
            p.begin_opShort1((short)0x7FFF,
                new test.Ice.operations.Test.Callback_MyClass_opShort1()
                    {
                        public void response(short value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == 0x7FFF);
        }
        
        {
            final GenericCallback<Integer> cb = new GenericCallback<Integer>(0);
            p.begin_opInt1(0x7FFFFFFF,
                new test.Ice.operations.Test.Callback_MyClass_opInt1()
                    {
                        public void response(int value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == 0x7FFFFFFF);
        }
        
        {
            final GenericCallback<Long> cb = new GenericCallback<Long>((long)0);
            p.begin_opLong1(0x7FFFFFFF,
                new test.Ice.operations.Test.Callback_MyClass_opLong1()
                    {
                        public void response(long value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == 0x7FFFFFFF);
        }
        
        {
            final GenericCallback<Float> cb = new GenericCallback<Float>(0.0f);
            p.begin_opFloat1(1.0f,
                new test.Ice.operations.Test.Callback_MyClass_opFloat1()
                    {
                        public void response(float value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == 1.0f);
        }
        
        {
            final GenericCallback<Double> cb = new GenericCallback<Double>(0.0);
            p.begin_opDouble1(1.0,
                new test.Ice.operations.Test.Callback_MyClass_opDouble1()
                    {
                        public void response(double value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value() == 1.0);
        }
        
        {
            final GenericCallback<String> cb = new GenericCallback<String>("");
            p.begin_opString1("opString1",
                new test.Ice.operations.Test.Callback_MyClass_opString1()
                    {
                        public void response(String value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value().equals("opString1"));
        }
        
        {
            final GenericCallback<String[]> cb = new GenericCallback<String[]>(null);
            p.begin_opStringS1(null,
                new test.Ice.operations.Test.Callback_MyClass_opStringS1()
                    {
                        public void response(String[] value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value().length == 0);
        }
        
        {
            final GenericCallback<Map<Byte, Boolean>> cb = new GenericCallback<Map<Byte, Boolean>>(null);
            p.begin_opByteBoolD1(null,
                new test.Ice.operations.Test.Callback_MyClass_opByteBoolD1()
                    {
                        public void response(Map<Byte, Boolean> value)
                        {
                            cb.response(value);
                        }
                        
                        public void exception(Ice.LocalException ex)
                        {
                            cb.exception(ex);
                        }
                    });
            test(cb.succeeded() && cb.value().size() == 0);
        }
    }
}
