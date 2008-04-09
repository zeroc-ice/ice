// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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

        public virtual bool check()
        {
            int cnt = 0;
            do
            {
                lock(this)
                {
                    if(_called)
                    {
                        return true;
                    }
                }
                Thread.Sleep(100);
            }
            while(++cnt < 50);

            return false; // Must be timeout
        }

        public virtual void called()
        {
            lock(this)
            {
                Debug.Assert(!_called);
                _called = true;
            }
        }

        private bool _called;
    }

    private class AMI_MyClass_opAByteSI 
    {
        public AMI_MyClass_opAByteSI(byte[] i)
        {
            _i = i;
        }

        public void response(byte[] r, byte[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private byte[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLByteSI 
    {
        public AMI_MyClass_opLByteSI(List<byte> i)
        {
            _i = i;
        }

        public void response(List<byte> r, List<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKByteSI 
    {
        public AMI_MyClass_opKByteSI(LinkedList<byte> i)
        {
            _i = i;
        }

        public void response(LinkedList<byte> r, LinkedList<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQByteSI
    {
        public AMI_MyClass_opQByteSI(Queue<byte> i)
        {
            _i = i;
        }

        public void response(Queue<byte> r, Queue<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSByteSI 
    {
        public AMI_MyClass_opSByteSI(Stack<byte> i)
        {
            _i = i;
        }

        public void response(Stack<byte> r, Stack<byte> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<byte> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCByteSI 
    {
        public AMI_MyClass_opCByteSI(CByteS i)
        {
            _i = i;
        }

        public void response(CByteS r, CByteS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CByteS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opABoolSI 
    {
        public AMI_MyClass_opABoolSI(bool[] i)
        {
            _i = i;
        }

        public void response(bool[] r, bool[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private bool[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLBoolSI 
    {
        public AMI_MyClass_opLBoolSI(List<bool> i)
        {
            _i = i;
        }

        public void response(List<bool> r, List<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKBoolSI 
    {
        public AMI_MyClass_opKBoolSI(LinkedList<bool> i)
        {
            _i = i;
        }

        public void response(LinkedList<bool> r, LinkedList<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQBoolSI 
    {
        public AMI_MyClass_opQBoolSI(Queue<bool> i)
        {
            _i = i;
        }

        public void response(Queue<bool> r, Queue<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSBoolSI 
    {
        public AMI_MyClass_opSBoolSI(Stack<bool> i)
        {
            _i = i;
        }

        public void response(Stack<bool> r, Stack<bool> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<bool> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCBoolSI 
    {
        public AMI_MyClass_opCBoolSI(CBoolS i)
        {
            _i = i;
        }

        public void response(CBoolS r, CBoolS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CBoolS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAShortSI 
    {
        public AMI_MyClass_opAShortSI(short[] i)
        {
            _i = i;
        }

        public void response(short[] r, short[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private short[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLShortSI 
    {
        public AMI_MyClass_opLShortSI(List<short> i)
        {
            _i = i;
        }

        public void response(List<short> r, List<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKShortSI 
    {
        public AMI_MyClass_opKShortSI(LinkedList<short> i)
        {
            _i = i;
        }

        public void response(LinkedList<short> r, LinkedList<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQShortSI 
    {
        public AMI_MyClass_opQShortSI(Queue<short> i)
        {
            _i = i;
        }

        public void response(Queue<short> r, Queue<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSShortSI 
    {
        public AMI_MyClass_opSShortSI(Stack<short> i)
        {
            _i = i;
        }

        public void response(Stack<short> r, Stack<short> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<short> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCShortSI 
    {
        public AMI_MyClass_opCShortSI(CShortS i)
        {
            _i = i;
        }

        public void response(CShortS r, CShortS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CShortS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAIntSI 
    {
        public AMI_MyClass_opAIntSI(int[] i)
        {
            _i = i;
        }

        public void response(int[] r, int[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private int[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLIntSI 
    {
        public AMI_MyClass_opLIntSI(List<int> i)
        {
            _i = i;
        }

        public void response(List<int> r, List<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKIntSI 
    {
        public AMI_MyClass_opKIntSI(LinkedList<int> i)
        {
            _i = i;
        }

        public void response(LinkedList<int> r, LinkedList<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQIntSI 
    {
        public AMI_MyClass_opQIntSI(Queue<int> i)
        {
            _i = i;
        }

        public void response(Queue<int> r, Queue<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSIntSI 
    {
        public AMI_MyClass_opSIntSI(Stack<int> i)
        {
            _i = i;
        }

        public void response(Stack<int> r, Stack<int> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<int> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCIntSI 
    {
        public AMI_MyClass_opCIntSI(CIntS i)
        {
            _i = i;
        }

        public void response(CIntS r, CIntS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CIntS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opALongSI 
    {
        public AMI_MyClass_opALongSI(long[] i)
        {
            _i = i;
        }

        public void response(long[] r, long[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private long[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLLongSI 
    {
        public AMI_MyClass_opLLongSI(List<long> i)
        {
            _i = i;
        }

        public void response(List<long> r, List<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKLongSI 
    {
        public AMI_MyClass_opKLongSI(LinkedList<long> i)
        {
            _i = i;
        }

        public void response(LinkedList<long> r, LinkedList<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQLongSI 
    {
        public AMI_MyClass_opQLongSI(Queue<long> i)
        {
            _i = i;
        }

        public void response(Queue<long> r, Queue<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSLongSI 
    {
        public AMI_MyClass_opSLongSI(Stack<long> i)
        {
            _i = i;
        }

        public void response(Stack<long> r, Stack<long> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<long> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCLongSI 
    {
        public AMI_MyClass_opCLongSI(CLongS i)
        {
            _i = i;
        }

        public void response(CLongS r, CLongS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CLongS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAFloatSI 
    {
        public AMI_MyClass_opAFloatSI(float[] i)
        {
            _i = i;
        }

        public void response(float[] r, float[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private float[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLFloatSI 
    {
        public AMI_MyClass_opLFloatSI(List<float> i)
        {
            _i = i;
        }

        public void response(List<float> r, List<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKFloatSI 
    {
        public AMI_MyClass_opKFloatSI(LinkedList<float> i)
        {
            _i = i;
        }

        public void response(LinkedList<float> r, LinkedList<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQFloatSI 
    {
        public AMI_MyClass_opQFloatSI(Queue<float> i)
        {
            _i = i;
        }

        public void response(Queue<float> r, Queue<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSFloatSI 
    {
        public AMI_MyClass_opSFloatSI(Stack<float> i)
        {
            _i = i;
        }

        public void response(Stack<float> r, Stack<float> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<float> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCFloatSI 
    {
        public AMI_MyClass_opCFloatSI(CFloatS i)
        {
            _i = i;
        }

        public void response(CFloatS r, CFloatS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CFloatS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opADoubleSI 
    {
        public AMI_MyClass_opADoubleSI(double[] i)
        {
            _i = i;
        }

        public void response(double[] r, double[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private double[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLDoubleSI 
    {
        public AMI_MyClass_opLDoubleSI(List<double> i)
        {
            _i = i;
        }

        public void response(List<double> r, List<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKDoubleSI 
    {
        public AMI_MyClass_opKDoubleSI(LinkedList<double> i)
        {
            _i = i;
        }

        public void response(LinkedList<double> r, LinkedList<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQDoubleSI 
    {
        public AMI_MyClass_opQDoubleSI(Queue<double> i)
        {
            _i = i;
        }

        public void response(Queue<double> r, Queue<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSDoubleSI 
    {
        public AMI_MyClass_opSDoubleSI(Stack<double> i)
        {
            _i = i;
        }

        public void response(Stack<double> r, Stack<double> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<double> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCDoubleSI 
    {
        public AMI_MyClass_opCDoubleSI(CDoubleS i)
        {
            _i = i;
        }

        public void response(CDoubleS r, CDoubleS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CDoubleS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAStringSI 
    {
        public AMI_MyClass_opAStringSI(string[] i)
        {
            _i = i;
        }

        public void response(string[] r, string[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private string[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLStringSI 
    {
        public AMI_MyClass_opLStringSI(List<string> i)
        {
            _i = i;
        }

        public void response(List<string> r, List<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKStringSI 
    {
        public AMI_MyClass_opKStringSI(LinkedList<string> i)
        {
            _i = i;
        }

        public void response(LinkedList<string> r, LinkedList<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQStringSI 
    {
        public AMI_MyClass_opQStringSI(Queue<string> i)
        {
            _i = i;
        }

        public void response(Queue<string> r, Queue<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSStringSI 
    {
        public AMI_MyClass_opSStringSI(Stack<string> i)
        {
            _i = i;
        }

        public void response(Stack<string> r, Stack<string> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<string> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCStringSI 
    {
        public AMI_MyClass_opCStringSI(CStringS i)
        {
            _i = i;
        }

        public void response(CStringS r, CStringS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CStringS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAObjectSI 
    {
        public AMI_MyClass_opAObjectSI(Ice.Object[] i)
        {
            _i = i;
        }

        public void response(Ice.Object[] r, Ice.Object[] o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Ice.Object[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLObjectSI 
    {
        public AMI_MyClass_opLObjectSI(List<Ice.Object> i)
        {
            _i = i;
        }

        public void response(List<Ice.Object> r, List<Ice.Object> o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<Ice.Object> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCObjectSI 
    {
        public AMI_MyClass_opCObjectSI(CObjectS i)
        {
            _i = i;
        }

        public void response(CObjectS r, CObjectS o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CObjectS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAObjectPrxSI 
    {
        public AMI_MyClass_opAObjectPrxSI(Ice.ObjectPrx[] i)
        {
            _i = i;
        }

        public void response(Ice.ObjectPrx[] r, Ice.ObjectPrx[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Ice.ObjectPrx[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLObjectPrxSI 
    {
        public AMI_MyClass_opLObjectPrxSI(List<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public void response(List<Ice.ObjectPrx> r, List<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKObjectPrxSI 
    {
        public AMI_MyClass_opKObjectPrxSI(LinkedList<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public void response(LinkedList<Ice.ObjectPrx> r, LinkedList<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQObjectPrxSI 
    {
        public AMI_MyClass_opQObjectPrxSI(Queue<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public void response(Queue<Ice.ObjectPrx> r, Queue<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSObjectPrxSI 
    {
        public AMI_MyClass_opSObjectPrxSI(Stack<Ice.ObjectPrx> i)
        {
            _i = i;
        }

        public void response(Stack<Ice.ObjectPrx> r, Stack<Ice.ObjectPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<Ice.ObjectPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCObjectPrxSI 
    {
        public AMI_MyClass_opCObjectPrxSI(CObjectPrxS i)
        {
            _i = i;
        }

        public void response(CObjectPrxS r, CObjectPrxS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CObjectPrxS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAStructSI 
    {
        public AMI_MyClass_opAStructSI(S[] i)
        {
            _i = i;
        }

        public void response(S[] r, S[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private S[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLStructSI 
    {
        public AMI_MyClass_opLStructSI(List<S> i)
        {
            _i = i;
        }

        public void response(List<S> r, List<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKStructSI 
    {
        public AMI_MyClass_opKStructSI(LinkedList<S> i)
        {
            _i = i;
        }

        public void response(LinkedList<S> r, LinkedList<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQStructSI 
    {
        public AMI_MyClass_opQStructSI(Queue<S> i)
        {
            _i = i;
        }

        public void response(Queue<S> r, Queue<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSStructSI 
    {
        public AMI_MyClass_opSStructSI(Stack<S> i)
        {
            _i = i;
        }

        public void response(Stack<S> r, Stack<S> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<S> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCStructSI 
    {
        public AMI_MyClass_opCStructSI(CStructS i)
        {
            _i = i;
        }

        public void response(CStructS r, CStructS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CStructS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACVSI 
    {
        public AMI_MyClass_opACVSI(CV[] i)
        {
            _i = i;
        }

        public void response(CV[] r, CV[] o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CV[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCVSI 
    {
        public AMI_MyClass_opLCVSI(List<CV> i)
        {
            _i = i;
        }

        public void response(List<CV> r, List<CV> o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<CV> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCVSI 
    {
        public AMI_MyClass_opCCVSI(CCVS i)
        {
            _i = i;
        }

        public void response(CCVS r, CCVS o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CCVS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACVPrxSI 
    {
        public AMI_MyClass_opACVPrxSI(CVPrx[] i)
        {
            _i = i;
        }

        public void response(CVPrx[] r, CVPrx[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CVPrx[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCVPrxSI 
    {
        public AMI_MyClass_opLCVPrxSI(List<CVPrx> i)
        {
            _i = i;
        }

        public void response(List<CVPrx> r, List<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKCVPrxSI 
    {
        public AMI_MyClass_opKCVPrxSI(LinkedList<CVPrx> i)
        {
            _i = i;
        }

        public void response(LinkedList<CVPrx> r, LinkedList<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQCVPrxSI 
    {
        public AMI_MyClass_opQCVPrxSI(Queue<CVPrx> i)
        {
            _i = i;
        }

        public void response(Queue<CVPrx> r, Queue<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSCVPrxSI 
    {
        public AMI_MyClass_opSCVPrxSI(Stack<CVPrx> i)
        {
            _i = i;
        }

        public void response(Stack<CVPrx> r, Stack<CVPrx> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<CVPrx> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCVPrxSI 
    {
        public AMI_MyClass_opCCVPrxSI(CCVPrxS i)
        {
            _i = i;
        }

        public void response(CCVPrxS r, CCVPrxS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CCVPrxS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opACRSI 
    {
        public AMI_MyClass_opACRSI(CR[] i)
        {
            _i = i;
        }

        public void response(CR[] r, CR[] o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CR[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLCRSI 
    {
        public AMI_MyClass_opLCRSI(List<CR> i)
        {
            _i = i;
        }

        public void response(List<CR> r, List<CR> o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<CR> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCCRSI 
    {
        public AMI_MyClass_opCCRSI(CCRS i)
        {
            _i = i;
        }

        public void response(CCRS r, CCRS o)
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

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CCRS _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opAEnSI 
    {
        public AMI_MyClass_opAEnSI(En[] i)
        {
            _i = i;
        }

        public void response(En[] r, En[] o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private En[] _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opLEnSI 
    {
        public AMI_MyClass_opLEnSI(List<En> i)
        {
            _i = i;
        }

        public void response(List<En> r, List<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private List<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opKEnSI 
    {
        public AMI_MyClass_opKEnSI(LinkedList<En> i)
        {
            _i = i;
        }

        public void response(LinkedList<En> r, LinkedList<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private LinkedList<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opQEnSI 
    {
        public AMI_MyClass_opQEnSI(Queue<En> i)
        {
            _i = i;
        }

        public void response(Queue<En> r, Queue<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Queue<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opSEnSI 
    {
        public AMI_MyClass_opSEnSI(Stack<En> i)
        {
            _i = i;
        }

        public void response(Stack<En> r, Stack<En> o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private Stack<En> _i;
        private Callback callback = new Callback();
    }

    private class AMI_MyClass_opCEnSI
    {
        public AMI_MyClass_opCEnSI(CEnS i)
        {
            _i = i;
        }

        public void response(CEnS r, CEnS o)
        {
            test(Ice.CollectionComparer.Equals(_i, o));
            test(Ice.CollectionComparer.Equals(_i, r));
            callback.called();
        }

        public void exception(Ice.Exception ex)
        {
            test(false);
        }

        public virtual bool check()
        {
            return callback.check();
        }

        private CEnS _i;
        private Callback callback = new Callback();
    }

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
            p.opAByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<byte> i = new List<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }

            AMI_MyClass_opLByteSI cb = new AMI_MyClass_opLByteSI(i);
            p.opLByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<byte> i = new LinkedList<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((byte)c);
            }

            AMI_MyClass_opKByteSI cb = new AMI_MyClass_opKByteSI(i);
            p.opKByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<byte> i = new Queue<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((byte)c);
            }

            AMI_MyClass_opQByteSI cb = new AMI_MyClass_opQByteSI(i);
            p.opQByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<byte> i = new Stack<byte>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((byte)c);
            }

            AMI_MyClass_opSByteSI cb = new AMI_MyClass_opSByteSI(i);
            p.opSByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CByteS i = new CByteS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((byte)c);
            }

            AMI_MyClass_opCByteSI cb = new AMI_MyClass_opCByteSI(i);
            p.opCByteS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            bool[] i = new bool[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c % 1 == 1;
            }

            AMI_MyClass_opABoolSI cb = new AMI_MyClass_opABoolSI(i);
            p.opABoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<bool> i = new List<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }

            AMI_MyClass_opLBoolSI cb = new AMI_MyClass_opLBoolSI(i);
            p.opLBoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<bool> i = new LinkedList<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c % 1 == 1);
            }

            AMI_MyClass_opKBoolSI cb = new AMI_MyClass_opKBoolSI(i);
            p.opKBoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<bool> i = new Queue<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c % 1 == 1);
            }

            AMI_MyClass_opQBoolSI cb = new AMI_MyClass_opQBoolSI(i);
            p.opQBoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<bool> i = new Stack<bool>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c % 1 == 1);
            }

            AMI_MyClass_opSBoolSI cb = new AMI_MyClass_opSBoolSI(i);
            p.opSBoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CBoolS i = new CBoolS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c % 1 == 1);
            }

            AMI_MyClass_opCBoolSI cb = new AMI_MyClass_opCBoolSI(i);
            p.opCBoolS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            short[] i = new short[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (short)c;
            }

            AMI_MyClass_opAShortSI cb = new AMI_MyClass_opAShortSI(i);
            p.opAShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<short> i = new List<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }

            AMI_MyClass_opLShortSI cb = new AMI_MyClass_opLShortSI(i);
            p.opLShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<short> i = new LinkedList<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((short)c);
            }

            AMI_MyClass_opKShortSI cb = new AMI_MyClass_opKShortSI(i);
            p.opKShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<short> i = new Queue<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((short)c);
            }

            AMI_MyClass_opQShortSI cb = new AMI_MyClass_opQShortSI(i);
            p.opQShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<short> i = new Stack<short>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((short)c);
            }

            AMI_MyClass_opSShortSI cb = new AMI_MyClass_opSShortSI(i);
            p.opSShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CShortS i = new CShortS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((short)c);
            }

            AMI_MyClass_opCShortSI cb = new AMI_MyClass_opCShortSI(i);
            p.opCShortS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            int[] i = new int[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (int)c;
            }

            AMI_MyClass_opAIntSI cb = new AMI_MyClass_opAIntSI(i);
            p.opAIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<int> i = new List<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }

            AMI_MyClass_opLIntSI cb = new AMI_MyClass_opLIntSI(i);
            p.opLIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<int> i = new LinkedList<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((int)c);
            }

            AMI_MyClass_opKIntSI cb = new AMI_MyClass_opKIntSI(i);
            p.opKIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<int> i = new Queue<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((int)c);
            }

            AMI_MyClass_opQIntSI cb = new AMI_MyClass_opQIntSI(i);
            p.opQIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<int> i = new Stack<int>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((int)c);
            }

            AMI_MyClass_opSIntSI cb = new AMI_MyClass_opSIntSI(i);
            p.opSIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CIntS i = new CIntS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((int)c);
            }

            AMI_MyClass_opCIntSI cb = new AMI_MyClass_opCIntSI(i);
            p.opCIntS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            long[] i = new long[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (long)c;
            }

            AMI_MyClass_opALongSI cb = new AMI_MyClass_opALongSI(i);
            p.opALongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<long> i = new List<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }

            AMI_MyClass_opLLongSI cb = new AMI_MyClass_opLLongSI(i);
            p.opLLongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<long> i = new LinkedList<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((long)c);
            }

            AMI_MyClass_opKLongSI cb = new AMI_MyClass_opKLongSI(i);
            p.opKLongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<long> i = new Queue<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((long)c);
            }

            AMI_MyClass_opQLongSI cb = new AMI_MyClass_opQLongSI(i);
            p.opQLongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<long> i = new Stack<long>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((long)c);
            }

            AMI_MyClass_opSLongSI cb = new AMI_MyClass_opSLongSI(i);
            p.opSLongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CLongS i = new CLongS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((long)c);
            }

            AMI_MyClass_opCLongSI cb = new AMI_MyClass_opCLongSI(i);
            p.opCLongS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            float[] i = new float[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (float)c;
            }

            AMI_MyClass_opAFloatSI cb = new AMI_MyClass_opAFloatSI(i);
            p.opAFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<float> i = new List<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }

            AMI_MyClass_opLFloatSI cb = new AMI_MyClass_opLFloatSI(i);
            p.opLFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<float> i = new LinkedList<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((float)c);
            }

            AMI_MyClass_opKFloatSI cb = new AMI_MyClass_opKFloatSI(i);
            p.opKFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<float> i = new Queue<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((float)c);
            }

            AMI_MyClass_opQFloatSI cb = new AMI_MyClass_opQFloatSI(i);
            p.opQFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<float> i = new Stack<float>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((float)c);
            }

            AMI_MyClass_opSFloatSI cb = new AMI_MyClass_opSFloatSI(i);
            p.opSFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CFloatS i = new CFloatS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((float)c);
            }

            AMI_MyClass_opCFloatSI cb = new AMI_MyClass_opCFloatSI(i);
            p.opCFloatS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            double[] i = new double[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (double)c;
            }

            AMI_MyClass_opADoubleSI cb = new AMI_MyClass_opADoubleSI(i);
            p.opADoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<double> i = new List<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }

            AMI_MyClass_opLDoubleSI cb = new AMI_MyClass_opLDoubleSI(i);
            p.opLDoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<double> i = new LinkedList<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((double)c);
            }

            AMI_MyClass_opKDoubleSI cb = new AMI_MyClass_opKDoubleSI(i);
            p.opKDoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<double> i = new Queue<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((double)c);
            }

            AMI_MyClass_opQDoubleSI cb = new AMI_MyClass_opQDoubleSI(i);
            p.opQDoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<double> i = new Stack<double>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((double)c);
            }

            AMI_MyClass_opSDoubleSI cb = new AMI_MyClass_opSDoubleSI(i);
            p.opSDoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CDoubleS i = new CDoubleS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((double)c);
            }

            AMI_MyClass_opCDoubleSI cb = new AMI_MyClass_opCDoubleSI(i);
            p.opCDoubleS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            string[] i = new string[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = c.ToString();
            }

            AMI_MyClass_opAStringSI cb = new AMI_MyClass_opAStringSI(i);
            p.opAStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<string> i = new List<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }

            AMI_MyClass_opLStringSI cb = new AMI_MyClass_opLStringSI(i);
            p.opLStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<string> i = new LinkedList<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(c.ToString());
            }

            AMI_MyClass_opKStringSI cb = new AMI_MyClass_opKStringSI(i);
            p.opKStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<string> i = new Queue<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(c.ToString());
            }

            AMI_MyClass_opQStringSI cb = new AMI_MyClass_opQStringSI(i);
            p.opQStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<string> i = new Stack<string>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(c.ToString());
            }

            AMI_MyClass_opSStringSI cb = new AMI_MyClass_opSStringSI(i);
            p.opSStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CStringS i = new CStringS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(c.ToString());
            }

            AMI_MyClass_opCStringSI cb = new AMI_MyClass_opCStringSI(i);
            p.opCStringS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Ice.Object[] i = new Ice.Object[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }

            AMI_MyClass_opAObjectSI cb = new AMI_MyClass_opAObjectSI(i);
            p.opAObjectS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<Ice.Object> i = new List<Ice.Object>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opLObjectSI cb = new AMI_MyClass_opLObjectSI(i);
            p.opLObjectS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CObjectS i = new CObjectS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opCObjectSI cb = new AMI_MyClass_opCObjectSI(i);
            p.opCObjectS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = communicator.stringToProxy(c.ToString());
            }

            AMI_MyClass_opAObjectPrxSI cb = new AMI_MyClass_opAObjectPrxSI(i);
            p.opAObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opLObjectPrxSI cb = new AMI_MyClass_opLObjectPrxSI(i);
            p.opLObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opKObjectPrxSI cb = new AMI_MyClass_opKObjectPrxSI(i);
            p.opKObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opQObjectPrxSI cb = new AMI_MyClass_opQObjectPrxSI(i);
            p.opQObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opSObjectPrxSI cb = new AMI_MyClass_opSObjectPrxSI(i);
            p.opSObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CObjectPrxS i = new CObjectPrxS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opCObjectPrxSI cb = new AMI_MyClass_opCObjectPrxSI(i);
            p.opCObjectPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            S[] i = new S[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c].i = c;
            }

            AMI_MyClass_opAStructSI cb = new AMI_MyClass_opAStructSI(i);
            p.opAStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<S> i = new List<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new S(c));
            }

            AMI_MyClass_opLStructSI cb = new AMI_MyClass_opLStructSI(i);
            p.opLStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<S> i = new LinkedList<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(new S(c));
            }

            AMI_MyClass_opKStructSI cb = new AMI_MyClass_opKStructSI(i);
            p.opKStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<S> i = new Queue<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(new S(c));
            }

            AMI_MyClass_opQStructSI cb = new AMI_MyClass_opQStructSI(i);
            p.opQStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<S> i = new Stack<S>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(new S(c));
            }

            AMI_MyClass_opSStructSI cb = new AMI_MyClass_opSStructSI(i);
            p.opSStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CStructS i = new CStructS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new S(c));
            }

            AMI_MyClass_opCStructSI cb = new AMI_MyClass_opCStructSI(i);
            p.opCStructS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CV[] i = new CV[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CV(c);
            }

            AMI_MyClass_opACVSI cb = new AMI_MyClass_opACVSI(i);
            p.opACVS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<CV> i = new List<CV>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opLCVSI cb = new AMI_MyClass_opLCVSI(i);
            p.opLCVS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CCVS i = new CCVS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CV(c));
            }

            AMI_MyClass_opCCVSI cb = new AMI_MyClass_opCCVSI(i);
            p.opCCVS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CVPrx[] i = new CVPrx[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
            }

            AMI_MyClass_opACVPrxSI cb = new AMI_MyClass_opACVPrxSI(i);
            p.opACVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<CVPrx> i = new List<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opLCVPrxSI cb = new AMI_MyClass_opLCVPrxSI(i);
            p.opLCVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<CVPrx> i = new LinkedList<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opKCVPrxSI cb = new AMI_MyClass_opKCVPrxSI(i);
            p.opKCVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<CVPrx> i = new Queue<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opQCVPrxSI cb = new AMI_MyClass_opQCVPrxSI(i);
            p.opQCVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<CVPrx> i = new Stack<CVPrx>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opSCVPrxSI cb = new AMI_MyClass_opSCVPrxSI(i);
            p.opSCVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CCVPrxS i = new CCVPrxS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(CVPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
            }

            AMI_MyClass_opCCVPrxSI cb = new AMI_MyClass_opCCVPrxSI(i);
            p.opCCVPrxS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CR[] i = new CR[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = new CR(new CV(c));
            }

            AMI_MyClass_opACRSI cb = new AMI_MyClass_opACRSI(i);
            p.opACRS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<CR> i = new List<CR>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CR(new CV(c)));
            }

            AMI_MyClass_opLCRSI cb = new AMI_MyClass_opLCRSI(i);
            p.opLCRS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CCRS i = new CCRS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add(new CR(new CV(c)));
            }

            AMI_MyClass_opCCRSI cb = new AMI_MyClass_opCCRSI(i);
            p.opCCRS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            En[] i = new En[_length];
            for(int c = 0; c < _length; ++c)
            {
                i[c] = (En)(c % 3);
            }

            AMI_MyClass_opAEnSI cb = new AMI_MyClass_opAEnSI(i);
            p.opAEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            List<En> i = new List<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((En)(c % 3));
            }

            AMI_MyClass_opLEnSI cb = new AMI_MyClass_opLEnSI(i);
            p.opLEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            LinkedList<En> i = new LinkedList<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.AddLast((En)(c % 3));
            }

            AMI_MyClass_opKEnSI cb = new AMI_MyClass_opKEnSI(i);
            p.opKEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Queue<En> i = new Queue<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Enqueue((En)(c % 3));
            }

            AMI_MyClass_opQEnSI cb = new AMI_MyClass_opQEnSI(i);
            p.opQEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            Stack<En> i = new Stack<En>();
            for(int c = 0; c < _length; ++c)
            {
                i.Push((En)(c % 3));
            }

            AMI_MyClass_opSEnSI cb = new AMI_MyClass_opSEnSI(i);
            p.opSEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }

        {
            CEnS i = new CEnS();
            for(int c = 0; c < _length; ++c)
            {
                i.Add((En)(c % 3));
            }

            AMI_MyClass_opCEnSI cb = new AMI_MyClass_opCEnSI(i);
            p.opCEnS_async(cb.response, cb.exception, i);
            test(cb.check());
        }
    }
}
