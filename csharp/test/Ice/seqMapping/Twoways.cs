//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using Test;

namespace Ice.seqMapping
{
    class Twoways
    {
        const int Length = 100;

        internal static void twoways(Communicator communicator, Test.IMyClassPrx p)
        {
            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                var (r, o) = p.opAByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                var (r, o) = p.opLByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opKByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opQByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var (r, o) = p.opSByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray();
                var (r, o) = p.opABoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToList();
                var (r, o) = p.opLBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opKBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opQBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 1 == 1).ToArray());
                var (r, o) = p.opSBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var (r, o) = p.opAShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var (r, o) = p.opLShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opKShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opQShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.opSShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var (r, o) = p.opAIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var (r, o) = p.opLIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opKIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opQIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.opSIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var (r, o) = p.opALongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var (r, o) = p.opLLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opKLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opQLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.opSLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var (r, o) = p.opAFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var (r, o) = p.opLFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opKFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opQFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.opSFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var (r, o) = p.opADoubleS(i);
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(r.SequenceEqual(i));

            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var (r, o) = p.opLDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opKDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opQDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.opSDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var (r, o) = p.opAStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var (r, o) = p.opLStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opKStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opQStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.opSStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as AnyClass).ToArray();
                var (r, o) = p.opAObjectS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x) as AnyClass).ToList<AnyClass?>();
                var (r, o) = p.opLObjectS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.opAObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList<IObjectPrx?>();
                var (r, o) = p.opLObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opKObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opQObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.opSObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray();
                var (r, o) = p.opAStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToList();
                var (r, o) = p.opLStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opKStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opQStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.S>(Enumerable.Range(0, Length).Select(x => new Test.S(x)).ToArray());
                var (r, o) = p.opSStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToArray();
                var (r, o) = p.opAStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList();
                var (r, o) = p.opLStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opKStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opQStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.SD>(Enumerable.Range(0, Length).Select(x => new Test.SD(x)).ToList());
                var (r, o) = p.opSStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToArray();
                var (r, o) = p.opACVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList<Test.CV?>();
                var (r, o) = p.opLCVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToArray();
                var (r, o) = p.opACRS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new Test.CR(new Test.CV(x))).ToList<Test.CR?>();
                var (r, o) = p.opLCRS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToArray();
                var (r, o) = p.opAEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList();
                var (r, o) = p.opLEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opKEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opQEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.En>(Enumerable.Range(0, Length).Select(x => (Test.En)(x % 3)).ToList());
                var (r, o) = p.opSEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.opAIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList<Test.IIPrx?>();
                var (r, o) = p.opLIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<Test.IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opKIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<Test.IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opQIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<Test.IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => Test.IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.opSIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var (r, o) = p.opCustomIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<Test.CV?>(
                    Enumerable.Range(0, Length).Select(x => new Test.CV(x)).ToList<Test.CV?>());
                var (r, o) = p.opCustomCVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var (r, o) = p.opCustomIntSS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<Test.CV?>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<Test.CV?>(Enumerable.Range(0, Length).Select(
                        x => new Test.CV(x)).ToList<Test.CV?>()));
                }
                var (r, o) = p.opCustomCVSS(i);
                for (int c = 0; c < Length; ++c)
                {
                    TestHelper.Assert(r[c].SequenceEqual(i[c], new CVComparer()));
                    TestHelper.Assert(o[c].SequenceEqual(i[c], new CVComparer()));
                }
            }

            {
                Serialize.Small i = new Serialize.Small();
                i.i = 99;
                Serialize.Small o;
                Serialize.Small r;

                try
                {
                    (r, o) = p.opSerialSmallCSharp(i);

                    TestHelper.Assert(o.i == 99);
                    TestHelper.Assert(r.i == 99);
                }
                catch (OperationNotExistException)
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
                i.d11 = 11.0;
                i.s1 = Serialize.Large.LargeString;
                Serialize.Large o;
                Serialize.Large r;

                try
                {
                    (r, o) = p.opSerialLargeCSharp(i);
                    TestHelper.Assert(o.d1 == 1.0);
                    TestHelper.Assert(o.d2 == 2.0);
                    TestHelper.Assert(o.d3 == 3.0);
                    TestHelper.Assert(o.d4 == 4.0);
                    TestHelper.Assert(o.d5 == 5.0);
                    TestHelper.Assert(o.d6 == 6.0);
                    TestHelper.Assert(o.d7 == 7.0);
                    TestHelper.Assert(o.d8 == 8.0);
                    TestHelper.Assert(o.d9 == 9.0);
                    TestHelper.Assert(o.d10 == 10.0);
                    TestHelper.Assert(o.d11 == 11.0);
                    TestHelper.Assert(o.s1 == Serialize.Large.LargeString);
                    TestHelper.Assert(r.d1 == 1.0);
                    TestHelper.Assert(r.d2 == 2.0);
                    TestHelper.Assert(r.d3 == 3.0);
                    TestHelper.Assert(r.d4 == 4.0);
                    TestHelper.Assert(r.d5 == 5.0);
                    TestHelper.Assert(r.d6 == 6.0);
                    TestHelper.Assert(r.d7 == 7.0);
                    TestHelper.Assert(r.d8 == 8.0);
                    TestHelper.Assert(r.d9 == 9.0);
                    TestHelper.Assert(r.d10 == 10.0);
                    TestHelper.Assert(r.d11 == 11.0);
                    TestHelper.Assert(r.s1 == Serialize.Large.LargeString);
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
                    TestHelper.Assert(o.o == null);
                    TestHelper.Assert(o.o2 != null);
                    TestHelper.Assert(((Serialize.Struct)(o.o2)).o == null);
                    TestHelper.Assert(((Serialize.Struct)(o.o2)).o2 == o.o2);
                    TestHelper.Assert(o.s == null);
                    TestHelper.Assert(o.s2!.Equals("Hello"));
                    TestHelper.Assert(r.o == null);
                    TestHelper.Assert(r.o2 != null);
                    TestHelper.Assert(((Serialize.Struct)(r.o2)).o == null);
                    TestHelper.Assert(((Serialize.Struct)(r.o2)).o2 == r.o2);
                    TestHelper.Assert(r.s == null);
                    TestHelper.Assert(r.s2!.Equals("Hello"));
                }
                catch (OperationNotExistException)
                {
                    // OK, talking to non-C# server.
                }
            }
        }
    }
}
