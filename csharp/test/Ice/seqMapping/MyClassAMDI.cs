//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice
{
    namespace seqMapping
    {
        namespace AMD
        {
            public sealed class MyClassI : Test.MyClass
            {
                public Task shutdownAsync(Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return null;
                }

                public Task<(byte[] returnValue, byte[] o)>
                opAByteSAsync(byte[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<byte> returnValue, List<byte> o)>
                opLByteSAsync(List<byte> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<byte> returnValue, LinkedList<byte> o)>
                opKByteSAsync(LinkedList<byte> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<byte> returnValue, Queue<byte> o)>
                opQByteSAsync(Queue<byte> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<byte> returnValue, Stack<byte> o)>
                opSByteSAsync(Stack<byte> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(bool[] returnValue, bool[] o)>
                opABoolSAsync(bool[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<bool> returnValue, List<bool> o)>
                opLBoolSAsync(List<bool> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<bool> returnValue, LinkedList<bool> o)>
                opKBoolSAsync(LinkedList<bool> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<bool> returnValue, Queue<bool> o)>
                opQBoolSAsync(Queue<bool> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<bool> returnValue, Stack<bool> o)>
                opSBoolSAsync(Stack<bool> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(short[] returnValue, short[] o)>
                opAShortSAsync(short[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<short> returnValue, List<short> o)>
                opLShortSAsync(List<short> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<short> returnValue, LinkedList<short> o)>
                opKShortSAsync(LinkedList<short> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<short> returnValue, Queue<short> o)>
                opQShortSAsync(Queue<short> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<short> returnValue, Stack<short> o)>
                opSShortSAsync(Stack<short> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(int[] returnValue, int[] o)>
                opAIntSAsync(int[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<int> returnValue, List<int> o)>
                opLIntSAsync(List<int> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<int> returnValue, LinkedList<int> o)>
                opKIntSAsync(LinkedList<int> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<int> returnValue, Queue<int> o)>
                opQIntSAsync(Queue<int> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<int> returnValue, Stack<int> o)>
                opSIntSAsync(Stack<int> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(long[] returnValue, long[] o)>
                opALongSAsync(long[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<long> returnValue, List<long> o)>
                opLLongSAsync(List<long> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<long> returnValue, LinkedList<long> o)>
                opKLongSAsync(LinkedList<long> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<long> returnValue, Queue<long> o)>
                opQLongSAsync(Queue<long> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<long> returnValue, Stack<long> o)>
                opSLongSAsync(Stack<long> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(float[] returnValue, float[] o)>
                opAFloatSAsync(float[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<float> returnValue, List<float> o)>
                opLFloatSAsync(List<float> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<float> returnValue, LinkedList<float> o)>
                opKFloatSAsync(LinkedList<float> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<float> returnValue, Queue<float> o)>
                opQFloatSAsync(Queue<float> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<float> returnValue, Stack<float> o)>
                opSFloatSAsync(Stack<float> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(double[] returnValue, double[] o)>
                opADoubleSAsync(double[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<double> returnValue, List<double> o)>
                opLDoubleSAsync(List<double> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<double> returnValue, LinkedList<double> o)>
                opKDoubleSAsync(LinkedList<double> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<double> returnValue, Queue<double> o)>
                opQDoubleSAsync(Queue<double> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<double> returnValue, Stack<double> o)>
                opSDoubleSAsync(Stack<double> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(string[] returnValue, string[] o)>
                opAStringSAsync(string[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<string> returnValue, List<string> o)>
                opLStringSAsync(List<string> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<string> returnValue, LinkedList<string> o)>
                opKStringSAsync(LinkedList<string> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<string> returnValue, Queue<string> o)>
                opQStringSAsync(Queue<string> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<string> returnValue, Stack<string> o)>
                opSStringSAsync(Stack<string> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Ice.Value[] returnValue, Ice.Value[] o)>
                opAObjectSAsync(Ice.Value[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Ice.Value> returnValue, List<Ice.Value> o)>
                opLObjectSAsync(List<Ice.Value> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Ice.IObjectPrx[] returnValue, Ice.IObjectPrx[] o)>
                opAObjectPrxSAsync(Ice.IObjectPrx[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Ice.IObjectPrx> returnValue, List<Ice.IObjectPrx> o)>
                opLObjectPrxSAsync(List<Ice.IObjectPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<Ice.IObjectPrx> returnValue, LinkedList<Ice.IObjectPrx> o)>
                opKObjectPrxSAsync(LinkedList<Ice.IObjectPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<Ice.IObjectPrx> returnValue, Queue<Ice.IObjectPrx> o)>
                opQObjectPrxSAsync(Queue<Ice.IObjectPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<Ice.IObjectPrx> returnValue, Stack<Ice.IObjectPrx> o)>
                opSObjectPrxSAsync(Stack<Ice.IObjectPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.S[] returnValue, Test.S[] o)>
                opAStructSAsync(Test.S[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.S> returnValue, List<Test.S> o)>
                opLStructSAsync(List<Test.S> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<Test.S> returnValue, LinkedList<Test.S> o)>
                opKStructSAsync(LinkedList<Test.S> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<Test.S> returnValue, Queue<Test.S> o)>
                opQStructSAsync(Queue<Test.S> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<Test.S> returnValue, Stack<Test.S> o)>
                opSStructSAsync(Stack<Test.S> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.SD[] returnValue, Test.SD[] o)>
                opAStructSDAsync(Test.SD[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.SD> returnValue, List<Test.SD> o)>
                opLStructSDAsync(List<Test.SD> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<Test.SD> returnValue, LinkedList<Test.SD> o)>
                opKStructSDAsync(LinkedList<Test.SD> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<Test.SD> returnValue, Queue<Test.SD> o)>
                opQStructSDAsync(Queue<Test.SD> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<Test.SD> returnValue, Stack<Test.SD> o)>
                opSStructSDAsync(Stack<Test.SD> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.CV[] returnValue, Test.CV[] o)>
                opACVSAsync(Test.CV[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.CV> returnValue, List<Test.CV> o)>
                opLCVSAsync(List<Test.CV> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.IPrx[] returnValue, Test.IPrx[] o)>
                opAIPrxSAsync(Test.IPrx[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.IPrx> returnValue, List<Test.IPrx> o)>
                opLIPrxSAsync(List<Test.IPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<Test.IPrx> returnValue, LinkedList<Test.IPrx> o)>
                opKIPrxSAsync(LinkedList<Test.IPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<Test.IPrx> returnValue, Queue<Test.IPrx> o)>
                opQIPrxSAsync(Queue<Test.IPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<Test.IPrx> returnValue, Stack<Test.IPrx> o)>
                opSIPrxSAsync(Stack<Test.IPrx> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.CR[] returnValue, Test.CR[] o)>
                opACRSAsync(Test.CR[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.CR> returnValue, List<Test.CR> o)>
                opLCRSAsync(List<Test.CR> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Test.En[] returnValue, Test.En[] o)>
                opAEnSAsync(Test.En[] i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(List<Test.En> returnValue, List<Test.En> o)>
                opLEnSAsync(List<Test.En> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(LinkedList<Test.En> returnValue, LinkedList<Test.En> o)>
                opKEnSAsync(LinkedList<Test.En> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Queue<Test.En> returnValue, Queue<Test.En> o)>
                opQEnSAsync(Queue<Test.En> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Stack<Test.En> returnValue, Stack<Test.En> o)>
                opSEnSAsync(Stack<Test.En> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Custom<int> returnValue, Custom<int> o)>
                opCustomIntSAsync(Custom<int> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Custom<Test.CV> returnValue, Custom<Test.CV> o)>
                opCustomCVSAsync(Custom<Test.CV> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Custom<Custom<int>> returnValue, Custom<Custom<int>> o)>
                opCustomIntSSAsync(Custom<Custom<int>> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Custom<Custom<Test.CV>> returnValue, Custom<Custom<Test.CV>> o)>
                opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Serialize.Small returnValue, Serialize.Small o)>
                opSerialSmallCSharpAsync(Serialize.Small i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Serialize.Large returnValue, Serialize.Large o)>
                opSerialLargeCSharpAsync(Serialize.Large i, Current current)
                {
                    return Task.FromResult((i, i));
                }

                public Task<(Serialize.Struct returnValue, Serialize.Struct o)>
                opSerialStructCSharpAsync(Serialize.Struct i, Current current)
                {
                    return Task.FromResult((i, i));
                }
            }
        }
    }
}
