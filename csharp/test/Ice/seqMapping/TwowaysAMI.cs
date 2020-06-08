//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public class TwowaysAMI
    {
        private const int Length = 100;

        internal static void twowaysAMI(Communicator communicator, IMyClassPrx p)
        {
            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                var r = p.opAByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                var r = p.opLByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opKByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opQByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opSByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.opCByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray();
                var r = p.opABoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToList();
                var r = p.opLBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.opKBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.opQBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.opSBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.opCBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var r = p.opAShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var r = p.opLShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opKShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opQShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opSShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.opCShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var r = p.opAIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var r = p.opLIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opKIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opQIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opSIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.opCIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var r = p.opALongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var r = p.opLLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opKLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opQLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opSLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.opCLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var r = p.opAFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var r = p.opLFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opKFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opQFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opSFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.opCFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var r = p.opADoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var r = p.opLDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opKDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opQDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opSDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.opCDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var r = p.opAStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var r = p.opLStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opKStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opQStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opSStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.opCStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToArray();
                var r = p.opAObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToList<AnyClass?>();
                var r = p.opLObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<AnyClass>(Enumerable.Range(0, Length).Select(
                    x => new CV(x) as AnyClass).ToArray());
                var r = p.opCObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.opAObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList<IObjectPrx?>();
                var r = p.opLObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opKObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opQObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opSObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opCObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToArray();
                var r = p.opAStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToList();
                var r = p.opLStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.opKStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.opQStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.opSStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.opCStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToArray();
                var r = p.opACVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>();
                var r = p.opLCVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.opAIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList<IIPrx?>();
                var r = p.opLIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opKIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opQIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opSIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.opCIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToArray();
                var r = p.opACRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToList<CR?>();
                var r = p.opLCRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = new Custom<CR>(Enumerable.Range(0, Length).Select(
                    x => new CR(new CV(x))).ToArray());
                var r = p.opLCRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray();
                var r = p.opAEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList();
                var r = p.opLEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<En>(Enumerable.Range(0, Length).Select(
                    x => (En)(x % 3)).ToArray());
                var r = p.opKEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.opQEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.opSEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.opCEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var r = p.opCustomIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<CV?>(
                    Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>());
                var r = p.opCustomCVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var r = p.opCustomIntSSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<CV?>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<CV?>(Enumerable.Range(0, Length).Select(
                        x => new CV(x)).ToList<CV?>()));
                }
                var r = p.opCustomCVSSAsync(i).Result;
                for (int c = 0; c < Length; ++c)
                {
                    TestHelper.Assert(r.o[c].SequenceEqual(i[c], new CVComparer()));
                    TestHelper.Assert(r.ReturnValue[c].SequenceEqual(i[c], new CVComparer()));
                }
            }

            {
                var i = new Serialize.Small();
                i.i = 99;

                (Serialize.Small ReturnValue, Serialize.Small o) = p.opSerialSmallCSharpAsync(i).Result;
                TestHelper.Assert(o.i == 99);
                TestHelper.Assert(ReturnValue.i == 99);
            }

            {
                var i = new Serialize.Large();
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

                (Serialize.Large ReturnValue, Serialize.Large o) = p.opSerialLargeCSharpAsync(i).Result;
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

                TestHelper.Assert(ReturnValue.d1 == 1.0);
                TestHelper.Assert(ReturnValue.d2 == 2.0);
                TestHelper.Assert(ReturnValue.d3 == 3.0);
                TestHelper.Assert(ReturnValue.d4 == 4.0);
                TestHelper.Assert(ReturnValue.d5 == 5.0);
                TestHelper.Assert(ReturnValue.d6 == 6.0);
                TestHelper.Assert(ReturnValue.d7 == 7.0);
                TestHelper.Assert(ReturnValue.d8 == 8.0);
                TestHelper.Assert(ReturnValue.d9 == 9.0);
                TestHelper.Assert(ReturnValue.d10 == 10.0);
                TestHelper.Assert(ReturnValue.d11 == 11.0);
                TestHelper.Assert(ReturnValue.s1 == Serialize.Large.LargeString);
            }

            {
                var i = new Serialize.Struct();
                i.o = null;
                i.o2 = i;
                i.s = null;
                i.s2 = "Hello";

                (Serialize.Struct ReturnValue, Serialize.Struct o) = p.opSerialStructCSharpAsync(i).Result;
                TestHelper.Assert(o.o == null);
                TestHelper.Assert(o.o2 == o);
                TestHelper.Assert(o.s == null);
                TestHelper.Assert(o.s2 == "Hello");

                TestHelper.Assert(ReturnValue.o == null);
                TestHelper.Assert(ReturnValue.o2 == ReturnValue);
                TestHelper.Assert(ReturnValue.s == null);
                TestHelper.Assert(ReturnValue.s2 == "Hello");
            }
        }
    }

    public class CVComparer : IEqualityComparer<AnyClass?>
    {
        public bool Equals(AnyClass? x, AnyClass? y)
        {
            if (x == y)
            {
                return true;
            }

            if (x == null || y == null)
            {
                return false;
            }
            return (x as CV)!.i == (y as CV)!.i;
        }

        public int GetHashCode(AnyClass? obj) => obj!.GetHashCode();
    }

    public class CRComparer : IEqualityComparer<CR?>
    {
        public bool Equals(CR? x, CR? y)
        {
            if (x == y)
            {
                return true;
            }

            if (x == null || y == null)
            {
                return false;
            }
            return x.v!.i == y.v!.i;
        }

        public int GetHashCode(CR? obj) => obj!.GetHashCode();
    }
}
