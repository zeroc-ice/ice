// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.SeqMapping
{
    public sealed class AsyncMyClass : IAsyncMyClass
    {
        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> OpAByteSAsync(
            byte[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> OpLByteSAsync(
            List<byte> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> OpKByteSAsync(
            LinkedList<byte> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> OpQByteSAsync(
            Queue<byte> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> OpSByteSAsync(
            Stack<byte> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> OpCByteSAsync(
            Custom<byte> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> OpABoolSAsync(
            bool[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> OpLBoolSAsync(
            List<bool> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> OpKBoolSAsync(
            LinkedList<bool> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> OpQBoolSAsync(
            Queue<bool> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> OpSBoolSAsync(
            Stack<bool> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> OpCBoolSAsync(
            Custom<bool> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<short>, ReadOnlyMemory<short>)> OpAShortSAsync(
            short[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> OpLShortSAsync(
            List<short> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> OpKShortSAsync(
            LinkedList<short> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> OpQShortSAsync(
            Queue<short> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> OpSShortSAsync(
            Stack<short> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> OpCShortSAsync(
            Custom<short> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<int>, ReadOnlyMemory<int>)> OpAIntSAsync(
            int[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpLIntSAsync(
            List<int> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpKIntSAsync(
            LinkedList<int> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpQIntSAsync(
            Queue<int> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpSIntSAsync(
            Stack<int> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpCIntSAsync(
            Custom<int> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<long>)> OpALongSAsync(
            long[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> OpLLongSAsync(
            List<long> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> OpKLongSAsync(
            LinkedList<long> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> OpQLongSAsync(
            Queue<long> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> OpSLongSAsync(
            Stack<long> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> OpCLongSAsync(
            Custom<long> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<float>, ReadOnlyMemory<float>)> OpAFloatSAsync(
            float[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> OpLFloatSAsync(
            List<float> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> OpKFloatSAsync(
            LinkedList<float> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> OpQFloatSAsync(
            Queue<float> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> OpSFloatSAsync(
            Stack<float> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> OpCFloatSAsync(
            Custom<float> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<double>)> OpADoubleSAsync(
            double[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> OpLDoubleSAsync(
            List<double> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> OpKDoubleSAsync(
            LinkedList<double> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> OpQDoubleSAsync(
            Queue<double> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> OpSDoubleSAsync(
            Stack<double> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> OpCDoubleSAsync(
            Custom<double> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpAStringSAsync(
            string[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpLStringSAsync(
            List<string> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpKStringSAsync(
            LinkedList<string> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpQStringSAsync(
            Queue<string> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpSStringSAsync(
            Stack<string> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> OpCStringSAsync(
            Custom<string> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<AnyClass?>, IEnumerable<AnyClass?>)> OpAObjectSAsync(
            AnyClass?[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<AnyClass?>, IEnumerable<AnyClass?>)> OpLObjectSAsync(
            List<AnyClass?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<AnyClass?>, IEnumerable<AnyClass?>)> OpCObjectSAsync(
            Custom<AnyClass?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpAObjectPrxSAsync(
            IObjectPrx?[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpLObjectPrxSAsync(
            List<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpKObjectPrxSAsync(
            LinkedList<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpQObjectPrxSAsync(
            Queue<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpSObjectPrxSAsync(
            Stack<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> OpCObjectPrxSAsync(
            Custom<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpAStructSAsync(
            S[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpLStructSAsync(
            List<S> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpKStructSAsync(
            LinkedList<S> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpQStructSAsync(
            Queue<S> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpSStructSAsync(
            Stack<S> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> OpCStructSAsync(
            Custom<S> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpAStructSDAsync(
            SD[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpLStructSDAsync(
            List<SD> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpKStructSDAsync(
            LinkedList<SD> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpQStructSDAsync(
            Queue<SD> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpSStructSDAsync(
            Stack<SD> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> OpCStructSDAsync(
            Custom<SD> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> OpACVSAsync(
            CV?[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> OpLCVSAsync(
            List<CV?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpAIPrxSAsync(
            IIPrx?[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpLIPrxSAsync(
            List<IIPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpKIPrxSAsync(
            LinkedList<IIPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpQIPrxSAsync(
            Queue<IIPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpSIPrxSAsync(
            Stack<IIPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> OpCIPrxSAsync(
            Custom<IIPrx?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<CR?>, IEnumerable<CR?>)> OpACRSAsync(
            CR?[] i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<CR?>, IEnumerable<CR?>)> OpLCRSAsync(
            List<CR?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<CR?>, IEnumerable<CR?>)> OpCCRSAsync(
            Custom<CR?> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpAEnSAsync(
            En[] i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpLEnSAsync(
            List<En> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpKEnSAsync(
            LinkedList<En> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpQEnSAsync(
            Queue<En> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpSEnSAsync(
            Stack<En> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> OpCEnSAsync(
            Custom<En> i,
            Current current,
            CancellationToken cancel) =>
            new((i, i));

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> OpCustomIntSAsync(
            Custom<int> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> OpCustomCVSAsync(
            Custom<CV?> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IEnumerable<int>>, IEnumerable<IEnumerable<int>>)> OpCustomIntSSAsync(
            Custom<Custom<int>> i,
            Current current,
            CancellationToken cancel) => new((i, i));

        public ValueTask<(IEnumerable<IEnumerable<CV?>>, IEnumerable<IEnumerable<CV?>>)> OpCustomCVSSAsync(
            Custom<Custom<CV?>> i,
            Current current,
            CancellationToken cancel) => new((i, i));
    }
}
