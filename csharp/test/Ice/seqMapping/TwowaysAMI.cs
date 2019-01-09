// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System.Diagnostics;
using System.Collections.Generic;

namespace Ice
{
    namespace seqMapping
    {
        public class TwowaysAMI
        {
            private static void test(bool b)
            {
                if(!b)
                {
                    throw new System.SystemException();
                }
            }

            private class CallbackBase
            {
                internal CallbackBase()
                {
                    _called = false;
                }

                public virtual void check()
                {
                    lock(this)
                    {
                        while(!_called)
                        {
                            System.Threading.Monitor.Wait(this);
                        }

                        _called = false;
                    }
                }

                public virtual void called()
                {
                    lock(this)
                    {
                        Debug.Assert(!_called);
                        _called = true;
                        System.Threading.Monitor.Pulse(this);
                    }
                }

                private bool _called;
            }

            private class Callback
            {
                public void opAByteSI(Ice.AsyncResult result)
                {
                    byte[] i =(byte[])result.AsyncState;
                    byte[] o;
                    byte[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAByteS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLByteSI(Ice.AsyncResult result)
                {
                    List<byte> i =(List<byte>)result.AsyncState;
                    List<byte> o;
                    List<byte> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLByteS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKByteSI(Ice.AsyncResult result)
                {
                    LinkedList<byte> i =(LinkedList<byte>)result.AsyncState;
                    LinkedList<byte> o;
                    LinkedList<byte> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKByteS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQByteSI(Ice.AsyncResult result)
                {
                    Queue<byte> i =(Queue<byte>)result.AsyncState;
                    Queue<byte> o;
                    Queue<byte> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQByteS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSByteSI(Ice.AsyncResult result)
                {
                    Stack<byte> i =(Stack<byte>)result.AsyncState;
                    Stack<byte> o;
                    Stack<byte> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSByteS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opABoolSI(Ice.AsyncResult result)
                {
                    bool[] i =(bool[])result.AsyncState;
                    bool[] o;
                    bool[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opABoolS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLBoolSI(Ice.AsyncResult result)
                {
                    List<bool> i =(List<bool>)result.AsyncState;
                    List<bool> o;
                    List<bool> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLBoolS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKBoolSI(Ice.AsyncResult result)
                {
                    LinkedList<bool> i =(LinkedList<bool>)result.AsyncState;
                    LinkedList<bool> o;
                    LinkedList<bool> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKBoolS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQBoolSI(Ice.AsyncResult result)
                {
                    Queue<bool> i =(Queue<bool>)result.AsyncState;
                    Queue<bool> o;
                    Queue<bool> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQBoolS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSBoolSI(Ice.AsyncResult result)
                {
                    Stack<bool> i =(Stack<bool>)result.AsyncState;
                    Stack<bool> o;
                    Stack<bool> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSBoolS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAShortSI(Ice.AsyncResult result)
                {
                    short[] i =(short[])result.AsyncState;
                    short[] o;
                    short[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAShortS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLShortSI(Ice.AsyncResult result)
                {
                    List<short> i =(List<short>)result.AsyncState;
                    List<short> o;
                    List<short> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLShortS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKShortSI(Ice.AsyncResult result)
                {
                    LinkedList<short> i =(LinkedList<short>)result.AsyncState;
                    LinkedList<short> o;
                    LinkedList<short> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKShortS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQShortSI(Ice.AsyncResult result)
                {
                    Queue<short> i =(Queue<short>)result.AsyncState;
                    Queue<short> o;
                    Queue<short> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQShortS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSShortSI(Ice.AsyncResult result)
                {
                    Stack<short> i =(Stack<short>)result.AsyncState;
                    Stack<short> o;
                    Stack<short> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSShortS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAIntSI(Ice.AsyncResult result)
                {
                    int[] i =(int[])result.AsyncState;
                    int[] o;
                    int[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLIntSI(Ice.AsyncResult result)
                {
                    List<int> i =(List<int>)result.AsyncState;
                    List<int> o;
                    List<int> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKIntSI(Ice.AsyncResult result)
                {
                    LinkedList<int> i =(LinkedList<int>)result.AsyncState;
                    LinkedList<int> o;
                    LinkedList<int> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQIntSI(Ice.AsyncResult result)
                {
                    Queue<int> i =(Queue<int>)result.AsyncState;
                    Queue<int> o;
                    Queue<int> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSIntSI(Ice.AsyncResult result)
                {
                    Stack<int> i =(Stack<int>)result.AsyncState;
                    Stack<int> o;
                    Stack<int> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opALongSI(Ice.AsyncResult result)
                {
                    long[] i =(long[])result.AsyncState;
                    long[] o;
                    long[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opALongS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLLongSI(Ice.AsyncResult result)
                {
                    List<long> i =(List<long>)result.AsyncState;
                    List<long> o;
                    List<long> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLLongS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKLongSI(Ice.AsyncResult result)
                {
                    LinkedList<long> i =(LinkedList<long>)result.AsyncState;
                    LinkedList<long> o;
                    LinkedList<long> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKLongS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQLongSI(Ice.AsyncResult result)
                {
                    Queue<long> i =(Queue<long>)result.AsyncState;
                    Queue<long> o;
                    Queue<long> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQLongS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSLongSI(Ice.AsyncResult result)
                {
                    Stack<long> i =(Stack<long>)result.AsyncState;
                    Stack<long> o;
                    Stack<long> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSLongS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAFloatSI(Ice.AsyncResult result)
                {
                    float[] i =(float[])result.AsyncState;
                    float[] o;
                    float[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAFloatS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLFloatSI(Ice.AsyncResult result)
                {
                    List<float> i =(List<float>)result.AsyncState;
                    List<float> o;
                    List<float> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLFloatS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKFloatSI(Ice.AsyncResult result)
                {
                    LinkedList<float> i =(LinkedList<float>)result.AsyncState;
                    LinkedList<float> o;
                    LinkedList<float> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKFloatS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQFloatSI(Ice.AsyncResult result)
                {
                    Queue<float> i =(Queue<float>)result.AsyncState;
                    Queue<float> o;
                    Queue<float> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQFloatS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSFloatSI(Ice.AsyncResult result)
                {
                    Stack<float> i =(Stack<float>)result.AsyncState;
                    Stack<float> o;
                    Stack<float> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSFloatS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opADoubleSI(Ice.AsyncResult result)
                {
                    double[] i =(double[])result.AsyncState;
                    double[] o;
                    double[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opADoubleS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLDoubleSI(Ice.AsyncResult result)
                {
                    List<double> i =(List<double>)result.AsyncState;
                    List<double> o;
                    List<double> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLDoubleS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKDoubleSI(Ice.AsyncResult result)
                {
                    LinkedList<double> i =(LinkedList<double>)result.AsyncState;
                    LinkedList<double> o;
                    LinkedList<double> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKDoubleS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQDoubleSI(Ice.AsyncResult result)
                {
                    Queue<double> i =(Queue<double>)result.AsyncState;
                    Queue<double> o;
                    Queue<double> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQDoubleS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSDoubleSI(Ice.AsyncResult result)
                {
                    Stack<double> i =(Stack<double>)result.AsyncState;
                    Stack<double> o;
                    Stack<double> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSDoubleS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAStringSI(Ice.AsyncResult result)
                {
                    string[] i =(string[])result.AsyncState;
                    string[] o;
                    string[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAStringS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLStringSI(Ice.AsyncResult result)
                {
                    List<string> i =(List<string>)result.AsyncState;
                    List<string> o;
                    List<string> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLStringS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKStringSI(Ice.AsyncResult result)
                {
                    LinkedList<string> i =(LinkedList<string>)result.AsyncState;
                    LinkedList<string> o;
                    LinkedList<string> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKStringS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQStringSI(Ice.AsyncResult result)
                {
                    Queue<string> i =(Queue<string>)result.AsyncState;
                    Queue<string> o;
                    Queue<string> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQStringS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSStringSI(Ice.AsyncResult result)
                {
                    Stack<string> i =(Stack<string>)result.AsyncState;
                    Stack<string> o;
                    Stack<string> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSStringS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAObjectSI(Ice.AsyncResult result)
                {
                    Ice.Value[] i =(Ice.Value[])result.AsyncState;
                    Ice.Value[] o;
                    Ice.Value[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAObjectS(out o, result);
                    System.Collections.IEnumerator eo = o.GetEnumerator();
                    System.Collections.IEnumerator er = r.GetEnumerator();
                    foreach(Test.CV obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.i ==((Test.CV)eo.Current).i);
                        test(obj.i ==((Test.CV)er.Current).i);
                    }
                    callback.called();
                }

                public void opLObjectSI(Ice.AsyncResult result)
                {
                    List<Ice.Value> i =(List<Ice.Value>)result.AsyncState;
                    List<Ice.Value> o;
                    List<Ice.Value> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLObjectS(out o, result);
                    IEnumerator<Ice.Value> eo = o.GetEnumerator();
                    IEnumerator<Ice.Value> er = r.GetEnumerator();
                    foreach(Test.CV obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.i ==((Test.CV)eo.Current).i);
                        test(obj.i ==((Test.CV)er.Current).i);
                    }
                    callback.called();
                }

                public void opAObjectPrxSI(Ice.AsyncResult result)
                {
                    Ice.ObjectPrx[] i =(Ice.ObjectPrx[])result.AsyncState;
                    Ice.ObjectPrx[] o;
                    Ice.ObjectPrx[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAObjectPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLObjectPrxSI(Ice.AsyncResult result)
                {
                    List<Ice.ObjectPrx> i =(List<Ice.ObjectPrx>)result.AsyncState;
                    List<Ice.ObjectPrx> o;
                    List<Ice.ObjectPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLObjectPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKObjectPrxSI(Ice.AsyncResult result)
                {
                    LinkedList<Ice.ObjectPrx> i =(LinkedList<Ice.ObjectPrx>)result.AsyncState;
                    LinkedList<Ice.ObjectPrx> o;
                    LinkedList<Ice.ObjectPrx> r =
                        Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKObjectPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQObjectPrxSI(Ice.AsyncResult result)
                {
                    Queue<Ice.ObjectPrx> i =(Queue<Ice.ObjectPrx>)result.AsyncState;
                    Queue<Ice.ObjectPrx> o;
                    Queue<Ice.ObjectPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQObjectPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSObjectPrxSI(Ice.AsyncResult result)
                {
                    Stack<Ice.ObjectPrx> i =(Stack<Ice.ObjectPrx>)result.AsyncState;
                    Stack<Ice.ObjectPrx> o;
                    Stack<Ice.ObjectPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSObjectPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAStructSI(Ice.AsyncResult result)
                {
                    Test.S[] i =(Test.S[])result.AsyncState;
                    Test.S[] o;
                    Test.S[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAStructS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLStructSI(Ice.AsyncResult result)
                {
                    List<Test.S> i =(List<Test.S>)result.AsyncState;
                    List<Test.S> o;
                    List<Test.S> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLStructS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKStructSI(Ice.AsyncResult result)
                {
                    LinkedList<Test.S> i =(LinkedList<Test.S>)result.AsyncState;
                    LinkedList<Test.S> o;
                    LinkedList<Test.S> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKStructS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQStructSI(Ice.AsyncResult result)
                {
                    Queue<Test.S> i =(Queue<Test.S>)result.AsyncState;
                    Queue<Test.S> o;
                    Queue<Test.S> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQStructS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSStructSI(Ice.AsyncResult result)
                {
                    Stack<Test.S> i =(Stack<Test.S>)result.AsyncState;
                    Stack<Test.S> o;
                    Stack<Test.S> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSStructS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opAStructSDI(Ice.AsyncResult result)
                {
                    Test.SD[] i =(Test.SD[])result.AsyncState;
                    Test.SD[] o;
                    Test.SD[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAStructSD(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLStructSDI(Ice.AsyncResult result)
                {
                    List<Test.SD> i =(List<Test.SD>)result.AsyncState;
                    List<Test.SD> o;
                    List<Test.SD> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLStructSD(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKStructSDI(Ice.AsyncResult result)
                {
                    LinkedList<Test.SD> i =(LinkedList<Test.SD>)result.AsyncState;
                    LinkedList<Test.SD> o;
                    LinkedList<Test.SD> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKStructSD(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQStructSDI(Ice.AsyncResult result)
                {
                    Queue<Test.SD> i =(Queue<Test.SD>)result.AsyncState;
                    Queue<Test.SD> o;
                    Queue<Test.SD> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQStructSD(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSStructSDI(Ice.AsyncResult result)
                {
                    Stack<Test.SD> i =(Stack<Test.SD>)result.AsyncState;
                    Stack<Test.SD> o;
                    Stack<Test.SD> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSStructSD(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opACVSI(Ice.AsyncResult result)
                {
                    Test.CV[] i =(Test.CV[])result.AsyncState;
                    Test.CV[] o;
                    Test.CV[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opACVS(out o, result);
                    System.Collections.IEnumerator eo = o.GetEnumerator();
                    System.Collections.IEnumerator er = r.GetEnumerator();
                    foreach(Test.CV obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.i ==((Test.CV)eo.Current).i);
                        test(obj.i ==((Test.CV)er.Current).i);
                    }
                    callback.called();
                }

                public void opLCVSI(Ice.AsyncResult result)
                {
                    List<Test.CV> i =(List<Test.CV>)result.AsyncState;
                    List<Test.CV> o;
                    List<Test.CV> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLCVS(out o, result);
                    IEnumerator<Test.CV> eo = o.GetEnumerator();
                    IEnumerator<Test.CV> er = r.GetEnumerator();
                    foreach(Test.CV obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.i == eo.Current.i);
                        test(obj.i == er.Current.i);
                    }
                    callback.called();
                }

                public void opIPrxSI(Ice.AsyncResult result)
                {
                    Test.IPrx[] i =(Test.IPrx[])result.AsyncState;
                    Test.IPrx[] o;
                    Test.IPrx[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAIPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLIPrxSI(Ice.AsyncResult result)
                {
                    List<Test.IPrx> i =(List<Test.IPrx>)result.AsyncState;
                    List<Test.IPrx> o;
                    List<Test.IPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLIPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKIPrxSI(Ice.AsyncResult result)
                {
                    LinkedList<Test.IPrx> i =(LinkedList<Test.IPrx>)result.AsyncState;
                    LinkedList<Test.IPrx> o;
                    LinkedList<Test.IPrx> r =
                        Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKIPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQIPrxSI(Ice.AsyncResult result)
                {
                    Queue<Test.IPrx> i =(Queue<Test.IPrx>)result.AsyncState;
                    Queue<Test.IPrx> o;
                    Queue<Test.IPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQIPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSIPrxSI(Ice.AsyncResult result)
                {
                    Stack<Test.IPrx> i =(Stack<Test.IPrx>)result.AsyncState;
                    Stack<Test.IPrx> o;
                    Stack<Test.IPrx> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSIPrxS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opACRSI(Ice.AsyncResult result)
                {
                    Test.CR[] i =(Test.CR[])result.AsyncState;
                    Test.CR[] o;
                    Test.CR[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opACRS(out o, result);
                    System.Collections.IEnumerator eo = o.GetEnumerator();
                    System.Collections.IEnumerator er = r.GetEnumerator();
                    foreach(Test.CR obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.v.i ==((Test.CR)eo.Current).v.i);
                        test(obj.v.i ==((Test.CR)er.Current).v.i);
                    }
                    callback.called();
                }

                public void opLCRSI(Ice.AsyncResult result)
                {
                    List<Test.CR> i =(List<Test.CR>)result.AsyncState;
                    List<Test.CR> o;
                    List<Test.CR> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLCRS(out o, result);
                    IEnumerator<Test.CR> eo = o.GetEnumerator();
                    IEnumerator<Test.CR> er = r.GetEnumerator();
                    foreach(Test.CR obj in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        test(obj.v.i == eo.Current.v.i);
                        test(obj.v.i == er.Current.v.i);
                    }
                    callback.called();
                }

                public void opAEnSI(Ice.AsyncResult result)
                {
                    Test.En[] i =(Test.En[])result.AsyncState;
                    Test.En[] o;
                    Test.En[] r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opAEnS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opLEnSI(Ice.AsyncResult result)
                {
                    List<Test.En> i =(List<Test.En>)result.AsyncState;
                    List<Test.En> o;
                    List<Test.En> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opLEnS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opKEnSI(Ice.AsyncResult result)
                {
                    LinkedList<Test.En> i =(LinkedList<Test.En>)result.AsyncState;
                    LinkedList<Test.En> o;
                    LinkedList<Test.En> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opKEnS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opQEnSI(Ice.AsyncResult result)
                {
                    Queue<Test.En> i =(Queue<Test.En>)result.AsyncState;
                    Queue<Test.En> o;
                    Queue<Test.En> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opQEnS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opSEnSI(Ice.AsyncResult result)
                {
                    Stack<Test.En> i =(Stack<Test.En>)result.AsyncState;
                    Stack<Test.En> o;
                    Stack<Test.En> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSEnS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opCustomIntSI(Ice.AsyncResult result)
                {
                    Custom<int> i =(Custom<int>)result.AsyncState;
                    Custom<int> o;
                    Custom<int> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opCustomIntS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opCustomCVSI(Ice.AsyncResult result)
                {
                    Custom<Test.CV> i =(Custom<Test.CV>)result.AsyncState;
                    Custom<Test.CV> o;
                    Custom<Test.CV> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opCustomCVS(out o, result);
                    IEnumerator<Test.CV> eo = o.GetEnumerator();
                    IEnumerator<Test.CV> er = r.GetEnumerator();
                    foreach(Test.CV obj in i)
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
                            test(obj.i == eo.Current.i);
                            test(obj.i == er.Current.i);
                        }
                    }
                    callback.called();
                }

                public void opCustomIntSSI(Ice.AsyncResult result)
                {
                    Custom<Custom<int>> i =(Custom<Custom<int>>)result.AsyncState;
                    Custom<Custom<int>> o;
                    Custom<Custom<int>> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opCustomIntSS(out o, result);
                    test(Ice.CollectionComparer.Equals(i, o));
                    test(Ice.CollectionComparer.Equals(i, r));
                    callback.called();
                }

                public void opCustomCVSSI(Ice.AsyncResult result)
                {
                    Custom<Custom<Test.CV>> i =(Custom<Custom<Test.CV>>)result.AsyncState;
                    Custom<Custom<Test.CV>> o;
                    Custom<Custom<Test.CV>> r = Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opCustomCVSS(out o, result);
                    IEnumerator<Custom<Test.CV>> eo = o.GetEnumerator();
                    IEnumerator<Custom<Test.CV>> er = r.GetEnumerator();
                    foreach(Custom<Test.CV> s in i)
                    {
                        eo.MoveNext();
                        er.MoveNext();
                        IEnumerator<Test.CV> io = eo.Current.GetEnumerator();
                        IEnumerator<Test.CV> ir = er.Current.GetEnumerator();
                        foreach(Test.CV obj in s)
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

                public void opSerialSmallCSharpNullI(Ice.AsyncResult result)
                {
                    try
                    {
                        Serialize.Small o;
                        Serialize.Small r =
                            Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSerialSmallCSharp(out o, result);
                        test(o == null);
                        test(r == null);
                        callback.called();
                    }
                    catch(Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }

                public void opSerialSmallCSharpI(Ice.AsyncResult result)
                {
                    try
                    {
                        Serialize.Small o;
                        Serialize.Small r =
                            Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSerialSmallCSharp(out o, result);
                        test(o.i == 99);
                        test(r.i == 99);
                        callback.called();
                    }
                    catch(Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }

                public void opSerialLargeCSharpI(Ice.AsyncResult result)
                {
                    try
                    {
                        Serialize.Large o;
                        Serialize.Large r =
                            Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSerialLargeCSharp(out o, result);
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
                    catch(Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }

                public void opSerialStructCSharpI(Ice.AsyncResult result)
                {
                    try
                    {
                        Serialize.Struct o;
                        Serialize.Struct r =
                            Test.MyClassPrxHelper.uncheckedCast(result.getProxy()).end_opSerialStructCSharp(out o, result);
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
                    catch(Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }

                public virtual void check()
                {
                    callback.check();
                }

                private CallbackBase callback = new CallbackBase();
            }

            static int _length = 100;

            internal static void twowaysAMI(Ice.Communicator communicator, Test.MyClassPrx p)
            {
                {
                    byte[] i = new byte[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(byte)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opAByteS(i, null, cb.opAByteSI, i);
                    cb.check();
                }

                {
                    List<byte> i = new List<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((byte)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLByteS(i, null, cb.opLByteSI, i);
                    cb.check();
                }

                {
                    LinkedList<byte> i = new LinkedList<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((byte)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKByteS(i, null, cb.opKByteSI, i);
                    cb.check();
                }

                {
                    Queue<byte> i = new Queue<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((byte)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQByteS(i, null, cb.opQByteSI, i);
                    cb.check();
                }

                {
                    Stack<byte> i = new Stack<byte>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((byte)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSByteS(i, null, cb.opSByteSI, i);
                    cb.check();
                }

                {
                    bool[] i = new bool[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c % 1 == 1;
                    }

                    Callback cb = new Callback();
                    p.begin_opABoolS(i, null, cb.opABoolSI, i);
                    cb.check();
                }

                {
                    List<bool> i = new List<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c % 1 == 1);
                    }

                    Callback cb = new Callback();
                    p.begin_opLBoolS(i, null, cb.opLBoolSI, i);
                    cb.check();
                }

                {
                    LinkedList<bool> i = new LinkedList<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c % 1 == 1);
                    }

                    Callback cb = new Callback();
                    p.begin_opKBoolS(i, null, cb.opKBoolSI, i);
                    cb.check();
                }

                {
                    Queue<bool> i = new Queue<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c % 1 == 1);
                    }

                    Callback cb = new Callback();
                    p.begin_opQBoolS(i, null, cb.opQBoolSI, i);
                    cb.check();
                }

                {
                    Stack<bool> i = new Stack<bool>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c % 1 == 1);
                    }

                    Callback cb = new Callback();
                    p.begin_opSBoolS(i, null, cb.opSBoolSI, i);
                    cb.check();
                }

                {
                    short[] i = new short[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(short)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opAShortS(i, null, cb.opAShortSI, i);
                    cb.check();
                }

                {
                    List<short> i = new List<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((short)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLShortS(i, null, cb.opLShortSI, i);
                    cb.check();
                }

                {
                    LinkedList<short> i = new LinkedList<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((short)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKShortS(i, null, cb.opKShortSI, i);
                    cb.check();
                }

                {
                    Queue<short> i = new Queue<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((short)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQShortS(i, null, cb.opQShortSI, i);
                    cb.check();
                }

                {
                    Stack<short> i = new Stack<short>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((short)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSShortS(i, null, cb.opSShortSI, i);
                    cb.check();
                }

                {
                    int[] i = new int[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(int)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opAIntS(i, null, cb.opAIntSI, i);
                    cb.check();
                }

                {
                    List<int> i = new List<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((int)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLIntS(i, null, cb.opLIntSI, i);
                    cb.check();
                }

                {
                    LinkedList<int> i = new LinkedList<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((int)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKIntS(i, null, cb.opKIntSI, i);
                    cb.check();
                }

                {
                    Queue<int> i = new Queue<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((int)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQIntS(i, null, cb.opQIntSI, i);
                    cb.check();
                }

                {
                    Stack<int> i = new Stack<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((int)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSIntS(i, null, cb.opSIntSI, i);
                    cb.check();
                }

                {
                    long[] i = new long[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(long)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opALongS(i, null, cb.opALongSI, i);
                    cb.check();
                }

                {
                    List<long> i = new List<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((long)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLLongS(i, null, cb.opLLongSI, i);
                    cb.check();
                }

                {
                    LinkedList<long> i = new LinkedList<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((long)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKLongS(i, null, cb.opKLongSI, i);
                    cb.check();
                }

                {
                    Queue<long> i = new Queue<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((long)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQLongS(i, null, cb.opQLongSI, i);
                    cb.check();
                }

                {
                    Stack<long> i = new Stack<long>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((long)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSLongS(i, null, cb.opSLongSI, i);
                    cb.check();
                }

                {
                    float[] i = new float[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(float)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opAFloatS(i, null, cb.opAFloatSI, i);
                    cb.check();
                }

                {
                    List<float> i = new List<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((float)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLFloatS(i, null, cb.opLFloatSI, i);
                    cb.check();
                }

                {
                    LinkedList<float> i = new LinkedList<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((float)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKFloatS(i, null, cb.opKFloatSI, i);
                    cb.check();
                }

                {
                    Queue<float> i = new Queue<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((float)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQFloatS(i, null, cb.opQFloatSI, i);
                    cb.check();
                }

                {
                    Stack<float> i = new Stack<float>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((float)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSFloatS(i, null, cb.opSFloatSI, i);
                    cb.check();
                }

                {
                    double[] i = new double[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(double)c;
                    }

                    Callback cb = new Callback();
                    p.begin_opADoubleS(i, null, cb.opADoubleSI, i);
                    cb.check();
                }

                {
                    List<double> i = new List<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((double)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opLDoubleS(i, null, cb.opLDoubleSI, i);
                    cb.check();
                }

                {
                    LinkedList<double> i = new LinkedList<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((double)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opKDoubleS(i, null, cb.opKDoubleSI, i);
                    cb.check();
                }

                {
                    Queue<double> i = new Queue<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((double)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opQDoubleS(i, null, cb.opQDoubleSI, i);
                    cb.check();
                }

                {
                    Stack<double> i = new Stack<double>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((double)c);
                    }

                    Callback cb = new Callback();
                    p.begin_opSDoubleS(i, null, cb.opSDoubleSI, i);
                    cb.check();
                }

                {
                    string[] i = new string[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = c.ToString();
                    }

                    Callback cb = new Callback();
                    p.begin_opAStringS(i, null, cb.opAStringSI, i);
                    cb.check();
                }

                {
                    List<string> i = new List<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c.ToString());
                    }

                    Callback cb = new Callback();
                    p.begin_opLStringS(i, null, cb.opLStringSI, i);
                    cb.check();
                }

                {
                    LinkedList<string> i = new LinkedList<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(c.ToString());
                    }

                    Callback cb = new Callback();
                    p.begin_opKStringS(i, null, cb.opKStringSI, i);
                    cb.check();
                }

                {
                    Queue<string> i = new Queue<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(c.ToString());
                    }

                    Callback cb = new Callback();
                    p.begin_opQStringS(i, null, cb.opQStringSI, i);
                    cb.check();
                }

                {
                    Stack<string> i = new Stack<string>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(c.ToString());
                    }

                    Callback cb = new Callback();
                    p.begin_opSStringS(i, null, cb.opSStringSI, i);
                    cb.check();
                }

                {
                    Ice.Value[] i = new Ice.Value[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CV(c);
                    }

                    Callback cb = new Callback();
                    p.begin_opAObjectS(i, null, cb.opAObjectSI, i);
                    cb.check();
                }

                {
                    List<Ice.Value> i = new List<Ice.Value>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opLObjectS(i, null, cb.opLObjectSI, i);
                    cb.check();
                }

                {
                    Ice.ObjectPrx[] i = new Ice.ObjectPrx[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = communicator.stringToProxy(c.ToString());
                    }

                    Callback cb = new Callback();
                    p.begin_opAObjectPrxS(i, null, cb.opAObjectPrxSI, i);
                    cb.check();
                }

                {
                    List<Ice.ObjectPrx> i = new List<Ice.ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(communicator.stringToProxy(c.ToString()));
                    }

                    Callback cb = new Callback();
                    p.begin_opLObjectPrxS(i, null, cb.opLObjectPrxSI, i);
                    cb.check();
                }

                {
                    LinkedList<Ice.ObjectPrx> i = new LinkedList<Ice.ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(communicator.stringToProxy(c.ToString()));
                    }

                    Callback cb = new Callback();
                    p.begin_opKObjectPrxS(i, null, cb.opKObjectPrxSI, i);
                    cb.check();
                }

                {
                    Queue<Ice.ObjectPrx> i = new Queue<Ice.ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(communicator.stringToProxy(c.ToString()));
                    }

                    Callback cb = new Callback();
                    p.begin_opQObjectPrxS(i, null, cb.opQObjectPrxSI, i);
                    cb.check();
                }

                {
                    Stack<Ice.ObjectPrx> i = new Stack<Ice.ObjectPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(communicator.stringToProxy(c.ToString()));
                    }

                    Callback cb = new Callback();
                    p.begin_opSObjectPrxS(i, null, cb.opSObjectPrxSI, i);
                    cb.check();
                }

                {
                    var i = new Test.S[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c].i = c;
                    }

                    Callback cb = new Callback();
                    p.begin_opAStructS(i, null, cb.opAStructSI, i);
                    cb.check();
                }

                {
                    var i = new List<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.S(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opLStructS(i, null, cb.opLStructSI, i);
                    cb.check();
                }

                {
                    var i = new LinkedList<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(new Test.S(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opKStructS(i, null, cb.opKStructSI, i);
                    cb.check();
                }

                {
                    var i = new Queue<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(new Test.S(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opQStructS(i, null, cb.opQStructSI, i);
                    cb.check();
                }

                {
                    var i = new Stack<Test.S>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(new Test.S(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opSStructS(i, null, cb.opSStructSI, i);
                    cb.check();
                }

                {
                    var i = new Test.CV[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CV(c);
                    }

                    Callback cb = new Callback();
                    p.begin_opACVS(i, null, cb.opACVSI, i);
                    cb.check();
                }

                {
                    var i = new List<Test.CV>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opLCVS(i, null, cb.opLCVSI, i);
                    cb.check();
                }

                {
                    Test.IPrx[] i = new Test.IPrx[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString()));
                    }

                    Callback cb = new Callback();
                    p.begin_opAIPrxS(i, null, cb.opIPrxSI, i);
                    cb.check();
                }

                {
                    List<Test.IPrx> i = new List<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    Callback cb = new Callback();
                    p.begin_opLIPrxS(i, null, cb.opLIPrxSI, i);
                    cb.check();
                }

                {
                    LinkedList<Test.IPrx> i = new LinkedList<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    Callback cb = new Callback();
                    p.begin_opKIPrxS(i, null, cb.opKIPrxSI, i);
                    cb.check();
                }

                {
                    Queue<Test.IPrx> i = new Queue<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    Callback cb = new Callback();
                    p.begin_opQIPrxS(i, null, cb.opQIPrxSI, i);
                    cb.check();
                }

                {
                    Stack<Test.IPrx> i = new Stack<Test.IPrx>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push(Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(c.ToString())));
                    }

                    Callback cb = new Callback();
                    p.begin_opSIPrxS(i, null, cb.opSIPrxSI, i);
                    cb.check();
                }

                {
                    var i = new Test.CR[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] = new Test.CR(new Test.CV(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opACRS(i, null, cb.opACRSI, i);
                    cb.check();
                }

                {
                    var i = new List<Test.CR>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CR(new Test.CV(c)));
                    }

                    Callback cb = new Callback();
                    p.begin_opLCRS(i, null, cb.opLCRSI, i);
                    cb.check();
                }

                {
                    var i = new Test.En[_length];
                    for(int c = 0; c < _length; ++c)
                    {
                        i[c] =(Test.En)(c % 3);
                    }

                    Callback cb = new Callback();
                    p.begin_opAEnS(i, null, cb.opAEnSI, i);
                    cb.check();
                }

                {
                    var i = new List<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add((Test.En)(c % 3));
                    }

                    Callback cb = new Callback();
                    p.begin_opLEnS(i, null, cb.opLEnSI, i);
                    cb.check();
                }

                {
                    var i = new LinkedList<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.AddLast((Test.En)(c % 3));
                    }

                    Callback cb = new Callback();
                    p.begin_opKEnS(i, null, cb.opKEnSI, i);
                    cb.check();
                }

                {
                    var i = new Queue<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Enqueue((Test.En)(c % 3));
                    }

                    Callback cb = new Callback();
                    p.begin_opQEnS(i, null, cb.opQEnSI, i);
                    cb.check();
                }

                {
                    var i = new Stack<Test.En>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Push((Test.En)(c % 3));
                    }

                    Callback cb = new Callback();
                    p.begin_opSEnS(i, null, cb.opSEnSI, i);
                    cb.check();
                }

                {
                    Custom<int> i = new Custom<int>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(c);
                    }

                    Callback cb = new Callback();
                    p.begin_opCustomIntS(i, null, cb.opCustomIntSI, i);
                    cb.check();
                }

                {
                    var i = new Custom<Test.CV>();
                    for(int c = 0; c < _length; ++c)
                    {
                        i.Add(new Test.CV(c));
                    }

                    Callback cb = new Callback();
                    p.begin_opCustomCVS(i, null, cb.opCustomCVSI, i);
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

                    Callback cb = new Callback();
                    p.begin_opCustomIntSS(i, null, cb.opCustomIntSSI, i);
                    cb.check();
                }

                {
                    var i = new Custom<Custom<Test.CV>>();
                    for(int c = 0; c < _length; ++c)
                    {
                        var inner = new Custom<Test.CV>();
                        for(int j = 0; j < c; ++j)
                        {
                            inner.Add(new Test.CV(j));
                        }
                        i.Add(inner);
                    }

                    Callback cb = new Callback();
                    p.begin_opCustomCVSS(i, null, cb.opCustomCVSSI, i);
                    cb.check();
                }

                {
                    Serialize.Small i = null;

                    Callback cb = new Callback();
                    p.begin_opSerialSmallCSharp(i, null, cb.opSerialSmallCSharpNullI, i);
                    cb.check();
                }

                {
                    Serialize.Small i = new Serialize.Small();
                    i.i = 99;

                    Callback cb = new Callback();
                    p.begin_opSerialSmallCSharp(i, null, cb.opSerialSmallCSharpI, i);
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

                    Callback cb = new Callback();
                    p.begin_opSerialLargeCSharp(i, null, cb.opSerialLargeCSharpI, i);
                    cb.check();
                }

                {
                    Serialize.Struct i = new Serialize.Struct();
                    i.o = null;
                    i.o2 = i;
                    i.s = null;
                    i.s2 = "Hello";

                    Callback cb = new Callback();
                    p.begin_opSerialStructCSharp(i, null, cb.opSerialStructCSharpI, i);
                    cb.check();
                }
            }
        }
    }
}
