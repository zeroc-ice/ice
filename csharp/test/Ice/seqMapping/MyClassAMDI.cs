//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace Ice.seqMapping.AMD
{
    public sealed class MyClass : Test.IMyClass
    {
        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<(ReadOnlyMemory<byte>, ReadOnlyMemory<byte>)> opAByteSAsync(byte[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> opLByteSAsync(List<byte> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> opKByteSAsync(LinkedList<byte> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> opQByteSAsync(Queue<byte> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<byte>, IEnumerable<byte>)> opSByteSAsync(Stack<byte> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<bool>, ReadOnlyMemory<bool>)> opABoolSAsync(bool[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> opLBoolSAsync(List<bool> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> opKBoolSAsync(LinkedList<bool> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> opQBoolSAsync(Queue<bool> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<bool>, IEnumerable<bool>)> opSBoolSAsync(Stack<bool> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<short>, ReadOnlyMemory<short>)> opAShortSAsync(short[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> opLShortSAsync(List<short> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> opKShortSAsync(LinkedList<short> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> opQShortSAsync(Queue<short> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<short>, IEnumerable<short>)> opSShortSAsync(Stack<short> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<int>, ReadOnlyMemory<int>)> opAIntSAsync(int[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opLIntSAsync(List<int> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opKIntSAsync(LinkedList<int> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opQIntSAsync(Queue<int> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opSIntSAsync(Stack<int> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<long>, ReadOnlyMemory<long>)> opALongSAsync(long[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> opLLongSAsync(List<long> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> opKLongSAsync(LinkedList<long> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> opQLongSAsync(Queue<long> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<long>, IEnumerable<long>)> opSLongSAsync(Stack<long> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<float>, ReadOnlyMemory<float>)> opAFloatSAsync(float[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> opLFloatSAsync(List<float> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> opKFloatSAsync(LinkedList<float> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> opQFloatSAsync(Queue<float> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<float>, IEnumerable<float>)> opSFloatSAsync(Stack<float> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(ReadOnlyMemory<double>, ReadOnlyMemory<double>)> opADoubleSAsync(double[] i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> opLDoubleSAsync(List<double> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> opKDoubleSAsync(LinkedList<double> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> opQDoubleSAsync(Queue<double> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<double>, IEnumerable<double>)> opSDoubleSAsync(Stack<double> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> opAStringSAsync(string[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> opLStringSAsync(List<string> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> opKStringSAsync(LinkedList<string> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> opQStringSAsync(Queue<string> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<string>, IEnumerable<string>)> opSStringSAsync(Stack<string> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<AnyClass?>, IEnumerable<AnyClass?>)> opAObjectSAsync(AnyClass?[] i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<AnyClass?>, IEnumerable<AnyClass?>)> opLObjectSAsync(List<AnyClass?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> opAObjectPrxSAsync(IObjectPrx?[] i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> opLObjectPrxSAsync(List<IObjectPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> opKObjectPrxSAsync(
            LinkedList<IObjectPrx?> i, Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> opQObjectPrxSAsync(Queue<IObjectPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IObjectPrx?>, IEnumerable<IObjectPrx?>)> opSObjectPrxSAsync(Stack<IObjectPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.S>, IEnumerable<Test.S>)> opAStructSAsync(Test.S[] i, Current current) =>
            ToReturnValue(i as IEnumerable<Test.S>);

        public ValueTask<(IEnumerable<Test.S>, IEnumerable<Test.S>)> opLStructSAsync(List<Test.S> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.S>, IEnumerable<Test.S>)> opKStructSAsync(LinkedList<Test.S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.S>, IEnumerable<Test.S>)> opQStructSAsync(Queue<Test.S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.S>, IEnumerable<Test.S>)> opSStructSAsync(Stack<Test.S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.SD>, IEnumerable<Test.SD>)> opAStructSDAsync(Test.SD[] i, Current current) =>
            ToReturnValue(i as IEnumerable<Test.SD>);

        public ValueTask<(IEnumerable<Test.SD>, IEnumerable<Test.SD>)> opLStructSDAsync(List<Test.SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.SD>, IEnumerable<Test.SD>)> opKStructSDAsync(LinkedList<Test.SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.SD>, IEnumerable<Test.SD>)> opQStructSDAsync(Queue<Test.SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.SD>, IEnumerable<Test.SD>)> opSStructSDAsync(Stack<Test.SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.CV?>, IEnumerable<Test.CV?>)> opACVSAsync(Test.CV?[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.CV?>, IEnumerable<Test.CV?>)> opLCVSAsync(List<Test.CV?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>)> opAIPrxSAsync(Test.IIPrx?[] i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>)> opLIPrxSAsync(List<Test.IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>)> opKIPrxSAsync(LinkedList<Test.IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>)> opQIPrxSAsync(Queue<Test.IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.IIPrx?>, IEnumerable<Test.IIPrx?>)> opSIPrxSAsync(Stack<Test.IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.CR?>, IEnumerable<Test.CR?>)> opACRSAsync(Test.CR?[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.CR?>, IEnumerable<Test.CR?>)> opLCRSAsync(List<Test.CR?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.En>, IEnumerable<Test.En>)> opAEnSAsync(Test.En[] i, Current current) =>
            ToReturnValue(i as IEnumerable<Test.En>);

        public ValueTask<(IEnumerable<Test.En>, IEnumerable<Test.En>)> opLEnSAsync(List<Test.En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.En>, IEnumerable<Test.En>)> opKEnSAsync(LinkedList<Test.En> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.En>, IEnumerable<Test.En>)> opQEnSAsync(Queue<Test.En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.En>, IEnumerable<Test.En>)> opSEnSAsync(Stack<Test.En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opCustomIntSAsync(Custom<int> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<Test.CV?>, IEnumerable<Test.CV?>)> opCustomCVSAsync(Custom<Test.CV?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Custom<int>>, IEnumerable<Custom<int>>)> opCustomIntSSAsync(Custom<Custom<int>> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<Custom<Test.CV?>>, IEnumerable<Custom<Test.CV?>>)> opCustomCVSSAsync(
            Custom<Custom<Test.CV?>> i, Current current) => ToReturnValue(i);

        public ValueTask<(Serialize.Small, Serialize.Small)> opSerialSmallCSharpAsync(Serialize.Small i,
            Current current) => new ValueTask<(Serialize.Small, Serialize.Small)>((i, i));

        public ValueTask<(Serialize.Large, Serialize.Large)> opSerialLargeCSharpAsync(Serialize.Large i,
            Current current) => new ValueTask<(Serialize.Large, Serialize.Large)>((i, i));

        public ValueTask<(Serialize.Struct, Serialize.Struct)> opSerialStructCSharpAsync(Serialize.Struct i,
            Current current) => new ValueTask<(Serialize.Struct, Serialize.Struct)>((i, i));

        private static ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)> ToReturnValue<T>(T[] input) where T : struct =>
            new ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)>((input, input));

        private static ValueTask<(IEnumerable<T>, IEnumerable<T>)> ToReturnValue<T>(IEnumerable<T> input) =>
            new ValueTask<(IEnumerable<T>, IEnumerable<T>)>((input, input));
    }
}
