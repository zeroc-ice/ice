//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.AMD.SeqMapping
{
    public sealed class MyClass : IMyClass
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

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> opAStructSAsync(S[] i, Current current) =>
            ToReturnValue(i as IEnumerable<S>);

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> opLStructSAsync(List<S> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> opKStructSAsync(LinkedList<S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> opQStructSAsync(Queue<S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<S>, IEnumerable<S>)> opSStructSAsync(Stack<S> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> opAStructSDAsync(SD[] i, Current current) =>
            ToReturnValue(i as IEnumerable<SD>);

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> opLStructSDAsync(List<SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> opKStructSDAsync(LinkedList<SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> opQStructSDAsync(Queue<SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<SD>, IEnumerable<SD>)> opSStructSDAsync(Stack<SD> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> opACVSAsync(CV?[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> opLCVSAsync(List<CV?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> opAIPrxSAsync(IIPrx?[] i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> opLIPrxSAsync(List<IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> opKIPrxSAsync(LinkedList<IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> opQIPrxSAsync(Queue<IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<IIPrx?>, IEnumerable<IIPrx?>)> opSIPrxSAsync(Stack<IIPrx?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<CR?>, IEnumerable<CR?>)> opACRSAsync(CR?[] i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<CR?>, IEnumerable<CR?>)> opLCRSAsync(List<CR?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> opAEnSAsync(En[] i, Current current) =>
            ToReturnValue(i as IEnumerable<En>);

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> opLEnSAsync(List<En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> opKEnSAsync(LinkedList<En> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> opQEnSAsync(Queue<En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<En>, IEnumerable<En>)> opSEnSAsync(Stack<En> i, Current current) =>
            ToReturnValue(i);

        public ValueTask<(IEnumerable<int>, IEnumerable<int>)> opCustomIntSAsync(ZeroC.Ice.Test.SeqMapping.Custom<int> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<CV?>, IEnumerable<CV?>)> opCustomCVSAsync(ZeroC.Ice.Test.SeqMapping.Custom<CV?> i,
            Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<ZeroC.Ice.Test.SeqMapping.Custom<int>>,
                          IEnumerable<ZeroC.Ice.Test.SeqMapping.Custom<int>>)>
            opCustomIntSSAsync(ZeroC.Ice.Test.SeqMapping.Custom<ZeroC.Ice.Test.SeqMapping.Custom<int>> i,
                Current current) => ToReturnValue(i);

        public ValueTask<(IEnumerable<ZeroC.Ice.Test.SeqMapping.Custom<CV?>>,
                          IEnumerable<ZeroC.Ice.Test.SeqMapping.Custom<CV?>>)> opCustomCVSSAsync(
            ZeroC.Ice.Test.SeqMapping.Custom<ZeroC.Ice.Test.SeqMapping.Custom<CV?>> i, Current current)
                => ToReturnValue(i);

        public ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Small, ZeroC.Ice.Test.SeqMapping.Serialize.Small)>
            opSerialSmallCSharpAsync(ZeroC.Ice.Test.SeqMapping.Serialize.Small i, Current current) =>
            new ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Small, ZeroC.Ice.Test.SeqMapping.Serialize.Small)>((i, i));

        public ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Large, ZeroC.Ice.Test.SeqMapping.Serialize.Large)>
            opSerialLargeCSharpAsync(ZeroC.Ice.Test.SeqMapping.Serialize.Large i, Current current) =>
            new ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Large, ZeroC.Ice.Test.SeqMapping.Serialize.Large)>((i, i));

        public ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Struct, ZeroC.Ice.Test.SeqMapping.Serialize.Struct)>
            opSerialStructCSharpAsync(ZeroC.Ice.Test.SeqMapping.Serialize.Struct i, Current current) =>
            new ValueTask<(ZeroC.Ice.Test.SeqMapping.Serialize.Struct, ZeroC.Ice.Test.SeqMapping.Serialize.Struct)>((i, i));

        private static ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)> ToReturnValue<T>(T[] input) where T : struct =>
            new ValueTask<(ReadOnlyMemory<T>, ReadOnlyMemory<T>)>((input, input));

        private static ValueTask<(IEnumerable<T>, IEnumerable<T>)> ToReturnValue<T>(IEnumerable<T> input) =>
            new ValueTask<(IEnumerable<T>, IEnumerable<T>)>((input, input));
    }
}
