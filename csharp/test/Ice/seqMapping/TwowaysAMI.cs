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
                var r = p.OpAByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                var r = p.OpLByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.OpKByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.OpQByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.OpSByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                var r = p.OpCByteSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray();
                var r = p.OpABoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToList();
                var r = p.OpLBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.OpKBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.OpQBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.OpSBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                var r = p.OpCBoolSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                var r = p.OpAShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                var r = p.OpLShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.OpKShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.OpQShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.OpSShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                var r = p.OpCShortSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToArray();
                var r = p.OpAIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                var r = p.OpLIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.OpKIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.OpQIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.OpSIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToArray());
                var r = p.OpCIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                var r = p.OpALongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                var r = p.OpLLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.OpKLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.OpQLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.OpSLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                var r = p.OpCLongSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                var r = p.OpAFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                var r = p.OpLFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.OpKFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.OpQFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.OpSFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                var r = p.OpCFloatSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                var r = p.OpADoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                var r = p.OpLDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.OpKDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.OpQDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.OpSDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                var r = p.OpCDoubleSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                var r = p.OpAStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                var r = p.OpLStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.OpKStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.OpQStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.OpSStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                var r = p.OpCStringSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToArray();
                var r = p.OpAObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToList<AnyClass?>();
                var r = p.OpLObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<AnyClass>(Enumerable.Range(0, Length).Select(
                    x => new CV(x) as AnyClass).ToArray());
                var r = p.OpCObjectSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.OpAObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList<IObjectPrx?>();
                var r = p.OpLObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpKObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpQObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpSObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpCObjectPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToArray();
                var r = p.OpAStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToList();
                var r = p.OpLStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.OpKStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.OpQStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.OpSStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                var r = p.OpCStructSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToArray();
                var r = p.OpACVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>();
                var r = p.OpLCVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray();
                var r = p.OpAIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList<IIPrx?>();
                var r = p.OpLIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpKIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpQIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpSIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                var r = p.OpCIPrxSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToArray();
                var r = p.OpACRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToList<CR?>();
                var r = p.OpLCRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = new Custom<CR>(Enumerable.Range(0, Length).Select(
                    x => new CR(new CV(x))).ToArray());
                var r = p.OpLCRSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray();
                var r = p.OpAEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList();
                var r = p.OpLEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<En>(Enumerable.Range(0, Length).Select(
                    x => (En)(x % 3)).ToArray());
                var r = p.OpKEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.OpQEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.OpSEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                var r = p.OpCEnSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                var r = p.OpCustomIntSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<CV?>(
                    Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>());
                var r = p.OpCustomCVSAsync(i).Result;
                TestHelper.Assert(r.o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(r.ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                var r = p.OpCustomIntSSAsync(i).Result;
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
                var r = p.OpCustomCVSSAsync(i).Result;
                for (int c = 0; c < Length; ++c)
                {
                    TestHelper.Assert(r.o[c].SequenceEqual(i[c], new CVComparer()));
                    TestHelper.Assert(r.ReturnValue[c].SequenceEqual(i[c], new CVComparer()));
                }
            }

            {
                var i = new Serialize.Small();
                i.i = 99;

                (Serialize.Small ReturnValue, Serialize.Small o) = p.OpSerialSmallCSharpAsync(i).Result;
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

                (Serialize.Large ReturnValue, Serialize.Large o) = p.OpSerialLargeCSharpAsync(i).Result;
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

                (Serialize.Struct ReturnValue, Serialize.Struct o) = p.OpSerialStructCSharpAsync(i).Result;
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
            return (x as CV)!.I == (y as CV)!.I;
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
            return x.V!.I == y.V!.I;
        }

        public int GetHashCode(CR? obj) => obj!.GetHashCode();
    }
}
