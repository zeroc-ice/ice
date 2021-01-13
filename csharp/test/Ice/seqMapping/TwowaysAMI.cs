// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Linq;
using ZeroC.Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public static class TwowaysAMI
    {
        private const int Length = 100;

        internal static void Run(Communicator communicator, IMyClassPrx p)
        {
            {
                byte[] i = Enumerable.Range(0, Length).Select(x => (byte)x).ToArray();
                (byte[] ReturnValue, byte[] o) = p.OpAByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (byte)x).ToList();
                (List<byte> ReturnValue, List<byte> o) = p.OpLByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                (LinkedList<byte> ReturnValue, LinkedList<byte> o) = p.OpKByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                (Queue<byte> ReturnValue, Queue<byte> o) = p.OpQByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                (Stack<byte> ReturnValue, Stack<byte> o) = p.OpSByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<byte>(Enumerable.Range(0, Length).Select(x => (byte)x).ToArray());
                (Custom<byte> ReturnValue, Custom<byte> o) = p.OpCByteSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                bool[] i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray();
                (bool[] ReturnValue, bool[] o) = p.OpABoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToList();
                (List<bool> ReturnValue, List<bool> o) = p.OpLBoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                (LinkedList<bool> ReturnValue, LinkedList<bool> o) = p.OpKBoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                (Queue<bool> ReturnValue, Queue<bool> o) = p.OpQBoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                (Stack<bool> ReturnValue, Stack<bool> o) = p.OpSBoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<bool>(Enumerable.Range(0, Length).Select(x => x % 2 == 1).ToArray());
                (Custom<bool> ReturnValue, Custom<bool> o) = p.OpCBoolSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                short[]? i = Enumerable.Range(0, Length).Select(x => (short)x).ToArray();
                (short[] ReturnValue, short[] o) = p.OpAShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (short)x).ToList();
                (List<short> ReturnValue, List<short> o) = p.OpLShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                (LinkedList<short> ReturnValue, LinkedList<short> o) = p.OpKShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                (Queue<short> ReturnValue, Queue<short> o) = p.OpQShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                (Stack<short> ReturnValue, Stack<short> o) = p.OpSShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<short>(Enumerable.Range(0, Length).Select(x => (short)x).ToArray());
                (Custom<short> ReturnValue, Custom<short> o) = p.OpCShortSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                int[] i = Enumerable.Range(0, Length).ToArray();
                (int[] ReturnValue, int[] o) = p.OpAIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).ToList();
                (List<int> ReturnValue, List<int> o) = p.OpLIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<int>(Enumerable.Range(0, Length).ToArray());
                (LinkedList<int> ReturnValue, LinkedList<int> o) = p.OpKIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<int>(Enumerable.Range(0, Length).ToArray());
                (Queue<int> ReturnValue, Queue<int> o) = p.OpQIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<int>(Enumerable.Range(0, Length).ToArray());
                (Stack<int> ReturnValue, Stack<int> o) = p.OpSIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToArray());
                (Custom<int> ReturnValue, Custom<int> o) = p.OpCIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                long[]? i = Enumerable.Range(0, Length).Select(x => (long)x).ToArray();
                (long[] ReturnValue, long[] o) = p.OpALongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (long)x).ToList();
                (List<long> ReturnValue, List<long> o) = p.OpLLongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                (LinkedList<long> ReturnValue, LinkedList<long> o) = p.OpKLongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                (Queue<long> ReturnValue, Queue<long> o) = p.OpQLongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                (Stack<long> ReturnValue, Stack<long> o) = p.OpSLongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<long>(Enumerable.Range(0, Length).Select(x => (long)x).ToArray());
                (Custom<long> ReturnValue, Custom<long> o) = p.OpCLongSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                float[] i = Enumerable.Range(0, Length).Select(x => (float)x).ToArray();
                (float[] ReturnValue, float[] o) = p.OpAFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (float)x).ToList();
                (List<float> ReturnValue, List<float> o) = p.OpLFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                (LinkedList<float> ReturnValue, LinkedList<float> o) = p.OpKFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                (Queue<float> ReturnValue, Queue<float> o) = p.OpQFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                (Stack<float> ReturnValue, Stack<float> o) = p.OpSFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<float>(Enumerable.Range(0, Length).Select(x => (float)x).ToArray());
                (Custom<float> ReturnValue, Custom<float> o) = p.OpCFloatSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                double[] i = Enumerable.Range(0, Length).Select(x => (double)x).ToArray();
                (double[] ReturnValue, double[] o) = p.OpADoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (double)x).ToList();
                (List<double> ReturnValue, List<double> o) = p.OpLDoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                (LinkedList<double> ReturnValue, LinkedList<double> o) = p.OpKDoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                (Queue<double> ReturnValue, Queue<double> o) = p.OpQDoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                (Stack<double> ReturnValue, Stack<double> o) = p.OpSDoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<double>(Enumerable.Range(0, Length).Select(x => (double)x).ToArray());
                (Custom<double> ReturnValue, Custom<double> o) = p.OpCDoubleSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                string[] i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray();
                (string[] ReturnValue, string[] o) = p.OpAStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => x.ToString()).ToList();
                (List<string> ReturnValue, List<string> o) = p.OpLStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                (LinkedList<string> ReturnValue, LinkedList<string> o) = p.OpKStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                (Queue<string> ReturnValue, Queue<string> o) = p.OpQStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                (Stack<string> ReturnValue, Stack<string> o) = p.OpSStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<string>(Enumerable.Range(0, Length).Select(x => x.ToString()).ToArray());
                (Custom<string> ReturnValue, Custom<string> o) = p.OpCStringSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                AnyClass[] i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToArray();
                (AnyClass?[] ReturnValue, AnyClass?[] o) = p.OpAObjectSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x) as AnyClass).ToList<AnyClass?>();
                (List<AnyClass?> ReturnValue, List<AnyClass?> o) = p.OpLObjectSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<AnyClass>(Enumerable.Range(0, Length).Select(
                    x => new CV(x) as AnyClass).ToArray());
                (Custom<AnyClass?> ReturnValue, Custom<AnyClass?> o) = p.OpCObjectSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                IObjectPrx[]? i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray();
                (IObjectPrx?[] ReturnValue, IObjectPrx?[] o) = p.OpAObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToList<IObjectPrx?>();
                (List<IObjectPrx?> ReturnValue, List<IObjectPrx?> o) = p.OpLObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                (LinkedList<IObjectPrx?> ReturnValue, LinkedList<IObjectPrx?> o) = p.OpKObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                (Queue<IObjectPrx?> ReturnValue, Queue<IObjectPrx?> o) = p.OpQObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                (Stack<IObjectPrx?> ReturnValue, Stack<IObjectPrx?> o) = p.OpSObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IObjectPrx?>(Enumerable.Range(0, Length).Select(
                    x => IObjectPrx.Parse(x.ToString(), communicator)).ToArray());
                (Custom<IObjectPrx?> ReturnValue, Custom<IObjectPrx?> o) = p.OpCObjectPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                S[]? i = Enumerable.Range(0, Length).Select(x => new S(x)).ToArray();
                (S[] ReturnValue, S[] o) = p.OpAStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new S(x)).ToList();
                (List<S> ReturnValue, List<S> o) = p.OpLStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                (LinkedList<S> ReturnValue, LinkedList<S> o) = p.OpKStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                (Queue<S> ReturnValue, Queue<S> o) = p.OpQStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                (Stack<S> ReturnValue, Stack<S> o) = p.OpSStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<S>(Enumerable.Range(0, Length).Select(x => new S(x)).ToArray());
                (Custom<S> ReturnValue, Custom<S> o) = p.OpCStructSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                CV[] i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToArray();
                (CV?[] ReturnValue, CV?[] o) = p.OpACVSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>();
                (List<CV?> ReturnValue, List<CV?> o) = p.OpLCVSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                IIPrx[] i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray();
                (IIPrx?[] ReturnValue, IIPrx?[] o) = p.OpAIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToList<IIPrx?>();
                (List<IIPrx?> ReturnValue, List<IIPrx?> o) = p.OpLIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                (LinkedList<IIPrx?> ReturnValue, LinkedList<IIPrx?> o) = p.OpKIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                (Queue<IIPrx?> ReturnValue, Queue<IIPrx?> o) = p.OpQIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                (Stack<IIPrx?> ReturnValue, Stack<IIPrx?> o) = p.OpSIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<IIPrx?>(Enumerable.Range(0, Length).Select(
                    x => IIPrx.Parse(x.ToString(), communicator)).ToArray());
                (Custom<IIPrx?> ReturnValue, Custom<IIPrx?> o) = p.OpCIPrxSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                CR[]? i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToArray();
                (CR?[] ReturnValue, CR?[] o) = p.OpACRSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => new CR(new CV(x))).ToList<CR?>();
                (List<CR?> ReturnValue, List<CR?> o) = p.OpLCRSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                var i = new Custom<CR>(Enumerable.Range(0, Length).Select(
                    x => new CR(new CV(x))).ToArray());
                (List<CR?> ReturnValue, List<CR?> o) = p.OpLCRSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CRComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CRComparer()));
            }

            {
                En[] i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray();
                (En[] ReturnValue, En[] o) = p.OpAEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToList();
                (List<En> ReturnValue, List<En> o) = p.OpLEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new LinkedList<En>(Enumerable.Range(0, Length).Select(
                    x => (En)(x % 3)).ToArray());
                (LinkedList<En> ReturnValue, LinkedList<En> o) = p.OpKEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Queue<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                (Queue<En> ReturnValue, Queue<En> o) = p.OpQEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Stack<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                (Stack<En> ReturnValue, Stack<En> o) = p.OpSEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<En>(Enumerable.Range(0, Length).Select(x => (En)(x % 3)).ToArray());
                (Custom<En> ReturnValue, Custom<En> o) = p.OpCEnSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<int>(Enumerable.Range(0, Length).ToList());
                (Custom<int> ReturnValue, Custom<int> o) = p.OpCustomIntSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<CV?>(
                    Enumerable.Range(0, Length).Select(x => new CV(x)).ToList<CV?>());
                (Custom<CV?> ReturnValue, Custom<CV?> o) = p.OpCustomCVSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i, new CVComparer()));
                TestHelper.Assert(ReturnValue.SequenceEqual(i, new CVComparer()));
            }

            {
                var i = new Custom<Custom<int>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<int>(Enumerable.Range(0, Length).ToList()));
                }
                (Custom<Custom<int>> ReturnValue, Custom<Custom<int>> o) = p.OpCustomIntSSAsync(i).Result;
                TestHelper.Assert(o.SequenceEqual(i));
                TestHelper.Assert(ReturnValue.SequenceEqual(i));
            }

            {
                var i = new Custom<Custom<CV?>>();
                for (int c = 0; c < Length; ++c)
                {
                    i.Add(new Custom<CV?>(Enumerable.Range(0, Length).Select(
                        x => new CV(x)).ToList<CV?>()));
                }
                (Custom<Custom<CV?>> ReturnValue, Custom<Custom<CV?>> o) = p.OpCustomCVSSAsync(i).Result;
                for (int c = 0; c < Length; ++c)
                {
                    TestHelper.Assert(o[c].SequenceEqual(i[c], new CVComparer()));
                    TestHelper.Assert(ReturnValue[c].SequenceEqual(i[c], new CVComparer()));
                }
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
