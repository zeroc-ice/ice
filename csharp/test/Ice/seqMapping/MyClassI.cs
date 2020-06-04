//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.SeqMapping
{
    public sealed class MyClass : IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public (ReadOnlyMemory<byte>, ReadOnlyMemory<byte>) opAByteS(byte[] i, Current current) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) opLByteS(List<byte> i, Current current) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) opKByteS(LinkedList<byte> i, Current current) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) opQByteS(Queue<byte> i, Current current) => (i, i);

        public (IEnumerable<byte>, IEnumerable<byte>) opSByteS(Stack<byte> i, Current current) => (i, i);

        public (ReadOnlyMemory<bool>, ReadOnlyMemory<bool>) opABoolS(bool[] i, Current current) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) opLBoolS(List<bool> i, Current current) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) opKBoolS(LinkedList<bool> i, Current current) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) opQBoolS(Queue<bool> i, Current current) => (i, i);

        public (IEnumerable<bool>, IEnumerable<bool>) opSBoolS(Stack<bool> i, Current current) => (i, i);

        public (ReadOnlyMemory<short>, ReadOnlyMemory<short>) opAShortS(short[] i, Current current) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) opLShortS(List<short> i, Current current) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) opKShortS(LinkedList<short> i, Current current) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) opQShortS(Queue<short> i, Current current) => (i, i);

        public (IEnumerable<short>, IEnumerable<short>) opSShortS(Stack<short> i, Current current) => (i, i);

        public (ReadOnlyMemory<int>, ReadOnlyMemory<int>) opAIntS(int[] i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opLIntS(List<int> i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opKIntS(LinkedList<int> i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opQIntS(Queue<int> i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opSIntS(Stack<int> i, Current current) => (i, i);

        public (ReadOnlyMemory<long>, ReadOnlyMemory<long>) opALongS(long[] i, Current current) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) opLLongS(List<long> i, Current current) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) opKLongS(LinkedList<long> i, Current current) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) opQLongS(Queue<long> i, Current current) => (i, i);

        public (IEnumerable<long>, IEnumerable<long>) opSLongS(Stack<long> i, Current current) => (i, i);

        public (ReadOnlyMemory<float>, ReadOnlyMemory<float>) opAFloatS(float[] i, Current current) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) opLFloatS(List<float> i, Current current) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) opKFloatS(LinkedList<float> i, Current current) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) opQFloatS(Queue<float> i, Current current) => (i, i);

        public (IEnumerable<float>, IEnumerable<float>) opSFloatS(Stack<float> i, Current current) => (i, i);

        public (ReadOnlyMemory<double>, ReadOnlyMemory<double>) opADoubleS(double[] i, Current current) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) opLDoubleS(List<double> i, Current current) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) opKDoubleS(LinkedList<double> i, Current current) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) opQDoubleS(Queue<double> i, Current current) => (i, i);

        public (IEnumerable<double>, IEnumerable<double>) opSDoubleS(Stack<double> i, Current current) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) opAStringS(string[] i, Current current) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) opLStringS(List<string> i, Current current) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) opKStringS(LinkedList<string> i, Current current) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) opQStringS(Queue<string> i, Current current) => (i, i);

        public (IEnumerable<string>, IEnumerable<string>) opSStringS(Stack<string> i, Current current) => (i, i);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) opAObjectS(AnyClass?[] i, Current current) => (i, i);

        public (IEnumerable<AnyClass?>, IEnumerable<AnyClass?>) opLObjectS(List<AnyClass?> i, Current current) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) opAObjectPrxS(IObjectPrx?[] i, Current current) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) opLObjectPrxS(List<IObjectPrx?> i, Current current) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) opKObjectPrxS(LinkedList<IObjectPrx?> i, Current current) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) opQObjectPrxS(Queue<IObjectPrx?> i, Current current) => (i, i);

        public (IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>) opSObjectPrxS(Stack<IObjectPrx?> i, Current current) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) opAStructS(S[] i, Current current) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) opLStructS(List<S> i, Current current) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) opKStructS(LinkedList<S> i, Current current) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) opQStructS(Queue<S> i, Current current) => (i, i);

        public (IEnumerable<S>, IEnumerable<S>) opSStructS(Stack<S> i, Current current) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) opAStructSD(SD[] i, Current current) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) opLStructSD(List<SD> i, Current current) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) opKStructSD(LinkedList<SD> i, Current current) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) opQStructSD(Queue<SD> i, Current current) => (i, i);

        public (IEnumerable<SD>, IEnumerable<SD>) opSStructSD(Stack<SD> i, Current current) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) opACVS(CV?[] i, Current current) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) opLCVS(List<CV?> i, Current current) => (i, i);

        public (IEnumerable<CR?>, IEnumerable<CR?>) opACRS(CR?[] i, Current current) => (i, i);

        public (IEnumerable<CR?>, IEnumerable<CR?>) opLCRS(List<CR?> i, Current current) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) opAEnS(En[] i, Current current) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) opLEnS(List<En> i, Current current) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) opKEnS(LinkedList<En> i, Current current) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) opQEnS(Queue<En> i, Current current) => (i, i);

        public (IEnumerable<En>, IEnumerable<En>) opSEnS(Stack<En> i, Current current) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) opAIPrxS(IIPrx?[] i, Current current) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) opLIPrxS(List<IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) opKIPrxS(LinkedList<IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) opQIPrxS(Queue<IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<IIPrx?>, IEnumerable<IIPrx?>) opSIPrxS(Stack<IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opCustomIntS(Custom<int> i, Current current) => (i, i);

        public (IEnumerable<CV?>, IEnumerable<CV?>) opCustomCVS(Custom<CV?> i, Current current) => (i, i);

        public (IEnumerable<Custom<int>>, IEnumerable<Custom<int>>) opCustomIntSS(Custom<Custom<int>> i, Current current) => (i, i);

        public (IEnumerable<Custom<CV?>>, IEnumerable<Custom<CV?>>) opCustomCVSS(Custom<Custom<CV?>> i, Current current) => (i, i);

        public (Serialize.Small, Serialize.Small) opSerialSmallCSharp(Serialize.Small i, Current current) => (i, i);

        public (Serialize.Large, Serialize.Large) opSerialLargeCSharp(Serialize.Large i, Current current) => (i, i);

        public (Serialize.Struct, Serialize.Struct) opSerialStructCSharp(Serialize.Struct i, Current current) => (i, i);
    }
}
