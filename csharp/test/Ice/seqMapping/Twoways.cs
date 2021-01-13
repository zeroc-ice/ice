// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using ZeroC.Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public static class Twoways
    {
        private const int Length = 100;

        internal static void Run(Communicator communicator, IMyClassPrx p)
        {
            {
                byte[]? i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                (byte[] r, byte[] o) = p.OpAByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new List<byte>(Enumerable.Range(0, Length).Select(x => (byte)x));
                (List<byte> r, List<byte> o) = p.OpLByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x));
                (LinkedList<byte> r, LinkedList<byte> o) = p.OpKByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x));
                (Queue<byte> r, Queue<byte> o) = p.OpQByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x));
                (Stack<byte> r, Stack<byte> o) = p.OpSByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<byte>(Enumerable.Range(0, Length).Select(x => (byte)x));
                (Custom<byte> r, Custom<byte> o) = p.OpCByteS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                bool[]? i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray();
                var (r, o) = p.OpABoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToList();
                var (r, o) = p.OpLBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var (r, o) = p.OpKBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var (r, o) = p.OpQBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var (r, o) = p.OpSBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var (r, o) = p.OpCBoolS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var (r, o) = p.OpAShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var (r, o) = p.OpLShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.OpKShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.OpQShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.OpSShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var (r, o) = p.OpCShortS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var (r, o) = p.OpAIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var (r, o) = p.OpLIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.OpKIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.OpQIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.OpSIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToArray());
                var (r, o) = p.OpCIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var (r, o) = p.OpALongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var (r, o) = p.OpLLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.OpKLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.OpQLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.OpSLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var (r, o) = p.OpCLongS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var (r, o) = p.OpAFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var (r, o) = p.OpLFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.OpKFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.OpQFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.OpSFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var (r, o) = p.OpCFloatS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var (r, o) = p.OpADoubleS(i);
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(r.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var (r, o) = p.OpLDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.OpKDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.OpQDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.OpSDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var (r, o) = p.OpCDoubleS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var (r, o) = p.OpAStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var (r, o) = p.OpLStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.OpKStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.OpQStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.OpSStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var (r, o) = p.OpCStringS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToArray();
                var (r, o) = p.OpAObjectS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToList<AnyClass?>();
                var (r, o) = p.OpLObjectS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<AnyClass>(Enumerable.Range(0, Length).Select(
                    x => new CV(x) as AnyClass).ToArray());
                var (r, o) = p.OpCObjectS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.OpAObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList<IObjectPrx?>();
                var (r, o) = p.OpLObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.OpKObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.OpQObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.OpSObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.OpCObjectPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToArray();
                var (r, o) = p.OpAStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToList();
                var (r, o) = p.OpLStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var (r, o) = p.OpKStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var (r, o) = p.OpQStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var (r, o) = p.OpSStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var (r, o) = p.OpCStructS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new SD(x)).ToArray();
                var (r, o) = p.OpAStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new SD(x)).ToList();
                var (r, o) = p.OpLStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<SD>(Enumerable.Range(0, Length).Select(x => new SD(x)).ToList());
                var (r, o) = p.OpKStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<SD>(Enumerable.Range(0, Length).Select(x => new SD(x)).ToList());
                var (r, o) = p.OpQStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<SD>(Enumerable.Range(0, Length).Select(x => new SD(x)).ToList());
                var (r, o) = p.OpSStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<SD>(Enumerable.Range(0, Length).Select(x => new SD(x)).ToList());
                var (r, o) = p.OpCStructSD(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToArray();
                var (r, o) = p.OpACVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>();
                var (r, o) = p.OpLCVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var (r, o) = p.OpAIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList<IIPrx?>();
                var (r, o) = p.OpLIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.OpKIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.OpQIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList());
                var (r, o) = p.OpSIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var (r, o) = p.OpCIPrxS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToArray();
                var (r, o) = p.OpACRS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToList<CR?>();
                var (r, o) = p.OpLCRS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = new Custom<CR>(Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToArray());
                var (r, o) = p.OpCCRS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray();
                var (r, o) = p.OpAEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList();
                var (r, o) = p.OpLEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new LinkedList<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList());
                var (r, o) = p.OpKEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Queue<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList());
                var (r, o) = p.OpQEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Stack<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList());
                var (r, o) = p.OpSEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList());
                var (r, o) = p.OpCEnS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var (r, o) = p.OpCustomIntS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<CV?>(
                    Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>());
                var (r, o) = p.OpCustomCVS(i);
                TestHelper.Assert(r.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var (r, o) = p.OpCustomIntSS(i);
                TestHelper.Assert(r.SequenceEqual(i));
                TestHelper.Assert(o.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<CV?>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<CV?>(Enumerable.Range(0, Length).Select(
                        x => new CV(x)).ToList<CV?>()));
                }
                var (r, o) = p.OpCustomCVSS(i);
                for (int c = 0; c < Length; ++c)
                {
                    TestHelper.Assert(r[c].SequenceEqual(i[c], new CVComparer()));
                    TestHelper.Assert(o[c].SequenceEqual(i[c], new CVComparer()));
                }
            }
        }
    }
}
