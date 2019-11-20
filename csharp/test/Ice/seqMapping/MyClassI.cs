//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace seqMapping
    {
        public sealed class MyClassI : Test.MyClass
        {
            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public byte[] opAByteS(byte[] i, out byte[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<byte> opLByteS(List<byte> i, out List<byte> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<byte> opKByteS(LinkedList<byte> i, out LinkedList<byte> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<byte> opQByteS(Queue<byte> i, out Queue<byte> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<byte> opSByteS(Stack<byte> i, out Stack<byte> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public bool[] opABoolS(bool[] i, out bool[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<bool> opLBoolS(List<bool> i, out List<bool> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<bool> opKBoolS(LinkedList<bool> i, out LinkedList<bool> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<bool> opQBoolS(Queue<bool> i, out Queue<bool> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<bool> opSBoolS(Stack<bool> i, out Stack<bool> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public short[] opAShortS(short[] i, out short[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<short> opLShortS(List<short> i, out List<short> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<short> opKShortS(LinkedList<short> i, out LinkedList<short> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<short> opQShortS(Queue<short> i, out Queue<short> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<short> opSShortS(Stack<short> i, out Stack<short> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public int[] opAIntS(int[] i, out int[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<int> opLIntS(List<int> i, out List<int> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<int> opKIntS(LinkedList<int> i, out LinkedList<int> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<int> opQIntS(Queue<int> i, out Queue<int> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<int> opSIntS(Stack<int> i, out Stack<int> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public long[] opALongS(long[] i, out long[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<long> opLLongS(List<long> i, out List<long> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<long> opKLongS(LinkedList<long> i, out LinkedList<long> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<long> opQLongS(Queue<long> i, out Queue<long> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<long> opSLongS(Stack<long> i, out Stack<long> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public float[] opAFloatS(float[] i, out float[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<float> opLFloatS(List<float> i, out List<float> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<float> opKFloatS(LinkedList<float> i, out LinkedList<float> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<float> opQFloatS(Queue<float> i, out Queue<float> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<float> opSFloatS(Stack<float> i, out Stack<float> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public double[] opADoubleS(double[] i, out double[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<double> opLDoubleS(List<double> i, out List<double> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<double> opKDoubleS(LinkedList<double> i, out LinkedList<double> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<double> opQDoubleS(Queue<double> i, out Queue<double> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<double> opSDoubleS(Stack<double> i, out Stack<double> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public string[] opAStringS(string[] i, out string[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<string> opLStringS(List<string> i, out List<string> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<string> opKStringS(LinkedList<string> i, out LinkedList<string> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<string> opQStringS(Queue<string> i, out Queue<string> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<string> opSStringS(Stack<string> i, out Stack<string> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Ice.Value[] opAObjectS(Ice.Value[] i, out Ice.Value[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Ice.Value> opLObjectS(List<Ice.Value> i, out List<Ice.Value> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Ice.ObjectPrx[] opAObjectPrxS(Ice.ObjectPrx[] i, out Ice.ObjectPrx[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Ice.ObjectPrx> opLObjectPrxS(List<Ice.ObjectPrx> i,
                                                                      out List<Ice.ObjectPrx> o,
                                                                      Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<Ice.ObjectPrx> opKObjectPrxS(LinkedList<Ice.ObjectPrx> i,
                                                                            out LinkedList<Ice.ObjectPrx> o,
                                                                            Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<Ice.ObjectPrx> opQObjectPrxS(Queue<Ice.ObjectPrx> i,
                                                                       out Queue<Ice.ObjectPrx> o,
                                                                       Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<Ice.ObjectPrx> opSObjectPrxS(Stack<Ice.ObjectPrx> i,
                                                                       out Stack<Ice.ObjectPrx> o,
                                                                       Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.S[] opAStructS(Test.S[] i, out Test.S[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.S> opLStructS(List<Test.S> i, out List<Test.S> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<Test.S> opKStructS(LinkedList<Test.S> i, out LinkedList<Test.S> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<Test.S> opQStructS(Queue<Test.S> i, out Queue<Test.S> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<Test.S> opSStructS(Stack<Test.S> i, out Stack<Test.S> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.SD[] opAStructSD(Test.SD[] i, out Test.SD[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.SD> opLStructSD(List<Test.SD> i, out List<Test.SD> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<Test.SD> opKStructSD(LinkedList<Test.SD> i, out LinkedList<Test.SD> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<Test.SD> opQStructSD(Queue<Test.SD> i, out Queue<Test.SD> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<Test.SD> opSStructSD(Stack<Test.SD> i, out Stack<Test.SD> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.CV[] opACVS(Test.CV[] i, out Test.CV[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.CV> opLCVS(List<Test.CV> i, out List<Test.CV> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.CR[] opACRS(Test.CR[] i, out Test.CR[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.CR> opLCRS(List<Test.CR> i, out List<Test.CR> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.En[] opAEnS(Test.En[] i, out Test.En[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.En> opLEnS(List<Test.En> i, out List<Test.En> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<Test.En> opKEnS(LinkedList<Test.En> i, out LinkedList<Test.En> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<Test.En> opQEnS(Queue<Test.En> i, out Queue<Test.En> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<Test.En> opSEnS(Stack<Test.En> i, out Stack<Test.En> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Test.IPrx[] opAIPrxS(Test.IPrx[] i, out Test.IPrx[] o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public List<Test.IPrx> opLIPrxS(List<Test.IPrx> i, out List<Test.IPrx> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public LinkedList<Test.IPrx> opKIPrxS(LinkedList<Test.IPrx> i, out LinkedList<Test.IPrx> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Queue<Test.IPrx> opQIPrxS(Queue<Test.IPrx> i, out Queue<Test.IPrx> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Stack<Test.IPrx> opSIPrxS(Stack<Test.IPrx> i, out Stack<Test.IPrx> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Custom<int> opCustomIntS(Custom<int> i, out Custom<int> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Custom<Test.CV> opCustomCVS(Custom<Test.CV> i, out Custom<Test.CV> o, Ice.Current current)
            {
                o = i;
                return i;
            }

            public Custom<Custom<int>> opCustomIntSS(Custom<Custom<int>> i, out Custom<Custom<int>> o,
                                                              Ice.Current current)
            {
                o = i;
                return i;
            }

            public Custom<Custom<Test.CV>> opCustomCVSS(Custom<Custom<Test.CV>> i, out Custom<Custom<Test.CV>> o,
                                                                 Ice.Current current)
            {
                o = i;
                return i;
            }

            public Serialize.Small opSerialSmallCSharp(Serialize.Small i, out Serialize.Small o,
                                                                Ice.Current current)
            {
                o = i;
                return i;
            }

            public Serialize.Large opSerialLargeCSharp(Serialize.Large i, out Serialize.Large o,
                                                                Ice.Current current)
            {
                o = i;
                return i;
            }

            public Serialize.Struct opSerialStructCSharp(Serialize.Struct i, out Serialize.Struct o,
                                                                  Ice.Current current)
            {
                o = i;
                return i;
            }
        }
    }
}
