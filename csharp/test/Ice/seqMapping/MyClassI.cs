// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice.Test.SeqMapping
{
    public sealed class MyClass : IMyClass
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) OpAByteS(
            byte[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) OpLByteS(
            List<byte> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) OpKByteS(
            LinkedList<byte> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) OpQByteS(
            Queue<byte> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) OpSByteS(
            Stack<byte> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) OpCByteS(
            Custom<byte> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) OpABoolS(
            bool[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) OpLBoolS(
            List<bool> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) OpKBoolS(
            LinkedList<bool> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) OpQBoolS(
            Queue<bool> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) OpSBoolS(
            Stack<bool> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) OpCBoolS(
            Custom<bool> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<short>, ReadOnlyMemory<short>) OpAShortS(
            short[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) OpLShortS(
            List<short> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) OpKShortS(
            LinkedList<short> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) OpQShortS(
            Queue<short> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) OpSShortS(
            Stack<short> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) OpCShortS(
            Custom<short> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<int>, ReadOnlyMemory<int>) OpAIntS(
            int[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpLIntS(
            List<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpKIntS(
            LinkedList<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpQIntS(
            Queue<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpSIntS(
            Stack<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpCIntS(
            Custom<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<long>, ReadOnlyMemory<long>) OpALongS(
            long[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) OpLLongS(
            List<long> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) OpKLongS(
            LinkedList<long> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) OpQLongS(
            Queue<long> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) OpSLongS(
            Stack<long> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) OpCLongS(
            Custom<long> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<float>, ReadOnlyMemory<float>) OpAFloatS(
            float[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) OpLFloatS(
            List<float> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) OpKFloatS(
            LinkedList<float> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) OpQFloatS(
            Queue<float> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) OpSFloatS(
            Stack<float> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) OpCFloatS(
            Custom<float> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<double>) OpADoubleS(
            double[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) OpLDoubleS(
            List<double> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) OpKDoubleS(
            LinkedList<double> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) OpQDoubleS(
            Queue<double> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) OpSDoubleS(
            Stack<double> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) OpCDoubleS(
            Custom<double> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpAStringS(
            string[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpLStringS(
            List<string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpKStringS(
            LinkedList<string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpQStringS(
            Queue<string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpSStringS(
            Stack<string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) OpCStringS(
            Custom<string> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) OpAObjectS(
            AnyClass?[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) OpLObjectS(
            List<AnyClass?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) OpCObjectS(
            Custom<AnyClass?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpAObjectPrxS(
            IObjectPrx?[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpLObjectPrxS(
            List<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpKObjectPrxS(
            LinkedList<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpQObjectPrxS(
            Queue<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpSObjectPrxS(
            Stack<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) OpCObjectPrxS(
            Custom<IObjectPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpAStructS(
            S[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpLStructS(
            List<S> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpKStructS(
            LinkedList<S> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpQStructS(
            Queue<S> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpSStructS(
            Stack<S> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) OpCStructS(
            Custom<S> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpAStructSD(
            SD[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpLStructSD(
            List<SD> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpKStructSD(
            LinkedList<SD> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpQStructSD(
            Queue<SD> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpSStructSD(
            Stack<SD> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) OpCStructSD(
            Custom<SD> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) OpACVS(
            CV?[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) OpLCVS(
            List<CV?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpAIPrxS(
            IIPrx?[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpLIPrxS(
            List<IIPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpKIPrxS(
            LinkedList<IIPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpQIPrxS(
            Queue<IIPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpSIPrxS(
            Stack<IIPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) OpCIPrxS(
            Custom<IIPrx?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CR?>, IEnumerable<CR?>) OpACRS(
            CR?[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CR?>, IEnumerable<CR?>) OpLCRS(
            List<CR?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CR?>, IEnumerable<CR?>) OpCCRS(
            Custom<CR?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpAEnS(
            En[] i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpLEnS(
            List<En> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpKEnS(
            LinkedList<En> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpQEnS(
            Queue<En> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpSEnS(
            Stack<En> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) OpCEnS(
            Custom<En> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) OpCustomIntS(
            Custom<int> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) OpCustomCVS(
            Custom<CV?> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IEnumerable<int>>, IEnumerable<IEnumerable<int>>) OpCustomIntSS(
            Custom<Custom<int>> i,
            Current current,
            CancellationToken cancel) => (i, i);

        public (IEnumerable<IEnumerable<CV?>>, IEnumerable<IEnumerable<CV?>>) OpCustomCVSS(
            Custom<Custom<CV?>> i,
            Current current,
            CancellationToken cancel) => (i, i);
    }
}
