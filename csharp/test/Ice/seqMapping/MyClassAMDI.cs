//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.seqMapping.AMD
{
    public sealed class MyClass : Test.IMyClass
    {
        public Task? shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return null;
        }

        public Task<(byte[], byte[])> opAByteSAsync(byte[] i, Current current) => Task.FromResult((i, i));

        public Task<(List<byte>, List<byte>)> opLByteSAsync(List<byte> i, Current current) => Task.FromResult((i, i));

        public Task<(LinkedList<byte>, LinkedList<byte>)>
        opKByteSAsync(LinkedList<byte> i, Current current) => Task.FromResult((i, i));

        public Task<(Queue<byte>, Queue<byte>)> opQByteSAsync(Queue<byte> i, Current current) => Task.FromResult((i, i));

        public Task<(Stack<byte>, Stack<byte>)> opSByteSAsync(Stack<byte> i, Current current) => Task.FromResult((i, i));

        public Task<(bool[], bool[])> opABoolSAsync(bool[] i, Current current) => Task.FromResult((i, i));

        public Task<(List<bool>, List<bool>)> opLBoolSAsync(List<bool> i, Current current) => Task.FromResult((i, i));

        public Task<(LinkedList<bool>, LinkedList<bool>)> opKBoolSAsync(LinkedList<bool> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<bool>, Queue<bool>)> opQBoolSAsync(Queue<bool> i, Current current) => Task.FromResult((i, i));

        public Task<(Stack<bool>, Stack<bool>)> opSBoolSAsync(Stack<bool> i, Current current) => Task.FromResult((i, i));

        public Task<(short[], short[])> opAShortSAsync(short[] i, Current current) => Task.FromResult((i, i));

        public Task<(List<short>, List<short>)> opLShortSAsync(List<short> i, Current current) => Task.FromResult((i, i));

        public Task<(LinkedList<short>, LinkedList<short>)>
        opKShortSAsync(LinkedList<short> i, Current current) => Task.FromResult((i, i));

        public Task<(Queue<short>, Queue<short>)> opQShortSAsync(Queue<short> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<short>, Stack<short>)> opSShortSAsync(Stack<short> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(int[], int[])> opAIntSAsync(int[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<int>, List<int>)> opLIntSAsync(List<int> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<int>, LinkedList<int>)> opKIntSAsync(LinkedList<int> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<int>, Queue<int>)> opQIntSAsync(Queue<int> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<int>, Stack<int>)> opSIntSAsync(Stack<int> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(long[], long[])> opALongSAsync(long[] i, Current current) => Task.FromResult((i, i));

        public Task<(List<long>, List<long>)> opLLongSAsync(List<long> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<long>, LinkedList<long>)> opKLongSAsync(LinkedList<long> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<long>, Queue<long>)> opQLongSAsync(Queue<long> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<long>, Stack<long>)> opSLongSAsync(Stack<long> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(float[], float[])> opAFloatSAsync(float[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<float>, List<float>)> opLFloatSAsync(List<float> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<float>, LinkedList<float>)> opKFloatSAsync(LinkedList<float> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<float>, Queue<float>)> opQFloatSAsync(Queue<float> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<float>, Stack<float>)> opSFloatSAsync(Stack<float> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(double[], double[])> opADoubleSAsync(double[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<double>, List<double>)> opLDoubleSAsync(List<double> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<double>, LinkedList<double>)> opKDoubleSAsync(LinkedList<double> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<double>, Queue<double>)> opQDoubleSAsync(Queue<double> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<double>, Stack<double>)> opSDoubleSAsync(Stack<double> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(string[], string[])> opAStringSAsync(string[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<string>, List<string>)> opLStringSAsync(List<string> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<string>, LinkedList<string>)> opKStringSAsync(LinkedList<string> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<string>, Queue<string>)> opQStringSAsync(Queue<string> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<string>, Stack<string>)> opSStringSAsync(Stack<string> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(AnyClass?[], AnyClass?[])> opAObjectSAsync(AnyClass?[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<AnyClass?>, List<AnyClass?>)> opLObjectSAsync(List<AnyClass?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(IObjectPrx?[], IObjectPrx?[])> opAObjectPrxSAsync(IObjectPrx?[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<IObjectPrx?>, List<IObjectPrx?>)> opLObjectPrxSAsync(List<IObjectPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<IObjectPrx?>, LinkedList<IObjectPrx?>)>
        opKObjectPrxSAsync(LinkedList<IObjectPrx?> i, Current current) => Task.FromResult((i, i));

        public Task<(Queue<IObjectPrx?>, Queue<IObjectPrx?>)> opQObjectPrxSAsync(Queue<IObjectPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<IObjectPrx?>, Stack<IObjectPrx?>)> opSObjectPrxSAsync(Stack<IObjectPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.S[], Test.S[])> opAStructSAsync(Test.S[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<Test.S>, List<Test.S>)> opLStructSAsync(List<Test.S> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<Test.S>, LinkedList<Test.S>)> opKStructSAsync(LinkedList<Test.S> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<Test.S>, Queue<Test.S>)> opQStructSAsync(Queue<Test.S> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<Test.S>, Stack<Test.S>)> opSStructSAsync(Stack<Test.S> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.SD[], Test.SD[])> opAStructSDAsync(Test.SD[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<Test.SD>, List<Test.SD>)> opLStructSDAsync(List<Test.SD> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<Test.SD>, LinkedList<Test.SD>)> opKStructSDAsync(LinkedList<Test.SD> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<Test.SD>, Queue<Test.SD>)> opQStructSDAsync(Queue<Test.SD> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<Test.SD>, Stack<Test.SD>)> opSStructSDAsync(Stack<Test.SD> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.CV?[], Test.CV?[])> opACVSAsync(Test.CV?[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<Test.CV?>, List<Test.CV?>)> opLCVSAsync(List<Test.CV?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.IIPrx?[], Test.IIPrx?[])> opAIPrxSAsync(Test.IIPrx?[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<Test.IIPrx?>, List<Test.IIPrx?>)> opLIPrxSAsync(List<Test.IIPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<Test.IIPrx?>, LinkedList<Test.IIPrx?>)>
        opKIPrxSAsync(LinkedList<Test.IIPrx?> i, Current current) => Task.FromResult((i, i));

        public Task<(Queue<Test.IIPrx?>, Queue<Test.IIPrx?>)> opQIPrxSAsync(Queue<Test.IIPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<Test.IIPrx?>, Stack<Test.IIPrx?>)> opSIPrxSAsync(Stack<Test.IIPrx?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.CR?[], Test.CR?[])> opACRSAsync(Test.CR?[] i, Current current) => Task.FromResult((i, i));

        public Task<(List<Test.CR?>, List<Test.CR?>)> opLCRSAsync(List<Test.CR?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Test.En[], Test.En[])> opAEnSAsync(Test.En[] i, Current current) =>
            Task.FromResult((i, i));

        public Task<(List<Test.En>, List<Test.En>)> opLEnSAsync(List<Test.En> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(LinkedList<Test.En>, LinkedList<Test.En>)> opKEnSAsync(LinkedList<Test.En> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Queue<Test.En>, Queue<Test.En>)> opQEnSAsync(Queue<Test.En> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Stack<Test.En>, Stack<Test.En>)> opSEnSAsync(Stack<Test.En> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Custom<int>, Custom<int>)> opCustomIntSAsync(Custom<int> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Custom<Test.CV?>, Custom<Test.CV?>)> opCustomCVSAsync(Custom<Test.CV?> i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Custom<Custom<int>>, Custom<Custom<int>>)>
        opCustomIntSSAsync(Custom<Custom<int>> i, Current current) => Task.FromResult((i, i));

        public Task<(Custom<Custom<Test.CV?>>, Custom<Custom<Test.CV?>>)>
        opCustomCVSSAsync(Custom<Custom<Test.CV?>> i, Current current) => Task.FromResult((i, i));

        public Task<(Serialize.Small, Serialize.Small)> opSerialSmallCSharpAsync(Serialize.Small i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Serialize.Large, Serialize.Large)> opSerialLargeCSharpAsync(Serialize.Large i, Current current) =>
            Task.FromResult((i, i));

        public Task<(Serialize.Struct, Serialize.Struct)> opSerialStructCSharpAsync(Serialize.Struct i, Current current) =>
            Task.FromResult((i, i));
    }
}
