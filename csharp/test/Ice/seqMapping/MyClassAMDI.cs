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

                public Task<Test.MyClass.OpAByteSReturnValue>
                opAByteSAsync(byte[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAByteSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLByteSReturnValue>
                opLByteSAsync(List<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLByteSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKByteSReturnValue>
                opKByteSAsync(LinkedList<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKByteSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQByteSReturnValue>
                opQByteSAsync(Queue<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQByteSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSByteSReturnValue>
                opSByteSAsync(Stack<byte> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSByteSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpABoolSReturnValue>
                opABoolSAsync(bool[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpABoolSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLBoolSReturnValue>
                opLBoolSAsync(List<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLBoolSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKBoolSReturnValue>
                opKBoolSAsync(LinkedList<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKBoolSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQBoolSReturnValue>
                opQBoolSAsync(Queue<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQBoolSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSBoolSReturnValue>
                opSBoolSAsync(Stack<bool> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSBoolSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAShortSReturnValue>
                opAShortSAsync(short[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAShortSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLShortSReturnValue>
                opLShortSAsync(List<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLShortSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKShortSReturnValue>
                opKShortSAsync(LinkedList<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKShortSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQShortSReturnValue>
                opQShortSAsync(Queue<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQShortSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSShortSReturnValue>
                opSShortSAsync(Stack<short> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSShortSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAIntSReturnValue>
                opAIntSAsync(int[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLIntSReturnValue>
                opLIntSAsync(List<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKIntSReturnValue>
                opKIntSAsync(LinkedList<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQIntSReturnValue>
                opQIntSAsync(Queue<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSIntSReturnValue>
                opSIntSAsync(Stack<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpALongSReturnValue>
                opALongSAsync(long[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpALongSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLLongSReturnValue>
                opLLongSAsync(List<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLLongSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKLongSReturnValue>
                opKLongSAsync(LinkedList<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKLongSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQLongSReturnValue>
                opQLongSAsync(Queue<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQLongSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSLongSReturnValue>
                opSLongSAsync(Stack<long> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSLongSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAFloatSReturnValue>
                opAFloatSAsync(float[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAFloatSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLFloatSReturnValue>
                opLFloatSAsync(List<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLFloatSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKFloatSReturnValue>
                opKFloatSAsync(LinkedList<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKFloatSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQFloatSReturnValue>
                opQFloatSAsync(Queue<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQFloatSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSFloatSReturnValue>
                opSFloatSAsync(Stack<float> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSFloatSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpADoubleSReturnValue>
                opADoubleSAsync(double[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpADoubleSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLDoubleSReturnValue>
                opLDoubleSAsync(List<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLDoubleSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKDoubleSReturnValue>
                opKDoubleSAsync(LinkedList<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKDoubleSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQDoubleSReturnValue>
                opQDoubleSAsync(Queue<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQDoubleSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSDoubleSReturnValue>
                opSDoubleSAsync(Stack<double> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSDoubleSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAStringSReturnValue>
                opAStringSAsync(string[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAStringSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLStringSReturnValue>
                opLStringSAsync(List<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLStringSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKStringSReturnValue>
                opKStringSAsync(LinkedList<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKStringSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQStringSReturnValue>
                opQStringSAsync(Queue<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQStringSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSStringSReturnValue>
                opSStringSAsync(Stack<string> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSStringSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAObjectSReturnValue>
                opAObjectSAsync(Value[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAObjectSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLObjectSReturnValue>
                opLObjectSAsync(List<Value> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLObjectSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAObjectPrxSReturnValue>
                opAObjectPrxSAsync(IObjectPrx[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAObjectPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLObjectPrxSReturnValue>
                opLObjectPrxSAsync(List<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLObjectPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKObjectPrxSReturnValue>
                opKObjectPrxSAsync(LinkedList<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKObjectPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQObjectPrxSReturnValue>
                opQObjectPrxSAsync(Queue<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQObjectPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSObjectPrxSReturnValue>
                opSObjectPrxSAsync(Stack<IObjectPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSObjectPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAStructSReturnValue>
                opAStructSAsync(Test.S[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAStructSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLStructSReturnValue>
                opLStructSAsync(List<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLStructSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKStructSReturnValue>
                opKStructSAsync(LinkedList<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKStructSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQStructSReturnValue>
                opQStructSAsync(Queue<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQStructSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSStructSReturnValue>
                opSStructSAsync(Stack<Test.S> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSStructSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAStructSDReturnValue>
                opAStructSDAsync(Test.SD[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAStructSDReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLStructSDReturnValue>
                opLStructSDAsync(List<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLStructSDReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKStructSDReturnValue>
                opKStructSDAsync(LinkedList<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKStructSDReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQStructSDReturnValue>
                opQStructSDAsync(Queue<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQStructSDReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSStructSDReturnValue>
                opSStructSDAsync(Stack<Test.SD> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSStructSDReturnValue(i, i));
                }

                public Task<Test.MyClass.OpACVSReturnValue>
                opACVSAsync(Test.CV[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpACVSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLCVSReturnValue>
                opLCVSAsync(List<Test.CV> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLCVSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAIPrxSReturnValue>
                opAIPrxSAsync(Test.IPrx[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAIPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLIPrxSReturnValue>
                opLIPrxSAsync(List<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLIPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKIPrxSReturnValue>
                opKIPrxSAsync(LinkedList<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKIPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQIPrxSReturnValue>
                opQIPrxSAsync(Queue<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQIPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSIPrxSReturnValue>
                opSIPrxSAsync(Stack<Test.IPrx> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSIPrxSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpACRSReturnValue>
                opACRSAsync(Test.CR[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpACRSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLCRSReturnValue>
                opLCRSAsync(List<Test.CR> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLCRSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpAEnSReturnValue>
                opAEnSAsync(Test.En[] i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpAEnSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpLEnSReturnValue>
                opLEnSAsync(List<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpLEnSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpKEnSReturnValue>
                opKEnSAsync(LinkedList<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpKEnSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpQEnSReturnValue>
                opQEnSAsync(Queue<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpQEnSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSEnSReturnValue>
                opSEnSAsync(Stack<Test.En> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSEnSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpCustomIntSReturnValue>
                opCustomIntSAsync(Custom<int> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpCustomIntSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpCustomCVSReturnValue>
                opCustomCVSAsync(Custom<Test.CV> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpCustomCVSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpCustomIntSSReturnValue>
                opCustomIntSSAsync(Custom<Custom<int>> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpCustomIntSSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpCustomCVSSReturnValue>
                opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpCustomCVSSReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSerialSmallCSharpReturnValue>
                opSerialSmallCSharpAsync(Serialize.Small i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSerialSmallCSharpReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSerialLargeCSharpReturnValue>
                opSerialLargeCSharpAsync(Serialize.Large i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSerialLargeCSharpReturnValue(i, i));
                }

                public Task<Test.MyClass.OpSerialStructCSharpReturnValue>
                opSerialStructCSharpAsync(Serialize.Struct i, Current current)
                {
                    return Task.FromResult(new Test.MyClass.OpSerialStructCSharpReturnValue(i, i));
                }
            }
        }
    }
}
