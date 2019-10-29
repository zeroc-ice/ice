//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace Ice
{
    namespace seqMapping
    {
        class Twoways
        {
            private static void test(bool b)
            {
                if (!b)
                {
                    throw new System.Exception();
                }
            }

            const int Length = 100;

            internal static void twoways(Communicator communicator, Test.MyClassPrx p)
            {
                {
                    var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                    byte[] o;
                    var r = p.opAByteS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                    List<byte> o;
                    var r = p.opLByteS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    LinkedList<byte> o;
                    var r = p.opKByteS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    Queue<byte> o;
                    var r = p.opQByteS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                    Stack<byte> o;
                    var r = p.opSByteS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray();
                    bool[] o;
                    var r = p.opABoolS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToList();
                    List<bool> o;
                    var r = p.opLBoolS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    LinkedList<bool> o;
                    var r = p.opKBoolS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    Queue<bool> o;
                    var r = p.opQBoolS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                    Stack<bool> o;
                    var r = p.opSBoolS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                    short[] o;
                    var r = p.opAShortS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                    List<short> o;
                    var r = p.opLShortS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    LinkedList<short> o;
                    var r = p.opKShortS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    Queue<short> o;
                    var r = p.opQShortS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                    Stack<short> o;
                    var r = p.opSShortS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).ToArray();
                    int[] o;
                    var r = p.opAIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).ToList();
                    List<int> o;
                    var r = p.opLIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                    LinkedList<int> o;
                    var r = p.opKIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                    Queue<int> o;
                    var r = p.opQIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                    Stack<int> o;
                    var r = p.opSIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                    long[] o;
                    var r = p.opALongS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                    List<long> o;
                    var r = p.opLLongS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    LinkedList<long> o;
                    var r = p.opKLongS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    Queue<long> o;
                    var r = p.opQLongS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                    Stack<long> o;
                    var r = p.opSLongS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                    float[] o;
                    var r = p.opAFloatS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                    List<float> o;
                    var r = p.opLFloatS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    LinkedList<float> o;
                    var r = p.opKFloatS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    Queue<float> o;
                    var r = p.opQFloatS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                    Stack<float> o;
                    var r = p.opSFloatS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                    double[] o;
                    var r = p.opADoubleS(i, out o);
                    test(o.SequenceEqual(i));
                    test(r.SequenceEqual(i));

                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                    List<double> o;
                    var r = p.opLDoubleS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    LinkedList<double> o;
                    var r = p.opKDoubleS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    Queue<double> o;
                    var r = p.opQDoubleS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                    Stack<double> o;
                    var r = p.opSDoubleS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                    string[] o;
                    var r = p.opAStringS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                    List<string> o;
                    var r = p.opLStringS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    LinkedList<string> o;
                    var r = p.opKStringS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    Queue<string> o;
                    var r = p.opQStringS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                    Stack<string> o;
                    var r = p.opSStringS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToArray();
                    Value[] o;
                    var r = p.opAObjectS(i, out o);
                    test(r.SequenceEqual(i, new CVComparer()));
                    test(o.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToList();
                    List<Value> o;
                    var r = p.opLObjectS(i, out o);
                    test(r.SequenceEqual(i, new CVComparer()));
                    test(o.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray();
                    ObjectPrx[] o;
                    var r = p.opAObjectPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToList();
                    List<ObjectPrx> o;
                    var r = p.opLObjectPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<ObjectPrx>(Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray());
                    LinkedList<ObjectPrx> o;
                    var r = p.opKObjectPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<ObjectPrx>(Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray());
                    Queue<ObjectPrx> o;
                    var r = p.opQObjectPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<ObjectPrx>(Enumerable.Range(0, Length).Select(
                        x => communicator.stringToProxy(x.ToString())).ToArray());
                    Stack<ObjectPrx> o;
                    var r = p.opSObjectPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray();
                    Test.S[] o;
                    var r = p.opAStructS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToList();
                    List<Test.S> o;
                    var r = p.opLStructS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    LinkedList<Test.S> o;
                    var r = p.opKStructS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    Queue<Test.S> o;
                    var r = p.opQStructS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                    Stack<Test.S> o;
                    var r = p.opSStructS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToArray();
                    Test.SD[] o;
                    var r = p.opAStructSD(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList();
                    List<Test.SD> o;
                    var r = p.opLStructSD(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                    LinkedList<Test.SD> o;
                    var r = p.opKStructSD(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                    Queue<Test.SD> o;
                    var r = p.opQStructSD(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                    Stack<Test.SD> o;
                    var r = p.opSStructSD(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToArray();
                    Test.CV[] o;
                    var r = p.opACVS(i, out o);
                    test(r.SequenceEqual(i, new CVComparer()));
                    test(o.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList();
                    List<Test.CV> o;
                    var r = p.opLCVS(i, out o);
                    test(r.SequenceEqual(i, new CVComparer()));
                    test(o.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToArray();
                    Test.CR[] o;
                    var r = p.opACRS(i, out o);
                    test(r.SequenceEqual(i, new CRComparer()));
                    test(o.SequenceEqual(i, new CRComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToList();
                    List<Test.CR> o;
                    var r = p.opLCRS(i, out o);
                    test(r.SequenceEqual(i, new CRComparer()));
                    test(o.SequenceEqual(i, new CRComparer()));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray();
                    Test.En[] o;
                    var r = p.opAEnS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList();
                    List<Test.En> o;
                    var r = p.opLEnS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                    LinkedList<Test.En> o;
                    var r = p.opKEnS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                    Queue<Test.En> o;
                    var r = p.opQEnS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                    Stack<Test.En> o;
                    var r = p.opSEnS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToArray();
                    Test.IPrx[] o;
                    var r = p.opAIPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToList();
                    List<Test.IPrx> o;
                    var r = p.opLIPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new LinkedList<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToList());
                    LinkedList<Test.IPrx> o;
                    var r = p.opKIPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Queue<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToList());
                    Queue<Test.IPrx> o;
                    var r = p.opQIPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Stack<Test.IPrx>(Enumerable.Range(0, Length).Select(
                        x => Test.IPrxHelper.uncheckedCast(communicator.stringToProxy(x.ToString()))).ToList());
                    Stack<Test.IPrx> o;
                    var r = p.opSIPrxS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                    Custom<int> o;
                    var r = p.opCustomIntS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList());
                    Custom<Test.CV> o;
                    var r = p.opCustomCVS(i, out o);
                    test(r.SequenceEqual(i, new CVComparer()));
                    test(o.SequenceEqual(i, new CVComparer()));
                }

                {
                    var i = new Custom<Custom<int>>();
                    for (int c = 0; c < Length; ++c)
                    {
                        i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                    }
                    Custom<Custom<int>> o;
                    var r = p.opCustomIntSS(i, out o);
                    test(r.SequenceEqual(i));
                    test(o.SequenceEqual(i));
                }

                {
                    var i = new Custom<Custom<Test.CV>>();
                    for (int c = 0; c < Length; ++c)
                    {
                        i.Add(new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList()));
                    }
                    Custom<Custom<Test.CV>> o;
                    var r = p.opCustomCVSS(i, out o);
                    for (int c = 0; c < Length; ++c)
                    {
                        test(r[c].SequenceEqual(i[c], new CVComparer()));
                        test(o[c].SequenceEqual(i[c], new CVComparer()));
                    }
                }

                {
                    Serialize.Small i = null;
                    Serialize.Small o;
                    Serialize.Small r;

                    r = p.opSerialSmallCSharp(i, out o);

                    test(o == null);
                    test(r == null);
                }

                {
                    Serialize.Small i = new Serialize.Small();
                    i.i = 99;
                    Serialize.Small o;
                    Serialize.Small r;

                    try
                    {
                        r = p.opSerialSmallCSharp(i, out o);

                        test(o.i == 99);
                        test(r.i == 99);
                    }
                    catch (Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
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
                    Serialize.Large o;
                    Serialize.Large r;

                    try
                    {
                        r = p.opSerialLargeCSharp(i, out o);

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
                    }
                    catch (Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }

                {
                    Serialize.Struct i = new Serialize.Struct();
                    i.o = null;
                    i.o2 = i;
                    i.s = null;
                    i.s2 = "Hello";
                    Serialize.Struct o;
                    Serialize.Struct r;

                    try
                    {
                        r = p.opSerialStructCSharp(i, out o);

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
                    }
                    catch (Ice.OperationNotExistException)
                    {
                        // OK, talking to non-C# server.
                    }
                }
            }
        }
    }
}
