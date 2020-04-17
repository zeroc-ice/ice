//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace Ice.seqMapping
{
    public sealed class MyClass : Test.IMyClass
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

        public (IEnumerable<Test.S>, IEnumerable<Test.S>) opAStructS(Test.S[] i, Current current) => (i, i);

        public (IEnumerable<Test.S>, IEnumerable<Test.S>) opLStructS(List<Test.S> i, Current current) => (i, i);

        public (IEnumerable<Test.S>, IEnumerable<Test.S>) opKStructS(LinkedList<Test.S> i, Current current) => (i, i);

        public (IEnumerable<Test.S>, IEnumerable<Test.S>) opQStructS(Queue<Test.S> i, Current current) => (i, i);

        public (IEnumerable<Test.S>, IEnumerable<Test.S>) opSStructS(Stack<Test.S> i, Current current) => (i, i);

        public (IEnumerable<Test.SD>, IEnumerable<Test.SD>) opAStructSD(Test.SD[] i, Current current) => (i, i);

        public (IEnumerable<Test.SD>, IEnumerable<Test.SD>) opLStructSD(List<Test.SD> i, Current current) => (i, i);

        public (IEnumerable<Test.SD>, IEnumerable<Test.SD>) opKStructSD(LinkedList<Test.SD> i, Current current) => (i, i);

        public (IEnumerable<Test.SD>, IEnumerable<Test.SD>) opQStructSD(Queue<Test.SD> i, Current current) => (i, i);

        public (IEnumerable<Test.SD>, IEnumerable<Test.SD>) opSStructSD(Stack<Test.SD> i, Current current) => (i, i);

        public (IEnumerable<Test.CV?>, IEnumerable<Test.CV?>) opACVS(Test.CV?[] i, Current current) => (i, i);

        public (IEnumerable<Test.CV?>, IEnumerable<Test.CV?>) opLCVS(List<Test.CV?> i, Current current) => (i, i);

        public (IEnumerable<Test.CR?>, IEnumerable<Test.CR?>) opACRS(Test.CR?[] i, Current current) => (i, i);

        public (IEnumerable<Test.CR?>, IEnumerable<Test.CR?>) opLCRS(List<Test.CR?> i, Current current) => (i, i);

        public (IEnumerable<Test.En>, IEnumerable<Test.En>) opAEnS(Test.En[] i, Current current) => (i, i);

        public (IEnumerable<Test.En>, IEnumerable<Test.En>) opLEnS(List<Test.En> i, Current current) => (i, i);

        public (IEnumerable<Test.En>, IEnumerable<Test.En>) opKEnS(LinkedList<Test.En> i, Current current) => (i, i);

        public (IEnumerable<Test.En>, IEnumerable<Test.En>) opQEnS(Queue<Test.En> i, Current current) => (i, i);

        public (IEnumerable<Test.En>, IEnumerable<Test.En>) opSEnS(Stack<Test.En> i, Current current) => (i, i);

        public (IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>) opAIPrxS(Test.IIPrx?[] i, Current current) => (i, i);

        public (IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>) opLIPrxS(List<Test.IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>) opKIPrxS(LinkedList<Test.IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>) opQIPrxS(Queue<Test.IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>) opSIPrxS(Stack<Test.IIPrx?> i, Current current) => (i, i);

        public (IEnumerable<int>, IEnumerable<int>) opCustomIntS(Custom<int> i, Current current) => (i, i);

        public (IEnumerable<Test.CV?>, IEnumerable<Test.CV?>) opCustomCVS(Custom<Test.CV?> i, Current current) => (i, i);

        public (IEnumerable<Custom<int>>, IEnumerable<Custom<int>>) opCustomIntSS(Custom<Custom<int>> i, Current current) => (i, i);

        public (IEnumerable<Custom<Test.CV?>>, IEnumerable<Custom<Test.CV?>>) opCustomCVSS(Custom<Custom<Test.CV?>> i, Current current) => (i, i);

        public (Serialize.Small, Serialize.Small) opSerialSmallCSharp(Serialize.Small i, Current current) => (i, i);

        public (Serialize.Large, Serialize.Large) opSerialLargeCSharp(Serialize.Large i, Current current) => (i, i);

        public (Serialize.Struct, Serialize.Struct) opSerialStructCSharp(Serialize.Struct i, Current current) => (i, i);
    }
}
