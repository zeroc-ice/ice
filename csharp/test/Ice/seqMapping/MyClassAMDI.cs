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

                public Task<Test.MyClass.opAByteSResult>
                opAByteSAsync(byte[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAByteSResult(i, i));
                }

                public Task<Test.MyClass.opLByteSResult>
                opLByteSAsync(List<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLByteSResult(i, i));
                }

                public Task<Test.MyClass.opKByteSResult>
                opKByteSAsync(LinkedList<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKByteSResult(i, i));
                }

                public Task<Test.MyClass.opQByteSResult>
                opQByteSAsync(Queue<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQByteSResult(i, i));
                }

                public Task<Test.MyClass.opSByteSResult>
                opSByteSAsync(Stack<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSByteSResult(i, i));
                }

                public Task<Test.MyClass.opABoolSResult>
                opABoolSAsync(bool[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opABoolSResult(i, i));
                }

                public Task<Test.MyClass.opLBoolSResult>
                opLBoolSAsync(List<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLBoolSResult(i, i));
                }

                public Task<Test.MyClass.opKBoolSResult>
                opKBoolSAsync(LinkedList<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKBoolSResult(i, i));
                }

                public Task<Test.MyClass.opQBoolSResult>
                opQBoolSAsync(Queue<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQBoolSResult(i, i));
                }

                public Task<Test.MyClass.opSBoolSResult>
                opSBoolSAsync(Stack<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSBoolSResult(i, i));
                }

                public Task<Test.MyClass.opAShortSResult>
                opAShortSAsync(short[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAShortSResult(i, i));
                }

                public Task<Test.MyClass.opLShortSResult>
                opLShortSAsync(List<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLShortSResult(i, i));
                }

                public Task<Test.MyClass.opKShortSResult>
                opKShortSAsync(LinkedList<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKShortSResult(i, i));
                }

                public Task<Test.MyClass.opQShortSResult>
                opQShortSAsync(Queue<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQShortSResult(i, i));
                }

                public Task<Test.MyClass.opSShortSResult>
                opSShortSAsync(Stack<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSShortSResult(i, i));
                }

                public Task<Test.MyClass.opAIntSResult>
                opAIntSAsync(int[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAIntSResult(i, i));
                }

                public Task<Test.MyClass.opLIntSResult>
                opLIntSAsync(List<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLIntSResult(i, i));
                }

                public Task<Test.MyClass.opKIntSResult>
                opKIntSAsync(LinkedList<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKIntSResult(i, i));
                }

                public Task<Test.MyClass.opQIntSResult>
                opQIntSAsync(Queue<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQIntSResult(i, i));
                }

                public Task<Test.MyClass.opSIntSResult>
                opSIntSAsync(Stack<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSIntSResult(i, i));
                }

                public Task<Test.MyClass.opALongSResult>
                opALongSAsync(long[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opALongSResult(i, i));
                }

                public Task<Test.MyClass.opLLongSResult>
                opLLongSAsync(List<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLLongSResult(i, i));
                }

                public Task<Test.MyClass.opKLongSResult>
                opKLongSAsync(LinkedList<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKLongSResult(i, i));
                }

                public Task<Test.MyClass.opQLongSResult>
                opQLongSAsync(Queue<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQLongSResult(i, i));
                }

                public Task<Test.MyClass.opSLongSResult>
                opSLongSAsync(Stack<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSLongSResult(i, i));
                }

                public Task<Test.MyClass.opAFloatSResult>
                opAFloatSAsync(float[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAFloatSResult(i, i));
                }

                public Task<Test.MyClass.opLFloatSResult>
                opLFloatSAsync(List<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLFloatSResult(i, i));
                }

                public Task<Test.MyClass.opKFloatSResult>
                opKFloatSAsync(LinkedList<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKFloatSResult(i, i));
                }

                public Task<Test.MyClass.opQFloatSResult>
                opQFloatSAsync(Queue<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQFloatSResult(i, i));
                }

                public Task<Test.MyClass.opSFloatSResult>
                opSFloatSAsync(Stack<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSFloatSResult(i, i));
                }

                public Task<Test.MyClass.opADoubleSResult>
                opADoubleSAsync(double[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opADoubleSResult(i, i));
                }

                public Task<Test.MyClass.opLDoubleSResult>
                opLDoubleSAsync(List<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLDoubleSResult(i, i));
                }

                public Task<Test.MyClass.opKDoubleSResult>
                opKDoubleSAsync(LinkedList<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKDoubleSResult(i, i));
                }

                public Task<Test.MyClass.opQDoubleSResult>
                opQDoubleSAsync(Queue<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQDoubleSResult(i, i));
                }

                public Task<Test.MyClass.opSDoubleSResult>
                opSDoubleSAsync(Stack<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSDoubleSResult(i, i));
                }

                public Task<Test.MyClass.opAStringSResult>
                opAStringSAsync(string[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAStringSResult(i, i));
                }

                public Task<Test.MyClass.opLStringSResult>
                opLStringSAsync(List<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLStringSResult(i, i));
                }

                public Task<Test.MyClass.opKStringSResult>
                opKStringSAsync(LinkedList<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKStringSResult(i, i));
                }

                public Task<Test.MyClass.opQStringSResult>
                opQStringSAsync(Queue<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQStringSResult(i, i));
                }

                public Task<Test.MyClass.opSStringSResult>
                opSStringSAsync(Stack<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSStringSResult(i, i));
                }

                public Task<Test.MyClass.opAObjectSResult>
                opAObjectSAsync(Value[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAObjectSResult(i, i));
                }

                public Task<Test.MyClass.opLObjectSResult>
                opLObjectSAsync(List<Value> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLObjectSResult(i, i));
                }

                public Task<Test.MyClass.opAObjectPrxSResult>
                opAObjectPrxSAsync(IObjectPrx[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass.opLObjectPrxSResult>
                opLObjectPrxSAsync(List<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass.opKObjectPrxSResult>
                opKObjectPrxSAsync(LinkedList<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass.opQObjectPrxSResult>
                opQObjectPrxSAsync(Queue<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass.opSObjectPrxSResult>
                opSObjectPrxSAsync(Stack<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass.opAStructSResult>
                opAStructSAsync(Test.S[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAStructSResult(i, i));
                }

                public Task<Test.MyClass.opLStructSResult>
                opLStructSAsync(List<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLStructSResult(i, i));
                }

                public Task<Test.MyClass.opKStructSResult>
                opKStructSAsync(LinkedList<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKStructSResult(i, i));
                }

                public Task<Test.MyClass.opQStructSResult>
                opQStructSAsync(Queue<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQStructSResult(i, i));
                }

                public Task<Test.MyClass.opSStructSResult>
                opSStructSAsync(Stack<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSStructSResult(i, i));
                }

                public Task<Test.MyClass.opAStructSDResult>
                opAStructSDAsync(Test.SD[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAStructSDResult(i, i));
                }

                public Task<Test.MyClass.opLStructSDResult>
                opLStructSDAsync(List<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLStructSDResult(i, i));
                }

                public Task<Test.MyClass.opKStructSDResult>
                opKStructSDAsync(LinkedList<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKStructSDResult(i, i));
                }

                public Task<Test.MyClass.opQStructSDResult>
                opQStructSDAsync(Queue<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQStructSDResult(i, i));
                }

                public Task<Test.MyClass.opSStructSDResult>
                opSStructSDAsync(Stack<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSStructSDResult(i, i));
                }

                public Task<Test.MyClass.opACVSResult>
                opACVSAsync(Test.CV[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opACVSResult(i, i));
                }

                public Task<Test.MyClass.opLCVSResult>
                opLCVSAsync(List<Test.CV> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLCVSResult(i, i));
                }

                public Task<Test.MyClass.opAIPrxSResult>
                opAIPrxSAsync(Test.IPrx[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAIPrxSResult(i, i));
                }

                public Task<Test.MyClass.opLIPrxSResult>
                opLIPrxSAsync(List<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLIPrxSResult(i, i));
                }

                public Task<Test.MyClass.opKIPrxSResult>
                opKIPrxSAsync(LinkedList<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKIPrxSResult(i, i));
                }

                public Task<Test.MyClass.opQIPrxSResult>
                opQIPrxSAsync(Queue<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQIPrxSResult(i, i));
                }

                public Task<Test.MyClass.opSIPrxSResult>
                opSIPrxSAsync(Stack<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSIPrxSResult(i, i));
                }

                public Task<Test.MyClass.opACRSResult>
                opACRSAsync(Test.CR[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opACRSResult(i, i));
                }

                public Task<Test.MyClass.opLCRSResult>
                opLCRSAsync(List<Test.CR> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLCRSResult(i, i));
                }

                public Task<Test.MyClass.opAEnSResult>
                opAEnSAsync(Test.En[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opAEnSResult(i, i));
                }

                public Task<Test.MyClass.opLEnSResult>
                opLEnSAsync(List<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opLEnSResult(i, i));
                }

                public Task<Test.MyClass.opKEnSResult>
                opKEnSAsync(LinkedList<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opKEnSResult(i, i));
                }

                public Task<Test.MyClass.opQEnSResult>
                opQEnSAsync(Queue<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opQEnSResult(i, i));
                }

                public Task<Test.MyClass.opSEnSResult>
                opSEnSAsync(Stack<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSEnSResult(i, i));
                }

                public Task<Test.MyClass.opCustomIntSResult>
                opCustomIntSAsync(Custom<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opCustomIntSResult(i, i));
                }

                public Task<Test.MyClass.opCustomCVSResult>
                opCustomCVSAsync(Custom<Test.CV> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opCustomCVSResult(i, i));
                }

                public Task<Test.MyClass.opCustomIntSSResult>
                opCustomIntSSAsync(Custom<Custom<int>> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opCustomIntSSResult(i, i));
                }

                public Task<Test.MyClass.opCustomCVSSResult>
                opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opCustomCVSSResult(i, i));
                }

                public Task<Test.MyClass.opSerialSmallCSharpResult>
                opSerialSmallCSharpAsync(Serialize.Small i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSerialSmallCSharpResult(i, i));
                }

                public Task<Test.MyClass.opSerialLargeCSharpResult>
                opSerialLargeCSharpAsync(Serialize.Large i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSerialLargeCSharpResult(i, i));
                }

                public Task<Test.MyClass.opSerialStructCSharpResult>
                opSerialStructCSharpAsync(Serialize.Struct i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.opSerialStructCSharpResult(i, i));
                }
            }
        }
    }
}
