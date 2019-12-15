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
                public Task shutdownAsync(Ice.Current current)
                {
                    current.Adapter.Communicator.shutdown();
                    return null;
                }

                public Task<Test.MyClass_OpAByteSResult>
                opAByteSAsync(byte[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAByteSResult(i, i));
                }

                public Task<Test.MyClass_OpLByteSResult>
                opLByteSAsync(List<byte> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLByteSResult(i, i));
                }

                public Task<Test.MyClass_OpKByteSResult>
                opKByteSAsync(LinkedList<byte> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKByteSResult(i, i));
                }

                public Task<Test.MyClass_OpQByteSResult>
                opQByteSAsync(Queue<byte> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQByteSResult(i, i));
                }

                public Task<Test.MyClass_OpSByteSResult>
                opSByteSAsync(Stack<byte> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSByteSResult(i, i));
                }

                public Task<Test.MyClass_OpABoolSResult>
                opABoolSAsync(bool[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpABoolSResult(i, i));
                }

                public Task<Test.MyClass_OpLBoolSResult>
                opLBoolSAsync(List<bool> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLBoolSResult(i, i));
                }

                public Task<Test.MyClass_OpKBoolSResult>
                opKBoolSAsync(LinkedList<bool> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKBoolSResult(i, i));
                }

                public Task<Test.MyClass_OpQBoolSResult>
                opQBoolSAsync(Queue<bool> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQBoolSResult(i, i));
                }

                public Task<Test.MyClass_OpSBoolSResult>
                opSBoolSAsync(Stack<bool> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSBoolSResult(i, i));
                }

                public Task<Test.MyClass_OpAShortSResult>
                opAShortSAsync(short[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAShortSResult(i, i));
                }

                public Task<Test.MyClass_OpLShortSResult>
                opLShortSAsync(List<short> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLShortSResult(i, i));
                }

                public Task<Test.MyClass_OpKShortSResult>
                opKShortSAsync(LinkedList<short> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKShortSResult(i, i));
                }

                public Task<Test.MyClass_OpQShortSResult>
                opQShortSAsync(Queue<short> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQShortSResult(i, i));
                }

                public Task<Test.MyClass_OpSShortSResult>
                opSShortSAsync(Stack<short> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSShortSResult(i, i));
                }

                public Task<Test.MyClass_OpAIntSResult>
                opAIntSAsync(int[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAIntSResult(i, i));
                }

                public Task<Test.MyClass_OpLIntSResult>
                opLIntSAsync(List<int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLIntSResult(i, i));
                }

                public Task<Test.MyClass_OpKIntSResult>
                opKIntSAsync(LinkedList<int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKIntSResult(i, i));
                }

                public Task<Test.MyClass_OpQIntSResult>
                opQIntSAsync(Queue<int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQIntSResult(i, i));
                }

                public Task<Test.MyClass_OpSIntSResult>
                opSIntSAsync(Stack<int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSIntSResult(i, i));
                }

                public Task<Test.MyClass_OpALongSResult>
                opALongSAsync(long[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpALongSResult(i, i));
                }

                public Task<Test.MyClass_OpLLongSResult>
                opLLongSAsync(List<long> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLLongSResult(i, i));
                }

                public Task<Test.MyClass_OpKLongSResult>
                opKLongSAsync(LinkedList<long> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKLongSResult(i, i));
                }

                public Task<Test.MyClass_OpQLongSResult>
                opQLongSAsync(Queue<long> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQLongSResult(i, i));
                }

                public Task<Test.MyClass_OpSLongSResult>
                opSLongSAsync(Stack<long> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSLongSResult(i, i));
                }

                public Task<Test.MyClass_OpAFloatSResult>
                opAFloatSAsync(float[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAFloatSResult(i, i));
                }

                public Task<Test.MyClass_OpLFloatSResult>
                opLFloatSAsync(List<float> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLFloatSResult(i, i));
                }

                public Task<Test.MyClass_OpKFloatSResult>
                opKFloatSAsync(LinkedList<float> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKFloatSResult(i, i));
                }

                public Task<Test.MyClass_OpQFloatSResult>
                opQFloatSAsync(Queue<float> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQFloatSResult(i, i));
                }

                public Task<Test.MyClass_OpSFloatSResult>
                opSFloatSAsync(Stack<float> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSFloatSResult(i, i));
                }

                public Task<Test.MyClass_OpADoubleSResult>
                opADoubleSAsync(double[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpADoubleSResult(i, i));
                }

                public Task<Test.MyClass_OpLDoubleSResult>
                opLDoubleSAsync(List<double> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLDoubleSResult(i, i));
                }

                public Task<Test.MyClass_OpKDoubleSResult>
                opKDoubleSAsync(LinkedList<double> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKDoubleSResult(i, i));
                }

                public Task<Test.MyClass_OpQDoubleSResult>
                opQDoubleSAsync(Queue<double> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQDoubleSResult(i, i));
                }

                public Task<Test.MyClass_OpSDoubleSResult>
                opSDoubleSAsync(Stack<double> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSDoubleSResult(i, i));
                }

                public Task<Test.MyClass_OpAStringSResult>
                opAStringSAsync(string[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAStringSResult(i, i));
                }

                public Task<Test.MyClass_OpLStringSResult>
                opLStringSAsync(List<string> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLStringSResult(i, i));
                }

                public Task<Test.MyClass_OpKStringSResult>
                opKStringSAsync(LinkedList<string> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKStringSResult(i, i));
                }

                public Task<Test.MyClass_OpQStringSResult>
                opQStringSAsync(Queue<string> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQStringSResult(i, i));
                }

                public Task<Test.MyClass_OpSStringSResult>
                opSStringSAsync(Stack<string> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSStringSResult(i, i));
                }

                public Task<Test.MyClass_OpAObjectSResult> opAObjectSAsync(Ice.Value[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAObjectSResult(i, i));
                }

                public Task<Test.MyClass_OpLObjectSResult>
                opLObjectSAsync(List<Ice.Value> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLObjectSResult(i, i));
                }

                public Task<Test.MyClass_OpAObjectPrxSResult>
                opAObjectPrxSAsync(Ice.IObjectPrx[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpLObjectPrxSResult>
                opLObjectPrxSAsync(List<Ice.IObjectPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpKObjectPrxSResult>
                opKObjectPrxSAsync(LinkedList<Ice.IObjectPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpQObjectPrxSResult>
                opQObjectPrxSAsync(Queue<Ice.IObjectPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpSObjectPrxSResult>
                opSObjectPrxSAsync(Stack<Ice.IObjectPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSObjectPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpAStructSResult>
                opAStructSAsync(Test.S[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAStructSResult(i, i));
                }

                public Task<Test.MyClass_OpLStructSResult>
                opLStructSAsync(List<Test.S> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLStructSResult(i, i));
                }

                public Task<Test.MyClass_OpKStructSResult>
                opKStructSAsync(LinkedList<Test.S> i, Ice.Current current)
                {
                    return Task.FromResult<Test.MyClass_OpKStructSResult>(new Test.MyClass_OpKStructSResult(i, i));
                }

                public Task<Test.MyClass_OpQStructSResult>
                opQStructSAsync(Queue<Test.S> i, Ice.Current current)
                {
                    return Task.FromResult<Test.MyClass_OpQStructSResult>(new Test.MyClass_OpQStructSResult(i, i));
                }

                public Task<Test.MyClass_OpSStructSResult>
                opSStructSAsync(Stack<Test.S> i, Ice.Current current)
                {
                    return Task.FromResult<Test.MyClass_OpSStructSResult>(new Test.MyClass_OpSStructSResult(i, i));
                }

                public Task<Test.MyClass_OpAStructSDResult>
                opAStructSDAsync(Test.SD[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAStructSDResult(i, i));
                }

                public Task<Test.MyClass_OpLStructSDResult>
                opLStructSDAsync(List<Test.SD> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLStructSDResult(i, i));
                }

                public Task<Test.MyClass_OpKStructSDResult>
                opKStructSDAsync(LinkedList<Test.SD> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKStructSDResult(i, i));
                }

                public Task<Test.MyClass_OpQStructSDResult>
                opQStructSDAsync(Queue<Test.SD> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQStructSDResult(i, i));
                }

                public Task<Test.MyClass_OpSStructSDResult>
                opSStructSDAsync(Stack<Test.SD> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSStructSDResult(i, i));
                }

                public Task<Test.MyClass_OpACVSResult>
                opACVSAsync(Test.CV[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpACVSResult(i, i));
                }

                public Task<Test.MyClass_OpLCVSResult>
                opLCVSAsync(List<Test.CV> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLCVSResult(i, i));
                }

                public Task<Test.MyClass_OpAIPrxSResult>
                opAIPrxSAsync(Test.IPrx[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAIPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpLIPrxSResult>
                opLIPrxSAsync(List<Test.IPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLIPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpKIPrxSResult>
                opKIPrxSAsync(LinkedList<Test.IPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKIPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpQIPrxSResult>
                opQIPrxSAsync(Queue<Test.IPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQIPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpSIPrxSResult>
                opSIPrxSAsync(Stack<Test.IPrx> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSIPrxSResult(i, i));
                }

                public Task<Test.MyClass_OpACRSResult>
                opACRSAsync(Test.CR[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpACRSResult(i, i));
                }

                public Task<Test.MyClass_OpLCRSResult>
                opLCRSAsync(List<Test.CR> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLCRSResult(i, i));
                }

                public Task<Test.MyClass_OpAEnSResult> opAEnSAsync(Test.En[] i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpAEnSResult(i, i));
                }

                public Task<Test.MyClass_OpLEnSResult>
                opLEnSAsync(List<Test.En> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpLEnSResult(i, i));
                }

                public Task<Test.MyClass_OpKEnSResult>
                opKEnSAsync(LinkedList<Test.En> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpKEnSResult(i, i));
                }

                public Task<Test.MyClass_OpQEnSResult>
                opQEnSAsync(Queue<Test.En> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpQEnSResult(i, i));
                }

                public Task<Test.MyClass_OpSEnSResult>
                opSEnSAsync(Stack<Test.En> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSEnSResult(i, i));
                }

                public Task<Test.MyClass_OpCustomIntSResult>
                opCustomIntSAsync(Custom<int> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpCustomIntSResult(i, i));
                }

                public Task<Test.MyClass_OpCustomCVSResult>
                opCustomCVSAsync(Custom<Test.CV> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpCustomCVSResult(i, i));
                }

                public Task<Test.MyClass_OpCustomIntSSResult>
                opCustomIntSSAsync(Custom<Custom<int>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpCustomIntSSResult(i, i));
                }

                public Task<Test.MyClass_OpCustomCVSSResult>
                opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpCustomCVSSResult(i, i));
                }

                public Task<Test.MyClass_OpSerialSmallCSharpResult>
                opSerialSmallCSharpAsync(Serialize.Small i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSerialSmallCSharpResult(i, i));
                }

                public Task<Test.MyClass_OpSerialLargeCSharpResult>
                opSerialLargeCSharpAsync(Serialize.Large i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSerialLargeCSharpResult(i, i));
                }

                public Task<Test.MyClass_OpSerialStructCSharpResult>
                opSerialStructCSharpAsync(Serialize.Struct i, Ice.Current current)
                {
                    return Task.FromResult(new Test.MyClass_OpSerialStructCSharpResult(i, i));
                }
            }
        }
    }
}
