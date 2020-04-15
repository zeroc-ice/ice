//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.seqMapping
{
    public sealed class MyClass : Test.IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public (byte[], byte[]) opAByteS(byte[] i, Current current) => (i, i);

        public (List<byte>, List<byte>) opLByteS(List<byte> i, Current current) => (i, i);

        public (LinkedList<byte>, LinkedList<byte>) opKByteS(LinkedList<byte> i, Current current) => (i, i);

        public (Queue<byte>, Queue<byte>) opQByteS(Queue<byte> i, Current current) => (i, i);

        public (Stack<byte>, Stack<byte>) opSByteS(Stack<byte> i, Current current) => (i, i);

        public (bool[], bool[]) opABoolS(bool[] i, Current current) => (i, i);

        public (List<bool>, List<bool>) opLBoolS(List<bool> i, Current current) => (i, i);

        public (LinkedList<bool>, LinkedList<bool>) opKBoolS(LinkedList<bool> i, Current current) => (i, i);

        public (Queue<bool>, Queue<bool>) opQBoolS(Queue<bool> i, Current current) => (i, i);

        public (Stack<bool>, Stack<bool>) opSBoolS(Stack<bool> i, Current current) => (i, i);

        public (short[], short[]) opAShortS(short[] i, Current current) => (i, i);

        public (List<short>, List<short>) opLShortS(List<short> i, Current current) => (i, i);

        public (LinkedList<short>, LinkedList<short>) opKShortS(LinkedList<short> i, Current current) => (i, i);

        public (Queue<short>, Queue<short>) opQShortS(Queue<short> i, Current current) => (i, i);

        public (Stack<short>, Stack<short>) opSShortS(Stack<short> i, Current current) => (i, i);

        public (int[], int[]) opAIntS(int[] i, Current current) => (i, i);

        public (List<int>, List<int>) opLIntS(List<int> i, Current current) => (i, i);

        public (LinkedList<int>, LinkedList<int>) opKIntS(LinkedList<int> i, Current current) => (i, i);

        public (Queue<int>, Queue<int>) opQIntS(Queue<int> i, Current current) => (i, i);

        public (Stack<int>, Stack<int>) opSIntS(Stack<int> i, Current current) => (i, i);

        public (long[], long[]) opALongS(long[] i, Current current) => (i, i);

        public (List<long>, List<long>) opLLongS(List<long> i, Current current) => (i, i);

        public (LinkedList<long>, LinkedList<long>) opKLongS(LinkedList<long> i, Current current) => (i, i);

        public (Queue<long>, Queue<long>) opQLongS(Queue<long> i, Current current) => (i, i);

        public (Stack<long>, Stack<long>) opSLongS(Stack<long> i, Current current) => (i, i);

        public (float[], float[]) opAFloatS(float[] i, Current current) => (i, i);

        public (List<float>, List<float>) opLFloatS(List<float> i, Current current) => (i, i);

        public (LinkedList<float>, LinkedList<float>) opKFloatS(LinkedList<float> i, Current current) => (i, i);

        public (Queue<float>, Queue<float>) opQFloatS(Queue<float> i, Current current) => (i, i);

        public (Stack<float>, Stack<float>) opSFloatS(Stack<float> i, Current current) => (i, i);

        public (double[], double[]) opADoubleS(double[] i, Current current) => (i, i);

        public (List<double>, List<double>) opLDoubleS(List<double> i, Current current) => (i, i);

        public (LinkedList<double>, LinkedList<double>) opKDoubleS(LinkedList<double> i, Current current) => (i, i);

        public (Queue<double>, Queue<double>) opQDoubleS(Queue<double> i, Current current) => (i, i);

        public (Stack<double>, Stack<double>) opSDoubleS(Stack<double> i, Current current) => (i, i);

        public (string[], string[]) opAStringS(string[] i, Current current) => (i, i);

        public (List<string>, List<string>) opLStringS(List<string> i, Current current) => (i, i);

        public (LinkedList<string>, LinkedList<string>) opKStringS(LinkedList<string> i, Current current) => (i, i);

        public (Queue<string>, Queue<string>) opQStringS(Queue<string> i, Current current) => (i, i);

        public (Stack<string>, Stack<string>) opSStringS(Stack<string> i, Current current) => (i, i);

        public (AnyClass?[], AnyClass?[]) opAObjectS(AnyClass?[] i, Current current) => (i, i);

        public (List<AnyClass?>, List<AnyClass?>) opLObjectS(List<AnyClass?> i, Current current) => (i, i);

        public (IObjectPrx?[], IObjectPrx?[]) opAObjectPrxS(IObjectPrx?[] i, Current current) => (i, i);

        public (List<IObjectPrx?>, List<IObjectPrx?>) opLObjectPrxS(List<IObjectPrx?> i, Current current) => (i, i);

        public (LinkedList<IObjectPrx?>, LinkedList<IObjectPrx?>)
        opKObjectPrxS(LinkedList<IObjectPrx?> i, Current current) => (i, i);

        public (Queue<IObjectPrx?>, Queue<IObjectPrx?>) opQObjectPrxS(Queue<IObjectPrx?> i, Current current) => (i, i);

        public (Stack<IObjectPrx?>, Stack<IObjectPrx?>) opSObjectPrxS(Stack<IObjectPrx?> i, Current current) => (i, i);

        public (Test.S[], Test.S[]) opAStructS(Test.S[] i, Current current) => (i, i);

        public (List<Test.S>, List<Test.S>) opLStructS(List<Test.S> i, Current current) => (i, i);

        public (LinkedList<Test.S>, LinkedList<Test.S>) opKStructS(LinkedList<Test.S> i, Current current) => (i, i);

        public (Queue<Test.S>, Queue<Test.S>) opQStructS(Queue<Test.S> i, Current current) => (i, i);

        public (Stack<Test.S>, Stack<Test.S>) opSStructS(Stack<Test.S> i, Current current) => (i, i);

        public (Test.SD[], Test.SD[]) opAStructSD(Test.SD[] i, Current current) => (i, i);

        public (List<Test.SD>, List<Test.SD>) opLStructSD(List<Test.SD> i, Current current) => (i, i);

        public (LinkedList<Test.SD>, LinkedList<Test.SD>) opKStructSD(LinkedList<Test.SD> i, Current current) => (i, i);

        public (Queue<Test.SD>, Queue<Test.SD>) opQStructSD(Queue<Test.SD> i, Current current) => (i, i);

        public (Stack<Test.SD>, Stack<Test.SD>) opSStructSD(Stack<Test.SD> i, Current current) => (i, i);

        public (Test.CV?[], Test.CV?[]) opACVS(Test.CV?[] i, Current current) => (i, i);

        public (List<Test.CV?>, List<Test.CV?>) opLCVS(List<Test.CV?> i, Current current) => (i, i);

        public (Test.CR?[], Test.CR?[]) opACRS(Test.CR?[] i, Current current) => (i, i);

        public (List<Test.CR?>, List<Test.CR?>) opLCRS(List<Test.CR?> i, Current current) => (i, i);

        public (Test.En[], Test.En[]) opAEnS(Test.En[] i, Current current) => (i, i);

        public (List<Test.En>, List<Test.En>) opLEnS(List<Test.En> i, Current current) => (i, i);

        public (LinkedList<Test.En>, LinkedList<Test.En>) opKEnS(LinkedList<Test.En> i, Current current) => (i, i);

        public (Queue<Test.En>, Queue<Test.En>) opQEnS(Queue<Test.En> i, Current current) => (i, i);

        public (Stack<Test.En>, Stack<Test.En>) opSEnS(Stack<Test.En> i, Current current) => (i, i);

        public (Test.IIPrx?[], Test.IIPrx?[]) opAIPrxS(Test.IIPrx?[] i, Current current) => (i, i);

        public (List<Test.IIPrx?>, List<Test.IIPrx?>) opLIPrxS(List<Test.IIPrx?> i, Current current) => (i, i);

        public (LinkedList<Test.IIPrx?>, LinkedList<Test.IIPrx?>)
        opKIPrxS(LinkedList<Test.IIPrx?> i, Current current) => (i, i);

        public (Queue<Test.IIPrx?>, Queue<Test.IIPrx?>) opQIPrxS(Queue<Test.IIPrx?> i, Current current) => (i, i);

        public (Stack<Test.IIPrx?>, Stack<Test.IIPrx?>) opSIPrxS(Stack<Test.IIPrx?> i, Current current) => (i, i);

        public (Custom<int>, Custom<int>) opCustomIntS(Custom<int> i, Current current) => (i, i);

        public (Custom<Test.CV?>, Custom<Test.CV?>) opCustomCVS(Custom<Test.CV?> i, Current current) => (i, i);

        public (Custom<Custom<int>>, Custom<Custom<int>>) opCustomIntSS(Custom<Custom<int>> i, Current current) => (i, i);

        public (Custom<Custom<Test.CV?>>, Custom<Custom<Test.CV?>>)
        opCustomCVSS(Custom<Custom<Test.CV?>> i, Current current) => (i, i);

        public (Serialize.Small?, Serialize.Small?) opSerialSmallCSharp(Serialize.Small? i, Current current) => (i, i);

        public (Serialize.Large?, Serialize.Large?) opSerialLargeCSharp(Serialize.Large? i, Current current) => (i, i);

        public (Serialize.Struct?, Serialize.Struct?) opSerialStructCSharp(Serialize.Struct? i, Current current) => (i, i);
    }
}
