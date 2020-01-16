//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

namespace Ice.seqMapping
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

        internal static void twoways(Communicator communicator, Test.IMyClassPrx p)
        {
            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                var (r, o) = p.opAByteS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                var (r, o) = p.opLByteS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opKByteS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opQByteS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opSByteS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray();
                var (r, o) = p.opABoolS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToList();
                var (r, o) = p.opLBoolS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opKBoolS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opQBoolS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opSBoolS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var (r, o) = p.opAShortS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var (r, o) = p.opLShortS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opKShortS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opQShortS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opSShortS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var (r, o) = p.opAIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var (r, o) = p.opLIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opKIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opQIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opSIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var (r, o) = p.opALongS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var (r, o) = p.opLLongS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opKLongS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opQLongS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opSLongS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var (r, o) = p.opAFloatS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var (r, o) = p.opLFloatS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opKFloatS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opQFloatS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opSFloatS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var (r, o) = p.opADoubleS(i);
                test(o.SequenceEqual(i));
                test(r.SequenceEqual(i));

            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var (r, o) = p.opLDoubleS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opKDoubleS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opQDoubleS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opSDoubleS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var (r, o) = p.opAStringS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var (r, o) = p.opLStringS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opKStringS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opQStringS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opSStringS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToArray();
                var (r, o) = p.opAObjectS(i);
                test(r.SequenceEqual(i, new CVComparer()));
                test(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as Value).ToList();
                var (r, o) = p.opLObjectS(i);
                test(r.SequenceEqual(i, new CVComparer()));
                test(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.opAObjectPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList();
                var (r, o) = p.opLObjectPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opKObjectPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opQObjectPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opSObjectPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray();
                var (r, o) = p.opAStructS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToList();
                var (r, o) = p.opLStructS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opKStructS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opQStructS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opSStructS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToArray();
                var (r, o) = p.opAStructSD(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList();
                var (r, o) = p.opLStructSD(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opKStructSD(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opQStructSD(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opSStructSD(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToArray();
                var (r, o) = p.opACVS(i);
                test(r.SequenceEqual(i, new CVComparer()));
                test(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList();
                var (r, o) = p.opLCVS(i);
                test(r.SequenceEqual(i, new CVComparer()));
                test(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToArray();
                var (r, o) = p.opACRS(i);
                test(r.SequenceEqual(i, new CRComparer()));
                test(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToList();
                var (r, o) = p.opLCRS(i);
                test(r.SequenceEqual(i, new CRComparer()));
                test(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray();
                var (r, o) = p.opAEnS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList();
                var (r, o) = p.opLEnS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opKEnS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opQEnS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opSEnS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.opAIPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList();
                var (r, o) = p.opLIPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opKIPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opQIPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.IIPrx>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opSIPrxS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var (r, o) = p.opCustomIntS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList());
                var (r, o) = p.opCustomCVS(i);
                test(r.SequenceEqual(i, new CVComparer()));
                test(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var (r, o) = p.opCustomIntSS(i);
                test(r.SequenceEqual(i));
                test(o.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<Test.CV>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<Test.CV>(Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList()));
                }
                var (r, o) = p.opCustomCVSS(i);
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

                (r, o) = p.opSerialSmallCSharp(i);

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
                    (r, o) = p.opSerialSmallCSharp(i);

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
                    (r, o) = p.opSerialLargeCSharp(i);

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
                    (r, o) = p.opSerialStructCSharp(i);

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
