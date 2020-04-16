//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.seqMapping.AMD
{
    public sealed class MyClass : Test.IMyClass
    {
        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<(byte[], byte[])> opAByteSAsync(byte[] i, Current current) => FromResult((i, i));

        public ValueTask<(List<byte>, List<byte>)> opLByteSAsync(List<byte> i, Current current) => FromResult((i, i));

        public ValueTask<(LinkedList<byte>, LinkedList<byte>)>
        opKByteSAsync(LinkedList<byte> i, Current current) => FromResult((i, i));

        public ValueTask<(Queue<byte>, Queue<byte>)> opQByteSAsync(Queue<byte> i, Current current) => FromResult((i, i));

        public ValueTask<(Stack<byte>, Stack<byte>)> opSByteSAsync(Stack<byte> i, Current current) => FromResult((i, i));

        public ValueTask<(bool[], bool[])> opABoolSAsync(bool[] i, Current current) => FromResult((i, i));

        public ValueTask<(List<bool>, List<bool>)> opLBoolSAsync(List<bool> i, Current current) => FromResult((i, i));

        public ValueTask<(LinkedList<bool>, LinkedList<bool>)> opKBoolSAsync(LinkedList<bool> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<bool>, Queue<bool>)> opQBoolSAsync(Queue<bool> i, Current current) => FromResult((i, i));

        public ValueTask<(Stack<bool>, Stack<bool>)> opSBoolSAsync(Stack<bool> i, Current current) => FromResult((i, i));

        public ValueTask<(short[], short[])> opAShortSAsync(short[] i, Current current) => FromResult((i, i));

        public ValueTask<(List<short>, List<short>)> opLShortSAsync(List<short> i, Current current) => FromResult((i, i));

        public ValueTask<(LinkedList<short>, LinkedList<short>)>
        opKShortSAsync(LinkedList<short> i, Current current) => FromResult((i, i));

        public ValueTask<(Queue<short>, Queue<short>)> opQShortSAsync(Queue<short> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<short>, Stack<short>)> opSShortSAsync(Stack<short> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(int[], int[])> opAIntSAsync(int[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<int>, List<int>)> opLIntSAsync(List<int> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<int>, LinkedList<int>)> opKIntSAsync(LinkedList<int> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<int>, Queue<int>)> opQIntSAsync(Queue<int> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<int>, Stack<int>)> opSIntSAsync(Stack<int> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(long[], long[])> opALongSAsync(long[] i, Current current) => FromResult((i, i));

        public ValueTask<(List<long>, List<long>)> opLLongSAsync(List<long> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<long>, LinkedList<long>)> opKLongSAsync(LinkedList<long> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<long>, Queue<long>)> opQLongSAsync(Queue<long> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<long>, Stack<long>)> opSLongSAsync(Stack<long> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(float[], float[])> opAFloatSAsync(float[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<float>, List<float>)> opLFloatSAsync(List<float> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<float>, LinkedList<float>)> opKFloatSAsync(LinkedList<float> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<float>, Queue<float>)> opQFloatSAsync(Queue<float> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<float>, Stack<float>)> opSFloatSAsync(Stack<float> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(double[], double[])> opADoubleSAsync(double[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<double>, List<double>)> opLDoubleSAsync(List<double> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<double>, LinkedList<double>)> opKDoubleSAsync(LinkedList<double> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<double>, Queue<double>)> opQDoubleSAsync(Queue<double> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<double>, Stack<double>)> opSDoubleSAsync(Stack<double> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(string[], string[])> opAStringSAsync(string[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<string>, List<string>)> opLStringSAsync(List<string> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<string>, LinkedList<string>)> opKStringSAsync(LinkedList<string> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<string>, Queue<string>)> opQStringSAsync(Queue<string> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<string>, Stack<string>)> opSStringSAsync(Stack<string> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(AnyClass?[], AnyClass?[])> opAObjectSAsync(AnyClass?[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<AnyClass?>, List<AnyClass?>)> opLObjectSAsync(List<AnyClass?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(IObjectPrx?[], IObjectPrx?[])> opAObjectPrxSAsync(IObjectPrx?[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<IObjectPrx?>, List<IObjectPrx?>)> opLObjectPrxSAsync(List<IObjectPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<IObjectPrx?>, LinkedList<IObjectPrx?>)>
        opKObjectPrxSAsync(LinkedList<IObjectPrx?> i, Current current) => FromResult((i, i));

        public ValueTask<(Queue<IObjectPrx?>, Queue<IObjectPrx?>)> opQObjectPrxSAsync(Queue<IObjectPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<IObjectPrx?>, Stack<IObjectPrx?>)> opSObjectPrxSAsync(Stack<IObjectPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.S[], Test.S[])> opAStructSAsync(Test.S[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<Test.S>, List<Test.S>)> opLStructSAsync(List<Test.S> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<Test.S>, LinkedList<Test.S>)> opKStructSAsync(LinkedList<Test.S> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<Test.S>, Queue<Test.S>)> opQStructSAsync(Queue<Test.S> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<Test.S>, Stack<Test.S>)> opSStructSAsync(Stack<Test.S> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.SD[], Test.SD[])> opAStructSDAsync(Test.SD[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<Test.SD>, List<Test.SD>)> opLStructSDAsync(List<Test.SD> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<Test.SD>, LinkedList<Test.SD>)> opKStructSDAsync(LinkedList<Test.SD> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<Test.SD>, Queue<Test.SD>)> opQStructSDAsync(Queue<Test.SD> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<Test.SD>, Stack<Test.SD>)> opSStructSDAsync(Stack<Test.SD> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.CV?[], Test.CV?[])> opACVSAsync(Test.CV?[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<Test.CV?>, List<Test.CV?>)> opLCVSAsync(List<Test.CV?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.IIPrx?[], Test.IIPrx?[])> opAIPrxSAsync(Test.IIPrx?[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<Test.IIPrx?>, List<Test.IIPrx?>)> opLIPrxSAsync(List<Test.IIPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<Test.IIPrx?>, LinkedList<Test.IIPrx?>)>
        opKIPrxSAsync(LinkedList<Test.IIPrx?> i, Current current) => FromResult((i, i));

        public ValueTask<(Queue<Test.IIPrx?>, Queue<Test.IIPrx?>)> opQIPrxSAsync(Queue<Test.IIPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<Test.IIPrx?>, Stack<Test.IIPrx?>)> opSIPrxSAsync(Stack<Test.IIPrx?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.CR?[], Test.CR?[])> opACRSAsync(Test.CR?[] i, Current current) => FromResult((i, i));

        public ValueTask<(List<Test.CR?>, List<Test.CR?>)> opLCRSAsync(List<Test.CR?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Test.En[], Test.En[])> opAEnSAsync(Test.En[] i, Current current) =>
            FromResult((i, i));

        public ValueTask<(List<Test.En>, List<Test.En>)> opLEnSAsync(List<Test.En> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(LinkedList<Test.En>, LinkedList<Test.En>)> opKEnSAsync(LinkedList<Test.En> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Queue<Test.En>, Queue<Test.En>)> opQEnSAsync(Queue<Test.En> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Stack<Test.En>, Stack<Test.En>)> opSEnSAsync(Stack<Test.En> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Custom<int>, Custom<int>)> opCustomIntSAsync(Custom<int> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Custom<Test.CV?>, Custom<Test.CV?>)> opCustomCVSAsync(Custom<Test.CV?> i, Current current) =>
            FromResult((i, i));

        public ValueTask<(Custom<Custom<int>>, Custom<Custom<int>>)>
        opCustomIntSSAsync(Custom<Custom<int>> i, Current current) => FromResult((i, i));

        public ValueTask<(Custom<Custom<Test.CV?>>, Custom<Custom<Test.CV?>>)>
        opCustomCVSSAsync(Custom<Custom<Test.CV?>> i, Current current) => FromResult((i, i));

        public ValueTask<(Serialize.Small?, Serialize.Small?)> opSerialSmallCSharpAsync(
            Serialize.Small? i, Current current) => FromResult((i, i));

        public ValueTask<(Serialize.Large?, Serialize.Large?)> opSerialLargeCSharpAsync(
            Serialize.Large? i, Current current) => FromResult((i, i));

        public ValueTask<(Serialize.Struct?, Serialize.Struct?)> opSerialStructCSharpAsync(
            Serialize.Struct? i, Current current) => FromResult((i, i));

        internal static ValueTask<T> FromResult<T>(T result) => new ValueTask<T>(result);
    }
}
