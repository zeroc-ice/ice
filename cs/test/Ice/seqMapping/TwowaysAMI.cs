// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;
using Test;

public class TwowaysAMI
{
    private static void test(bool b)
    {
        if(!b)
        {
            throw new System.SystemException();
        }
    }

    private class Callback
    {
        internal Callback()
        {
            _called = false;
        }

        public virtual void check()
        {
            _m.Lock();
            try
            {
                while(!_called)
                {
                    _m.Wait();
                }
        
                _called = false;
            }
            finally
            {
                _m.Unlock();
            }
        }

        public virtual void called()
        {
            _m.Lock();
            try
            {
                Debug.Assert(!_called);
                _called = true;
                _m.Notify();
            }
            finally
            {
                _m.Unlock();
            }
        }

        private bool _called;
        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    private class AMI_MyClass_opAByteSI : Test.AMI_MyClass_opAByteS
    {
        public AMI_MyClass_opAByteSI(byte[] i)
        {
            _i = i;
        }

        public override void ice_response(byte[] r, byte[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
            callback.check();
        }

        private byte[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLByteSI : Test.AMI_MyClass_opLByteS
    {
        public AMI_MyClass_opLByteSI(List<byte> i)
        {
            _i = i;
        }

        public override void ice_response(List<byte> r, List<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKByteSI : Test.AMI_MyClass_opKByteS
    {
        public AMI_MyClass_opKByteSI(LinkedList<byte> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<byte> r, LinkedList<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQByteSI : Test.AMI_MyClass_opQByteS
    {
        public AMI_MyClass_opQByteSI(Queue<byte> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<byte> r, Queue<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSByteSI : Test.AMI_MyClass_opSByteS
    {
        public AMI_MyClass_opSByteSI(Stack<byte> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<byte> r, Stack<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCByteSI : Test.AMI_MyClass_opCByteS
    {
        public AMI_MyClass_opCByteSI(CByteS i)
        {
            _i = i;
        }

        public override void ice_response(CByteS r, CByteS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CByteS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opABoolSI : Test.AMI_MyClass_opABoolS
    {
        public AMI_MyClass_opABoolSI(bool[] i)
        {
            _i = i;
        }

        public override void ice_response(bool[] r, bool[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private bool[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLBoolSI : Test.AMI_MyClass_opLBoolS
    {
        public AMI_MyClass_opLBoolSI(List<bool> i)
        {
            _i = i;
        }

        public override void ice_response(List<bool> r, List<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKBoolSI : Test.AMI_MyClass_opKBoolS
    {
        public AMI_MyClass_opKBoolSI(LinkedList<bool> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<bool> r, LinkedList<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQBoolSI : Test.AMI_MyClass_opQBoolS
    {
        public AMI_MyClass_opQBoolSI(Queue<bool> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<bool> r, Queue<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSBoolSI : Test.AMI_MyClass_opSBoolS
    {
        public AMI_MyClass_opSBoolSI(Stack<bool> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<bool> r, Stack<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCBoolSI : Test.AMI_MyClass_opCBoolS
    {
        public AMI_MyClass_opCBoolSI(CBoolS i)
        {
            _i = i;
        }

        public override void ice_response(CBoolS r, CBoolS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CBoolS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAShortSI : Test.AMI_MyClass_opAShortS
    {
        public AMI_MyClass_opAShortSI(short[] i)
        {
            _i = i;
        }

        public override void ice_response(short[] r, short[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private short[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLShortSI : Test.AMI_MyClass_opLShortS
    {
        public AMI_MyClass_opLShortSI(List<short> i)
        {
            _i = i;
        }

        public override void ice_response(List<short> r, List<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKShortSI : Test.AMI_MyClass_opKShortS
    {
        public AMI_MyClass_opKShortSI(LinkedList<short> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<short> r, LinkedList<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQShortSI : Test.AMI_MyClass_opQShortS
    {
        public AMI_MyClass_opQShortSI(Queue<short> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<short> r, Queue<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSShortSI : Test.AMI_MyClass_opSShortS
    {
        public AMI_MyClass_opSShortSI(Stack<short> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<short> r, Stack<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCShortSI : Test.AMI_MyClass_opCShortS
    {
        public AMI_MyClass_opCShortSI(CShortS i)
        {
            _i = i;
        }

        public override void ice_response(CShortS r, CShortS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CShortS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAIntSI : Test.AMI_MyClass_opAIntS
    {
        public AMI_MyClass_opAIntSI(int[] i)
        {
            _i = i;
        }

        public override void ice_response(int[] r, int[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private int[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLIntSI : Test.AMI_MyClass_opLIntS
    {
        public AMI_MyClass_opLIntSI(List<int> i)
        {
            _i = i;
        }

        public override void ice_response(List<int> r, List<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKIntSI : Test.AMI_MyClass_opKIntS
    {
        public AMI_MyClass_opKIntSI(LinkedList<int> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<int> r, LinkedList<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQIntSI : Test.AMI_MyClass_opQIntS
    {
        public AMI_MyClass_opQIntSI(Queue<int> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<int> r, Queue<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSIntSI : Test.AMI_MyClass_opSIntS
    {
        public AMI_MyClass_opSIntSI(Stack<int> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<int> r, Stack<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCIntSI : Test.AMI_MyClass_opCIntS
    {
        public AMI_MyClass_opCIntSI(CIntS i)
        {
            _i = i;
        }

        public override void ice_response(CIntS r, CIntS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CIntS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opALongSI : Test.AMI_MyClass_opALongS
    {
        public AMI_MyClass_opALongSI(long[] i)
        {
            _i = i;
        }

        public override void ice_response(long[] r, long[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private long[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLLongSI : Test.AMI_MyClass_opLLongS
    {
        public AMI_MyClass_opLLongSI(List<long> i)
        {
            _i = i;
        }

        public override void ice_response(List<long> r, List<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKLongSI : Test.AMI_MyClass_opKLongS
    {
        public AMI_MyClass_opKLongSI(LinkedList<long> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<long> r, LinkedList<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQLongSI : Test.AMI_MyClass_opQLongS
    {
        public AMI_MyClass_opQLongSI(Queue<long> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<long> r, Queue<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSLongSI : Test.AMI_MyClass_opSLongS
    {
        public AMI_MyClass_opSLongSI(Stack<long> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<long> r, Stack<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCLongSI : Test.AMI_MyClass_opCLongS
    {
        public AMI_MyClass_opCLongSI(CLongS i)
        {
            _i = i;
        }

        public override void ice_response(CLongS r, CLongS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CLongS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAFloatSI : Test.AMI_MyClass_opAFloatS
    {
        public AMI_MyClass_opAFloatSI(float[] i)
        {
            _i = i;
        }

        public override void ice_response(float[] r, float[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private float[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLFloatSI : Test.AMI_MyClass_opLFloatS
    {
        public AMI_MyClass_opLFloatSI(List<float> i)
        {
            _i = i;
        }

        public override void ice_response(List<float> r, List<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKFloatSI : Test.AMI_MyClass_opKFloatS
    {
        public AMI_MyClass_opKFloatSI(LinkedList<float> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<float> r, LinkedList<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQFloatSI : Test.AMI_MyClass_opQFloatS
    {
        public AMI_MyClass_opQFloatSI(Queue<float> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<float> r, Queue<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSFloatSI : Test.AMI_MyClass_opSFloatS
    {
        public AMI_MyClass_opSFloatSI(Stack<float> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<float> r, Stack<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCFloatSI : Test.AMI_MyClass_opCFloatS
    {
        public AMI_MyClass_opCFloatSI(CFloatS i)
        {
            _i = i;
        }

        public override void ice_response(CFloatS r, CFloatS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CFloatS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opADoubleSI : Test.AMI_MyClass_opADoubleS
    {
        public AMI_MyClass_opADoubleSI(double[] i)
        {
            _i = i;
        }

        public override void ice_response(double[] r, double[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private double[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLDoubleSI : Test.AMI_MyClass_opLDoubleS
    {
        public AMI_MyClass_opLDoubleSI(List<double> i)
        {
            _i = i;
        }

        public override void ice_response(List<double> r, List<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKDoubleSI : Test.AMI_MyClass_opKDoubleS
    {
        public AMI_MyClass_opKDoubleSI(LinkedList<double> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<double> r, LinkedList<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQDoubleSI : Test.AMI_MyClass_opQDoubleS
    {
        public AMI_MyClass_opQDoubleSI(Queue<double> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<double> r, Queue<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSDoubleSI : Test.AMI_MyClass_opSDoubleS
    {
        public AMI_MyClass_opSDoubleSI(Stack<double> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<double> r, Stack<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCDoubleSI : Test.AMI_MyClass_opCDoubleS
    {
        public AMI_MyClass_opCDoubleSI(CDoubleS i)
        {
            _i = i;
        }

        public override void ice_response(CDoubleS r, CDoubleS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CDoubleS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAStringSI : Test.AMI_MyClass_opAStringS
    {
        public AMI_MyClass_opAStringSI(string[] i)
        {
            _i = i;
        }

        public override void ice_response(string[] r, string[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private string[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLStringSI : Test.AMI_MyClass_opLStringS
    {
        public AMI_MyClass_opLStringSI(List<string> i)
        {
            _i = i;
        }

        public override void ice_response(List<string> r, List<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKStringSI : Test.AMI_MyClass_opKStringS
    {
        public AMI_MyClass_opKStringSI(LinkedList<string> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<string> r, LinkedList<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQStringSI : Test.AMI_MyClass_opQStringS
    {
        public AMI_MyClass_opQStringSI(Queue<string> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<string> r, Queue<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSStringSI : Test.AMI_MyClass_opSStringS
    {
        public AMI_MyClass_opSStringSI(Stack<string> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<string> r, Stack<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCStringSI : Test.AMI_MyClass_opCStringS
    {
        public AMI_MyClass_opCStringSI(CStringS i)
        {
            _i = i;
        }

        public override void ice_response(CStringS r, CStringS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CStringS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAObjectSI : Test.AMI_MyClass_opAObjectS
    {
        public AMI_MyClass_opAObjectSI(Ice.Object[] i)
        {
            _i = i;
        }

        public override void ice_response(Ice.Object[] r, Ice.Object[] o)
        {
            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Ice.Object[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLObjectSI : Test.AMI_MyClass_opLObjectS
    {
        public AMI_MyClass_opLObjectSI(List<Ice.Object> i)
        {
            _i = i;
        }

        public override void ice_response(List<Ice.Object> r, List<Ice.Object> o)
        {
            IEnumerator<Ice.Object> eo = o.GetEnumerator();
            IEnumerator<Ice.Object> er = r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<Ice.Object> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCObjectSI : Test.AMI_MyClass_opCObjectS
    {
        public AMI_MyClass_opCObjectSI(CObjectS i)
        {
            _i = i;
        }

        public override void ice_response(CObjectS r, CObjectS o)
        {
            IEnumerator<Ice.Object> eo = (IEnumerator<Ice.Object>)o.GetEnumerator();
            IEnumerator<Ice.Object> er = (IEnumerator<Ice.Object>)r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CObjectS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAObjectPrxSI : Test.AMI_MyClass_opAObjectPrxS
    {
        public AMI_MyClass_opAObjectPrxSI(Ice.ObjectPrx[] i)
        {
            _i = i;
        }

        public override void ice_response(Ice.ObjectPrx[] r, Ice.ObjectPrx[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Ice.ObjectPrx[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLObjectPrxSI : Test.AMI_MyClass_opLObjectPrxS
    {
        public AMI_MyClass_opLObjectPrxSI(List<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public override void ice_response(List<Ice.ObjectPrx> r, List<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKObjectPrxSI : Test.AMI_MyClass_opKObjectPrxS
    {
        public AMI_MyClass_opKObjectPrxSI(LinkedList<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<Ice.ObjectPrx> r, LinkedList<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQObjectPrxSI : Test.AMI_MyClass_opQObjectPrxS
    {
        public AMI_MyClass_opQObjectPrxSI(Queue<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<Ice.ObjectPrx> r, Queue<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSObjectPrxSI : Test.AMI_MyClass_opSObjectPrxS
    {
        public AMI_MyClass_opSObjectPrxSI(Stack<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<Ice.ObjectPrx> r, Stack<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCObjectPrxSI : Test.AMI_MyClass_opCObjectPrxS
    {
        public AMI_MyClass_opCObjectPrxSI(CObjectPrxS i)
        {
            _i = i;
        }

        public override void ice_response(CObjectPrxS r, CObjectPrxS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CObjectPrxS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAStructSI : Test.AMI_MyClass_opAStructS
    {
        public AMI_MyClass_opAStructSI(S[] i)
        {
            _i = i;
        }

        public override void ice_response(S[] r, S[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private S[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLStructSI : Test.AMI_MyClass_opLStructS
    {
        public AMI_MyClass_opLStructSI(List<S> i)
        {
            _i = i;
        }

        public override void ice_response(List<S> r, List<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKStructSI : Test.AMI_MyClass_opKStructS
    {
        public AMI_MyClass_opKStructSI(LinkedList<S> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<S> r, LinkedList<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQStructSI : Test.AMI_MyClass_opQStructS
    {
        public AMI_MyClass_opQStructSI(Queue<S> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<S> r, Queue<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSStructSI : Test.AMI_MyClass_opSStructS
    {
        public AMI_MyClass_opSStructSI(Stack<S> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<S> r, Stack<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCStructSI : Test.AMI_MyClass_opCStructS
    {
        public AMI_MyClass_opCStructSI(CStructS i)
        {
            _i = i;
        }

        public override void ice_response(CStructS r, CStructS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CStructS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAStructSDI : Test.AMI_MyClass_opAStructSD
    {
        public AMI_MyClass_opAStructSDI(SD[] i)
        {
            _i = i;
        }

        public override void ice_response(SD[] r, SD[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private SD[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLStructSDI : Test.AMI_MyClass_opLStructSD
    {
        public AMI_MyClass_opLStructSDI(List<SD> i)
        {
            _i = i;
        }

        public override void ice_response(List<SD> r, List<SD> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<SD> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKStructSDI : Test.AMI_MyClass_opKStructSD
    {
        public AMI_MyClass_opKStructSDI(LinkedList<SD> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<SD> r, LinkedList<SD> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<SD> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQStructSDI : Test.AMI_MyClass_opQStructSD
    {
        public AMI_MyClass_opQStructSDI(Queue<SD> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<SD> r, Queue<SD> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<SD> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSStructSDI : Test.AMI_MyClass_opSStructSD
    {
        public AMI_MyClass_opSStructSDI(Stack<SD> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<SD> r, Stack<SD> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<SD> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCStructSDI : Test.AMI_MyClass_opCStructSD
    {
        public AMI_MyClass_opCStructSDI(CStructSD i)
        {
            _i = i;
        }

        public override void ice_response(CStructSD r, CStructSD o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CStructSD _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACVSI : Test.AMI_MyClass_opACVS
    {
        public AMI_MyClass_opACVSI(CV[] i)
        {
            _i = i;
        }

        public override void ice_response(CV[] r, CV[] o)
        {
            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CV[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCVSI : Test.AMI_MyClass_opLCVS
    {
        public AMI_MyClass_opLCVSI(List<CV> i)
        {
            _i = i;
        }

        public override void ice_response(List<CV> r, List<CV> o)
        {
            IEnumerator<CV> eo = o.GetEnumerator();
            IEnumerator<CV> er = r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<CV> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCVSI : Test.AMI_MyClass_opCCVS
    {
        public AMI_MyClass_opCCVSI(CCVS i)
        {
            _i = i;
        }

        public override void ice_response(CCVS r, CCVS o)
        {
            IEnumerator<CV> eo = (IEnumerator<CV>)o.GetEnumerator();
            IEnumerator<CV> er = (IEnumerator<CV>)r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.i == ((CV)eo.Current).i);
                test(obj.i == ((CV)er.Current).i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CCVS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACVPrxSI : Test.AMI_MyClass_opACVPrxS
    {
        public AMI_MyClass_opACVPrxSI(CVPrx[] i)
        {
            _i = i;
        }

        public override void ice_response(CVPrx[] r, CVPrx[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CVPrx[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCVPrxSI : Test.AMI_MyClass_opLCVPrxS
    {
        public AMI_MyClass_opLCVPrxSI(List<CVPrx> i)
        {
            _i = i;
        }

        public override void ice_response(List<CVPrx> r, List<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKCVPrxSI : Test.AMI_MyClass_opKCVPrxS
    {
        public AMI_MyClass_opKCVPrxSI(LinkedList<CVPrx> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<CVPrx> r, LinkedList<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQCVPrxSI : Test.AMI_MyClass_opQCVPrxS
    {
        public AMI_MyClass_opQCVPrxSI(Queue<CVPrx> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<CVPrx> r, Queue<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSCVPrxSI : Test.AMI_MyClass_opSCVPrxS
    {
        public AMI_MyClass_opSCVPrxSI(Stack<CVPrx> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<CVPrx> r, Stack<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCVPrxSI : Test.AMI_MyClass_opCCVPrxS
    {
        public AMI_MyClass_opCCVPrxSI(CCVPrxS i)
        {
            _i = i;
        }

        public override void ice_response(CCVPrxS r, CCVPrxS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CCVPrxS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACRSI : Test.AMI_MyClass_opACRS
    {
        public AMI_MyClass_opACRSI(CR[] i)
        {
            _i = i;
        }

        public override void ice_response(CR[] r, CR[] o)
        {
            System.Collections.IEnumerator eo = o.GetEnumerator();
            System.Collections.IEnumerator er = r.GetEnumerator();
            foreach(CR obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((CR)eo.Current).v.i);
                test(obj.v.i == ((CR)er.Current).v.i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CR[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCRSI : Test.AMI_MyClass_opLCRS
    {
        public AMI_MyClass_opLCRSI(List<CR> i)
        {
            _i = i;
        }

        public override void ice_response(List<CR> r, List<CR> o)
        {
            IEnumerator<CR> eo = o.GetEnumerator();
            IEnumerator<CR> er = r.GetEnumerator();
            foreach(CR obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((CR)eo.Current).v.i);
                test(obj.v.i == ((CR)er.Current).v.i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<CR> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCRSI : Test.AMI_MyClass_opCCRS
    {
        public AMI_MyClass_opCCRSI(CCRS i)
        {
            _i = i;
        }

        public override void ice_response(CCRS r, CCRS o)
        {
            IEnumerator<CR> eo = (IEnumerator<CR>)o.GetEnumerator();
            IEnumerator<CR> er = (IEnumerator<CR>)r.GetEnumerator();
            foreach(CR obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                test(obj.v.i == ((CR)eo.Current).v.i);
                test(obj.v.i == ((CR)er.Current).v.i);
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CCRS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAEnSI : Test.AMI_MyClass_opAEnS
    {
        public AMI_MyClass_opAEnSI(En[] i)
        {
            _i = i;
        }

        public override void ice_response(En[] r, En[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private En[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLEnSI : Test.AMI_MyClass_opLEnS
    {
        public AMI_MyClass_opLEnSI(List<En> i)
        {
            _i = i;
        }

        public override void ice_response(List<En> r, List<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private List<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKEnSI : Test.AMI_MyClass_opKEnS
    {
        public AMI_MyClass_opKEnSI(LinkedList<En> i)
        {
            _i = i;
        }

        public override void ice_response(LinkedList<En> r, LinkedList<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private LinkedList<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQEnSI : Test.AMI_MyClass_opQEnS
    {
        public AMI_MyClass_opQEnSI(Queue<En> i)
        {
            _i = i;
        }

        public override void ice_response(Queue<En> r, Queue<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Queue<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSEnSI : Test.AMI_MyClass_opSEnS
    {
        public AMI_MyClass_opSEnSI(Stack<En> i)
        {
            _i = i;
        }

        public override void ice_response(Stack<En> r, Stack<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Stack<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCEnSI : Test.AMI_MyClass_opCEnS
    {
        public AMI_MyClass_opCEnSI(CEnS i)
        {
            _i = i;
        }

        public override void ice_response(CEnS r, CEnS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private CEnS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCustomIntSI : Test.AMI_MyClass_opCustomIntS
    {
        public AMI_MyClass_opCustomIntSI(Custom<int> i)
        {
            _i = i;
        }

        public override void ice_response(Custom<int> r, Custom<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Custom<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCustomCVSI : Test.AMI_MyClass_opCustomCVS
    {
        public AMI_MyClass_opCustomCVSI(Custom<CV> i)
        {
            _i = i;
        }

        public override void ice_response(Custom<CV> r, Custom<CV> o)
        {
            IEnumerator<CV> eo = (IEnumerator<CV>)o.GetEnumerator();
            IEnumerator<CV> er = (IEnumerator<CV>)r.GetEnumerator();
            foreach(CV obj in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                if(obj == null)
                {
                    test(eo.Current == null);
                    test(er.Current == null);
                }
                else
                {
                    test(obj.i == ((CV)eo.Current).i);
                    test(obj.i == ((CV)er.Current).i);
                }
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Custom<CV> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCustomIntSSI : Test.AMI_MyClass_opCustomIntSS
    {
        public AMI_MyClass_opCustomIntSSI(Custom<Custom<int>> i)
        {
            _i = i;
        }

        public override void ice_response(Custom<Custom<int>> r, Custom<Custom<int>> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Custom<Custom<int>> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCustomCVSSI : Test.AMI_MyClass_opCustomCVSS
    {
        public AMI_MyClass_opCustomCVSSI(Custom<Custom<CV>> i)
        {
            _i = i;
        }

        public override void ice_response(Custom<Custom<CV>> r, Custom<Custom<CV>> o)
        {
            IEnumerator<Custom<CV>> eo = (IEnumerator<Custom<CV>>)o.GetEnumerator();
            IEnumerator<Custom<CV>> er = (IEnumerator<Custom<CV>>)r.GetEnumerator();
            foreach(Custom<CV> s in _i)
            {
                eo.MoveNext();
                er.MoveNext();
                IEnumerator<CV> io = (IEnumerator<CV>)eo.Current.GetEnumerator();
                IEnumerator<CV> ir = (IEnumerator<CV>)er.Current.GetEnumerator();
                foreach(CV obj in s)
                {
                    io.MoveNext();
                    ir.MoveNext();
                    if(obj == null)
                    {
                        test(io.Current == null);
                        test(ir.Current == null);
                    }
                    else
                    {
                        test(obj.i == io.Current.i);
                        test(obj.i == ir.Current.i);
                    }
                }
            }
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual void check()
        {
             callback.check();
        }

        private Custom<Custom<CV>> _i;
        private Callback callback = new Callback();
    }

#if !COMPACT && !SILVERLIGHT
    private class AMI_MyClass_opSerialSmallCSharpNull : Test.AMI_MyClass_opSerialSmallCSharp
    {
        public AMI_MyClass_opSerialSmallCSharpNull()
        {
        }

        public override void ice_response(Serialize.Small r, Serialize.Small o)
        {
            test(o == null);
            test(r == null);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.OperationNotExistException); // OK, talking to non-C# server.
        }

        public virtual void check()
        {
             callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSerialSmallCSharp : Test.AMI_MyClass_opSerialSmallCSharp
    {
        public AMI_MyClass_opSerialSmallCSharp()
        {
        }

        public override void ice_response(Serialize.Small r, Serialize.Small o)
        {
            test(o.i == 99);
            test(r.i == 99);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.OperationNotExistException); // OK, talking to non-C# server.
        }

        public virtual void check()
        {
             callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSerialLargeCSharp : Test.AMI_MyClass_opSerialLargeCSharp
    {
        public AMI_MyClass_opSerialLargeCSharp()
        {
        }

        public override void ice_response(Serialize.Large r, Serialize.Large o)
        {
            test(o.d1 == 1.0);
            test(o.d2 == 2.0);
            test(o.d3 == 3.0);
            test(o.d4 == 4.0);
            test(o.d5 == 5.0);
            test(o.d6 == 6.0);
            test(o.d7 == 7.0);
            test(o.d8 == 8.0);
            test(o.d9 == 9.0);
            test(o.d10 == 10.0);
            test(r.d1 == 1.0);
            test(r.d2 == 2.0);
            test(r.d3 == 3.0);
            test(r.d4 == 4.0);
            test(r.d5 == 5.0);
            test(r.d6 == 6.0);
            test(r.d7 == 7.0);
            test(r.d8 == 8.0);
            test(r.d9 == 9.0);
            test(r.d10 == 10.0);
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.OperationNotExistException); // OK, talking to non-C# server.
        }

        public virtual void check()
        {
             callback.check();
        }

        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSerialStructCSharp : Test.AMI_MyClass_opSerialStructCSharp
    {
        public AMI_MyClass_opSerialStructCSharp()
        {
        }

        public override void ice_response(Serialize.Struct r, Serialize.Struct o)
        {
            test(o.o == null);
            test(o.o2 != null);
            test(((Serialize.Struct)(o.o2)).o == null);
            test(((Serialize.Struct)(o.o2)).o2 == o.o2);
            test(o.s == null);
            test(o.s2.Equals("Hello"));
            test(r.o == null);
            test(r.o2 != null);
            test(((Serialize.Struct)(r.o2)).o == null);
            test(((Serialize.Struct)(r.o2)).o2 == r.o2);
            test(r.s == null);
            test(r.s2.Equals("Hello"));
            callback.called();
        }

        public override void ice_exception(Ice.Exception ex)
        {
            test(ex is Ice.OperationNotExistException); // OK, talking to non-C# server.
        }

        public virtual void check()
        {
             callback.check();
        }

        private Callback callback = new Callback();
    }
#endif

    static int _length = 100;

    internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
    {
        {
            byte[] i = new byte[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (byte)c;
            }

            AMI_MyClass_opAByteSI cb = new AMI_MyClass_opAByteSI(i);
            p.opAByteS_async(cb, i);
            cb.check();
        }

        {
            List<byte> i = new List<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }

            AMI_MyClass_opLByteSI cb = new AMI_MyClass_opLByteSI(i);
            p.opLByteS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<byte> i = new LinkedList<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }

            AMI_MyClass_opKByteSI cb = new AMI_MyClass_opKByteSI(i);
            p.opKByteS_async(cb, i);
            cb.check();
        }

        {
            Queue<byte> i = new Queue<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }

            AMI_MyClass_opQByteSI cb = new AMI_MyClass_opQByteSI(i);
            p.opQByteS_async(cb, i);
            cb.check();
        }

        {
            Stack<byte> i = new Stack<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }

            AMI_MyClass_opSByteSI cb = new AMI_MyClass_opSByteSI(i);
            p.opSByteS_async(cb, i);
            cb.check();
        }

        {
            CByteS i = new CByteS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }

            AMI_MyClass_opCByteSI cb = new AMI_MyClass_opCByteSI(i);
            p.opCByteS_async(cb, i);
            cb.check();
        }

        {
            bool[] i = new bool[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }

            AMI_MyClass_opABoolSI cb = new AMI_MyClass_opABoolSI(i);
            p.opABoolS_async(cb, i);
            cb.check();
        }

        {
            List<bool> i = new List<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }

            AMI_MyClass_opLBoolSI cb = new AMI_MyClass_opLBoolSI(i);
            p.opLBoolS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<bool> i = new LinkedList<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }

            AMI_MyClass_opKBoolSI cb = new AMI_MyClass_opKBoolSI(i);
            p.opKBoolS_async(cb, i);
            cb.check();
        }

        {
            Queue<bool> i = new Queue<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }

            AMI_MyClass_opQBoolSI cb = new AMI_MyClass_opQBoolSI(i);
            p.opQBoolS_async(cb, i);
            cb.check();
        }

        {
            Stack<bool> i = new Stack<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }

            AMI_MyClass_opSBoolSI cb = new AMI_MyClass_opSBoolSI(i);
            p.opSBoolS_async(cb, i);
            cb.check();
        }

        {
            CBoolS i = new CBoolS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }

            AMI_MyClass_opCBoolSI cb = new AMI_MyClass_opCBoolSI(i);
            p.opCBoolS_async(cb, i);
            cb.check();
        }

        {
            short[] i = new short[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (short)c;
            }

            AMI_MyClass_opAShortSI cb = new AMI_MyClass_opAShortSI(i);
            p.opAShortS_async(cb, i);
            cb.check();
        }

        {
            List<short> i = new List<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }

            AMI_MyClass_opLShortSI cb = new AMI_MyClass_opLShortSI(i);
            p.opLShortS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<short> i = new LinkedList<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }

            AMI_MyClass_opKShortSI cb = new AMI_MyClass_opKShortSI(i);
            p.opKShortS_async(cb, i);
            cb.check();
        }

        {
            Queue<short> i = new Queue<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }

            AMI_MyClass_opQShortSI cb = new AMI_MyClass_opQShortSI(i);
            p.opQShortS_async(cb, i);
            cb.check();
        }

        {
            Stack<short> i = new Stack<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }

            AMI_MyClass_opSShortSI cb = new AMI_MyClass_opSShortSI(i);
            p.opSShortS_async(cb, i);
            cb.check();
        }

        {
            CShortS i = new CShortS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }

            AMI_MyClass_opCShortSI cb = new AMI_MyClass_opCShortSI(i);
            p.opCShortS_async(cb, i);
            cb.check();
        }

        {
            int[] i = new int[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (int)c;
            }

            AMI_MyClass_opAIntSI cb = new AMI_MyClass_opAIntSI(i);
            p.opAIntS_async(cb, i);
            cb.check();
        }

        {
            List<int> i = new List<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }

            AMI_MyClass_opLIntSI cb = new AMI_MyClass_opLIntSI(i);
            p.opLIntS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<int> i = new LinkedList<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((int)c);
            }

            AMI_MyClass_opKIntSI cb = new AMI_MyClass_opKIntSI(i);
            p.opKIntS_async(cb, i);
            cb.check();
        }

        {
            Queue<int> i = new Queue<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((int)c);
            }

            AMI_MyClass_opQIntSI cb = new AMI_MyClass_opQIntSI(i);
            p.opQIntS_async(cb, i);
            cb.check();
        }

        {
            Stack<int> i = new Stack<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((int)c);
            }

            AMI_MyClass_opSIntSI cb = new AMI_MyClass_opSIntSI(i);
            p.opSIntS_async(cb, i);
            cb.check();
        }

        {
            CIntS i = new CIntS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }

            AMI_MyClass_opCIntSI cb = new AMI_MyClass_opCIntSI(i);
            p.opCIntS_async(cb, i);
            cb.check();
        }

        {
            long[] i = new long[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (long)c;
            }

            AMI_MyClass_opALongSI cb = new AMI_MyClass_opALongSI(i);
            p.opALongS_async(cb, i);
            cb.check();
        }

        {
            List<long> i = new List<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }

            AMI_MyClass_opLLongSI cb = new AMI_MyClass_opLLongSI(i);
            p.opLLongS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<long> i = new LinkedList<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }

            AMI_MyClass_opKLongSI cb = new AMI_MyClass_opKLongSI(i);
            p.opKLongS_async(cb, i);
            cb.check();
        }

        {
            Queue<long> i = new Queue<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((long)c);
            }

            AMI_MyClass_opQLongSI cb = new AMI_MyClass_opQLongSI(i);
            p.opQLongS_async(cb, i);
            cb.check();
        }

        {
            Stack<long> i = new Stack<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((long)c);
            }

            AMI_MyClass_opSLongSI cb = new AMI_MyClass_opSLongSI(i);
            p.opSLongS_async(cb, i);
            cb.check();
        }

        {
            CLongS i = new CLongS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }

            AMI_MyClass_opCLongSI cb = new AMI_MyClass_opCLongSI(i);
            p.opCLongS_async(cb, i);
            cb.check();
        }

        {
            float[] i = new float[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (float)c;
            }

            AMI_MyClass_opAFloatSI cb = new AMI_MyClass_opAFloatSI(i);
            p.opAFloatS_async(cb, i);
            cb.check();
        }

        {
            List<float> i = new List<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }

            AMI_MyClass_opLFloatSI cb = new AMI_MyClass_opLFloatSI(i);
            p.opLFloatS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<float> i = new LinkedList<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((float)c);
            }

            AMI_MyClass_opKFloatSI cb = new AMI_MyClass_opKFloatSI(i);
            p.opKFloatS_async(cb, i);
            cb.check();
        }

        {
            Queue<float> i = new Queue<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((float)c);
            }

            AMI_MyClass_opQFloatSI cb = new AMI_MyClass_opQFloatSI(i);
            p.opQFloatS_async(cb, i);
            cb.check();
        }

        {
            Stack<float> i = new Stack<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((float)c);
            }

            AMI_MyClass_opSFloatSI cb = new AMI_MyClass_opSFloatSI(i);
            p.opSFloatS_async(cb, i);
            cb.check();
        }

        {
            CFloatS i = new CFloatS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }

            AMI_MyClass_opCFloatSI cb = new AMI_MyClass_opCFloatSI(i);
            p.opCFloatS_async(cb, i);
            cb.check();
        }

        {
            double[] i = new double[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (double)c;
            }

            AMI_MyClass_opADoubleSI cb = new AMI_MyClass_opADoubleSI(i);
            p.opADoubleS_async(cb, i);
            cb.check();
        }

        {
            List<double> i = new List<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }

            AMI_MyClass_opLDoubleSI cb = new AMI_MyClass_opLDoubleSI(i);
            p.opLDoubleS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<double> i = new LinkedList<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((double)c);
            }

            AMI_MyClass_opKDoubleSI cb = new AMI_MyClass_opKDoubleSI(i);
            p.opKDoubleS_async(cb, i);
            cb.check();
        }

        {
            Queue<double> i = new Queue<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((double)c);
            }

            AMI_MyClass_opQDoubleSI cb = new AMI_MyClass_opQDoubleSI(i);
            p.opQDoubleS_async(cb, i);
            cb.check();
        }

        {
            Stack<double> i = new Stack<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((double)c);
            }

            AMI_MyClass_opSDoubleSI cb = new AMI_MyClass_opSDoubleSI(i);
            p.opSDoubleS_async(cb, i);
            cb.check();
        }

        {
            CDoubleS i = new CDoubleS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }

            AMI_MyClass_opCDoubleSI cb = new AMI_MyClass_opCDoubleSI(i);
            p.opCDoubleS_async(cb, i);
            cb.check();
        }

        {
            string[] i = new string[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c.ToString();
            }

            AMI_MyClass_opAStringSI cb = new AMI_MyClass_opAStringSI(i);
            p.opAStringS_async(cb, i);
            cb.check();
        }

        {
            List<string> i = new List<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }

            AMI_MyClass_opLStringSI cb = new AMI_MyClass_opLStringSI(i);
            p.opLStringS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<string> i = new LinkedList<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c.ToString());
            }

            AMI_MyClass_opKStringSI cb = new AMI_MyClass_opKStringSI(i);
            p.opKStringS_async(cb, i);
            cb.check();
        }

        {
            Queue<string> i = new Queue<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c.ToString());
            }

            AMI_MyClass_opQStringSI cb = new AMI_MyClass_opQStringSI(i);
            p.opQStringS_async(cb, i);
            cb.check();
        }

        {
            Stack<string> i = new Stack<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c.ToString());
            }

            AMI_MyClass_opSStringSI cb = new AMI_MyClass_opSStringSI(i);
            p.opSStringS_async(cb, i);
            cb.check();
        }

        {
            CStringS i = new CStringS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }

            AMI_MyClass_opCStringSI cb = new AMI_MyClass_opCStringSI(i);
            p.opCStringS_async(cb, i);
            cb.check();
        }

        {
            Ice.Object[] i = new Ice.Object[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }

            AMI_MyClass_opAObjectSI cb = new AMI_MyClass_opAObjectSI(i);
            p.opAObjectS_async(cb, i);
            cb.check();
        }

        {
            List<Ice.Object> i = new List<Ice.Object>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opLObjectSI cb = new AMI_MyClass_opLObjectSI(i);
            p.opLObjectS_async(cb, i);
            cb.check();
        }

        {
            CObjectS i = new CObjectS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opCObjectSI cb = new AMI_MyClass_opCObjectSI(i);
            p.opCObjectS_async(cb, i);
            cb.check();
        }

        {
            Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }

            AMI_MyClass_opAObjectPrxSI cb = new AMI_MyClass_opAObjectPrxSI(i);
            p.opAObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opLObjectPrxSI cb = new AMI_MyClass_opLObjectPrxSI(i);
            p.opLObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opKObjectPrxSI cb = new AMI_MyClass_opKObjectPrxSI(i);
            p.opKObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opQObjectPrxSI cb = new AMI_MyClass_opQObjectPrxSI(i);
            p.opQObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opSObjectPrxSI cb = new AMI_MyClass_opSObjectPrxSI(i);
            p.opSObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            CObjectPrxS i = new CObjectPrxS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opCObjectPrxSI cb = new AMI_MyClass_opCObjectPrxSI(i);
            p.opCObjectPrxS_async(cb, i);
            cb.check();
        }

        {
            S[] i = new S[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }

            AMI_MyClass_opAStructSI cb = new AMI_MyClass_opAStructSI(i);
            p.opAStructS_async(cb, i);
            cb.check();
        }

        {
            List<S> i = new List<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new S(c));
            }

            AMI_MyClass_opLStructSI cb = new AMI_MyClass_opLStructSI(i);
            p.opLStructS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<S> i = new LinkedList<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(new S(c));
            }

            AMI_MyClass_opKStructSI cb = new AMI_MyClass_opKStructSI(i);
            p.opKStructS_async(cb, i);
            cb.check();
        }

        {
            Queue<S> i = new Queue<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(new S(c));
            }

            AMI_MyClass_opQStructSI cb = new AMI_MyClass_opQStructSI(i);
            p.opQStructS_async(cb, i);
            cb.check();
        }

        {
            Stack<S> i = new Stack<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(new S(c));
            }

            AMI_MyClass_opSStructSI cb = new AMI_MyClass_opSStructSI(i);
            p.opSStructS_async(cb, i);
            cb.check();
        }

        {
            CStructS i = new CStructS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new S(c));
            }

            AMI_MyClass_opCStructSI cb = new AMI_MyClass_opCStructSI(i);
            p.opCStructS_async(cb, i);
            cb.check();
        }

        {
            SD[] i = new SD[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new SD(c);
            }

            AMI_MyClass_opAStructSDI cb = new AMI_MyClass_opAStructSDI(i);
            p.opAStructSD_async(cb, i);
            cb.check();
        }

        {
            List<SD> i = new List<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new SD(c));
            }

            AMI_MyClass_opLStructSDI cb = new AMI_MyClass_opLStructSDI(i);
            p.opLStructSD_async(cb, i);
            cb.check();
        }

        {
            LinkedList<SD> i = new LinkedList<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(new SD(c));
            }

            AMI_MyClass_opKStructSDI cb = new AMI_MyClass_opKStructSDI(i);
            p.opKStructSD_async(cb, i);
            cb.check();
        }

        {
            Queue<SD> i = new Queue<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(new SD(c));
            }

            AMI_MyClass_opQStructSDI cb = new AMI_MyClass_opQStructSDI(i);
            p.opQStructSD_async(cb, i);
            cb.check();
        }

        {
            Stack<SD> i = new Stack<SD>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(new SD(c));
            }

            AMI_MyClass_opSStructSDI cb = new AMI_MyClass_opSStructSDI(i);
            p.opSStructSD_async(cb, i);
            cb.check();
        }

        {
            CStructSD i = new CStructSD();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new SD(c));
            }

            AMI_MyClass_opCStructSDI cb = new AMI_MyClass_opCStructSDI(i);
            p.opCStructSD_async(cb, i);
            cb.check();
        }

        {
            CV[] i = new CV[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }

            AMI_MyClass_opACVSI cb = new AMI_MyClass_opACVSI(i);
            p.opACVS_async(cb, i);
            cb.check();
        }

        {
            List<CV> i = new List<CV>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opLCVSI cb = new AMI_MyClass_opLCVSI(i);
            p.opLCVS_async(cb, i);
            cb.check();
        }

        {
            CCVS i = new CCVS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opCCVSI cb = new AMI_MyClass_opCCVSI(i);
            p.opCCVS_async(cb, i);
            cb.check();
        }

        {
            CVPrx[] i = new CVPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opACVPrxSI cb = new AMI_MyClass_opACVPrxSI(i);
            p.opACVPrxS_async(cb, i);
            cb.check();
        }

        {
            List<CVPrx> i = new List<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opLCVPrxSI cb = new AMI_MyClass_opLCVPrxSI(i);
            p.opLCVPrxS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<CVPrx> i = new LinkedList<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opKCVPrxSI cb = new AMI_MyClass_opKCVPrxSI(i);
            p.opKCVPrxS_async(cb, i);
            cb.check();
        }

        {
            Queue<CVPrx> i = new Queue<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opQCVPrxSI cb = new AMI_MyClass_opQCVPrxSI(i);
            p.opQCVPrxS_async(cb, i);
            cb.check();
        }

        {
            Stack<CVPrx> i = new Stack<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opSCVPrxSI cb = new AMI_MyClass_opSCVPrxSI(i);
            p.opSCVPrxS_async(cb, i);
            cb.check();
        }

        {
            CCVPrxS i = new CCVPrxS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opCCVPrxSI cb = new AMI_MyClass_opCCVPrxSI(i);
            p.opCCVPrxS_async(cb, i);
            cb.check();
        }

        {
            CR[] i = new CR[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CR(new CV(c));
            }

            AMI_MyClass_opACRSI cb = new AMI_MyClass_opACRSI(i);
            p.opACRS_async(cb, i);
            cb.check();
        }

        {
            List<CR> i = new List<CR>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CR(new CV(c)));
            }

            AMI_MyClass_opLCRSI cb = new AMI_MyClass_opLCRSI(i);
            p.opLCRS_async(cb, i);
            cb.check();
        }

        {
            CCRS i = new CCRS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CR(new CV(c)));
            }

            AMI_MyClass_opCCRSI cb = new AMI_MyClass_opCCRSI(i);
            p.opCCRS_async(cb, i);
            cb.check();
        }

        {
            En[] i = new En[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (En)(c % 3);
            }

            AMI_MyClass_opAEnSI cb = new AMI_MyClass_opAEnSI(i);
            p.opAEnS_async(cb, i);
            cb.check();
        }

        {
            List<En> i = new List<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((En)(c % 3));
            }

            AMI_MyClass_opLEnSI cb = new AMI_MyClass_opLEnSI(i);
            p.opLEnS_async(cb, i);
            cb.check();
        }

        {
            LinkedList<En> i = new LinkedList<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((En)(c % 3));
            }

            AMI_MyClass_opKEnSI cb = new AMI_MyClass_opKEnSI(i);
            p.opKEnS_async(cb, i);
            cb.check();
        }

        {
            Queue<En> i = new Queue<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((En)(c % 3));
            }

            AMI_MyClass_opQEnSI cb = new AMI_MyClass_opQEnSI(i);
            p.opQEnS_async(cb, i);
            cb.check();
        }

        {
            Stack<En> i = new Stack<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((En)(c % 3));
            }

            AMI_MyClass_opSEnSI cb = new AMI_MyClass_opSEnSI(i);
            p.opSEnS_async(cb, i);
            cb.check();
        }

        {
            CEnS i = new CEnS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((En)(c % 3));
            }

            AMI_MyClass_opCEnSI cb = new AMI_MyClass_opCEnSI(i);
            p.opCEnS_async(cb, i);
            cb.check();
        }

        {
            Custom<int> i = new Custom<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c);
            }

            AMI_MyClass_opCustomIntSI cb = new AMI_MyClass_opCustomIntSI(i);
            p.opCustomIntS_async(cb, i);
            cb.check();
        }

        {
            Custom<CV> i = new Custom<CV>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opCustomCVSI cb = new AMI_MyClass_opCustomCVSI(i);
            p.opCustomCVS_async(cb, i);
            cb.check();
        }

        {
            Custom<Custom<int>> i = new Custom<Custom<int>>();
            for(int c = 0; c < _length; ++c)
            {
                Custom<int> inner = new Custom<int>();
                for(int j = 0; j < c; ++j)
                {
                    inner.Add(j);
                }
                i.Add(inner);
            }

            AMI_MyClass_opCustomIntSSI cb = new AMI_MyClass_opCustomIntSSI(i);
            p.opCustomIntSS_async(cb, i);
            cb.check();
        }

        {
            Custom<Custom<CV>> i = new Custom<Custom<CV>>();
            for(int c = 0; c < _length; ++c)
            {
                Custom<CV> inner = new Custom<CV>();
                for(int j = 0; j < c; ++j)
                {
                    inner.Add(new CV(j));
                }
                i.Add(inner);
            }

            AMI_MyClass_opCustomCVSSI cb = new AMI_MyClass_opCustomCVSSI(i);
            p.opCustomCVSS_async(cb, i);
            cb.check();
        }

#if !COMPACT && !SILVERLIGHT
        {
            Serialize.Small i = null;

            AMI_MyClass_opSerialSmallCSharpNull cb = new AMI_MyClass_opSerialSmallCSharpNull();
            p.opSerialSmallCSharp_async(cb, i);
            cb.check();
        }

        {
            Serialize.Small i = new Serialize.Small();
            i.i = 99;

            AMI_MyClass_opSerialSmallCSharp cb = new AMI_MyClass_opSerialSmallCSharp();
            p.opSerialSmallCSharp_async(cb, i);
            cb.check();
        }

        {
            Serialize.Large i = new Serialize.Large();
            i.d1 = 1.0;
            i.d2 = 2.0;
            i.d3 = 3.0;
            i.d4 = 4.0;
            i.d5 = 5.0;
            i.d6 = 6.0;
            i.d7 = 7.0;
            i.d8 = 8.0;
            i.d9 = 9.0;
            i.d10 = 10.0;

            AMI_MyClass_opSerialLargeCSharp cb = new AMI_MyClass_opSerialLargeCSharp();
            p.opSerialLargeCSharp_async(cb, i);
            cb.check();
        }

        {
            Serialize.Struct i = new Serialize.Struct();
            i.o = null;
            i.o2 = i;
            i.s = null;
            i.s2 = "Hello";

            AMI_MyClass_opSerialStructCSharp cb = new AMI_MyClass_opSerialStructCSharp();
            p.opSerialStructCSharp_async(cb, i);
            cb.check();
        }
#endif
    }
}
