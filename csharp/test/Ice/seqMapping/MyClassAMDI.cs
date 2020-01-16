//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;

namespace Ice.seqMapping.AMD
{
    public sealed class MyClass : Test.IMyClass
    {
        public Task shutdownAsync(Current current)
        {
            current.Adapter.Communicator.shutdown();
            return null;
        }

        public Task<Test.IMyClass.OpAByteSReturnValue>
        opAByteSAsync(byte[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAByteSReturnValue(i, i));

        public Task<Test.IMyClass.OpLByteSReturnValue>
        opLByteSAsync(List<byte> i, Current current) => Task.FromResult(new Test.IMyClass.OpLByteSReturnValue(i, i));

        public Task<Test.IMyClass.OpKByteSReturnValue>
        opKByteSAsync(LinkedList<byte> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKByteSReturnValue(i, i));

        public Task<Test.IMyClass.OpQByteSReturnValue>
        opQByteSAsync(Queue<byte> i, Current current) => Task.FromResult(new Test.IMyClass.OpQByteSReturnValue(i, i));

        public Task<Test.IMyClass.OpSByteSReturnValue>
        opSByteSAsync(Stack<byte> i, Current current) => Task.FromResult(new Test.IMyClass.OpSByteSReturnValue(i, i));

        public Task<Test.IMyClass.OpABoolSReturnValue>
        opABoolSAsync(bool[] i, Current current) => Task.FromResult(new Test.IMyClass.OpABoolSReturnValue(i, i));

        public Task<Test.IMyClass.OpLBoolSReturnValue>
        opLBoolSAsync(List<bool> i, Current current) => Task.FromResult(new Test.IMyClass.OpLBoolSReturnValue(i, i));

        public Task<Test.IMyClass.OpKBoolSReturnValue>
        opKBoolSAsync(LinkedList<bool> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKBoolSReturnValue(i, i));

        public Task<Test.IMyClass.OpQBoolSReturnValue>
        opQBoolSAsync(Queue<bool> i, Current current) => Task.FromResult(new Test.IMyClass.OpQBoolSReturnValue(i, i));

        public Task<Test.IMyClass.OpSBoolSReturnValue>
        opSBoolSAsync(Stack<bool> i, Current current) => Task.FromResult(new Test.IMyClass.OpSBoolSReturnValue(i, i));

        public Task<Test.IMyClass.OpAShortSReturnValue>
        opAShortSAsync(short[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAShortSReturnValue(i, i));

        public Task<Test.IMyClass.OpLShortSReturnValue>
        opLShortSAsync(List<short> i, Current current) => Task.FromResult(new Test.IMyClass.OpLShortSReturnValue(i, i));

        public Task<Test.IMyClass.OpKShortSReturnValue>
        opKShortSAsync(LinkedList<short> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKShortSReturnValue(i, i));

        public Task<Test.IMyClass.OpQShortSReturnValue>
        opQShortSAsync(Queue<short> i, Current current) => Task.FromResult(new Test.IMyClass.OpQShortSReturnValue(i, i));

        public Task<Test.IMyClass.OpSShortSReturnValue>
        opSShortSAsync(Stack<short> i, Current current) => Task.FromResult(new Test.IMyClass.OpSShortSReturnValue(i, i));

        public Task<Test.IMyClass.OpAIntSReturnValue>
        opAIntSAsync(int[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpLIntSReturnValue>
        opLIntSAsync(List<int> i, Current current) => Task.FromResult(new Test.IMyClass.OpLIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpKIntSReturnValue>
        opKIntSAsync(LinkedList<int> i, Current current) => Task.FromResult(new Test.IMyClass.OpKIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpQIntSReturnValue>
        opQIntSAsync(Queue<int> i, Current current) => Task.FromResult(new Test.IMyClass.OpQIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpSIntSReturnValue>
        opSIntSAsync(Stack<int> i, Current current) => Task.FromResult(new Test.IMyClass.OpSIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpALongSReturnValue>
        opALongSAsync(long[] i, Current current) => Task.FromResult(new Test.IMyClass.OpALongSReturnValue(i, i));

        public Task<Test.IMyClass.OpLLongSReturnValue>
        opLLongSAsync(List<long> i, Current current) => Task.FromResult(new Test.IMyClass.OpLLongSReturnValue(i, i));

        public Task<Test.IMyClass.OpKLongSReturnValue>
        opKLongSAsync(LinkedList<long> i, Current current) => Task.FromResult(new Test.IMyClass.OpKLongSReturnValue(i, i));

        public Task<Test.IMyClass.OpQLongSReturnValue>
        opQLongSAsync(Queue<long> i, Current current) => Task.FromResult(new Test.IMyClass.OpQLongSReturnValue(i, i));

        public Task<Test.IMyClass.OpSLongSReturnValue>
        opSLongSAsync(Stack<long> i, Current current) => Task.FromResult(new Test.IMyClass.OpSLongSReturnValue(i, i));

        public Task<Test.IMyClass.OpAFloatSReturnValue>
        opAFloatSAsync(float[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAFloatSReturnValue(i, i));

        public Task<Test.IMyClass.OpLFloatSReturnValue>
        opLFloatSAsync(List<float> i, Current current) => Task.FromResult(new Test.IMyClass.OpLFloatSReturnValue(i, i));

        public Task<Test.IMyClass.OpKFloatSReturnValue>
        opKFloatSAsync(LinkedList<float> i, Current current) => Task.FromResult(new Test.IMyClass.OpKFloatSReturnValue(i, i));

        public Task<Test.IMyClass.OpQFloatSReturnValue>
        opQFloatSAsync(Queue<float> i, Current current) => Task.FromResult(new Test.IMyClass.OpQFloatSReturnValue(i, i));

        public Task<Test.IMyClass.OpSFloatSReturnValue>
        opSFloatSAsync(Stack<float> i, Current current) => Task.FromResult(new Test.IMyClass.OpSFloatSReturnValue(i, i));

        public Task<Test.IMyClass.OpADoubleSReturnValue>
        opADoubleSAsync(double[] i, Current current) => Task.FromResult(new Test.IMyClass.OpADoubleSReturnValue(i, i));

        public Task<Test.IMyClass.OpLDoubleSReturnValue>
        opLDoubleSAsync(List<double> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLDoubleSReturnValue(i, i));

        public Task<Test.IMyClass.OpKDoubleSReturnValue>
        opKDoubleSAsync(LinkedList<double> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKDoubleSReturnValue(i, i));

        public Task<Test.IMyClass.OpQDoubleSReturnValue>
        opQDoubleSAsync(Queue<double> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQDoubleSReturnValue(i, i));

        public Task<Test.IMyClass.OpSDoubleSReturnValue>
        opSDoubleSAsync(Stack<double> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSDoubleSReturnValue(i, i));

        public Task<Test.IMyClass.OpAStringSReturnValue>
        opAStringSAsync(string[] i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpAStringSReturnValue(i, i));

        public Task<Test.IMyClass.OpLStringSReturnValue>
        opLStringSAsync(List<string> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLStringSReturnValue(i, i));

        public Task<Test.IMyClass.OpKStringSReturnValue>
        opKStringSAsync(LinkedList<string> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKStringSReturnValue(i, i));

        public Task<Test.IMyClass.OpQStringSReturnValue>
        opQStringSAsync(Queue<string> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQStringSReturnValue(i, i));

        public Task<Test.IMyClass.OpSStringSReturnValue>
        opSStringSAsync(Stack<string> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSStringSReturnValue(i, i));

        public Task<Test.IMyClass.OpAObjectSReturnValue>
        opAObjectSAsync(AnyClass[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAObjectSReturnValue(i, i));

        public Task<Test.IMyClass.OpLObjectSReturnValue>
        opLObjectSAsync(List<AnyClass> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLObjectSReturnValue(i, i));

        public Task<Test.IMyClass.OpAObjectPrxSReturnValue>
        opAObjectPrxSAsync(IObjectPrx[] i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpAObjectPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpLObjectPrxSReturnValue>
        opLObjectPrxSAsync(List<IObjectPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLObjectPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpKObjectPrxSReturnValue>
        opKObjectPrxSAsync(LinkedList<IObjectPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKObjectPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpQObjectPrxSReturnValue>
        opQObjectPrxSAsync(Queue<IObjectPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQObjectPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpSObjectPrxSReturnValue>
        opSObjectPrxSAsync(Stack<IObjectPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSObjectPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpAStructSReturnValue>
        opAStructSAsync(Test.S[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAStructSReturnValue(i, i));

        public Task<Test.IMyClass.OpLStructSReturnValue>
        opLStructSAsync(List<Test.S> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLStructSReturnValue(i, i));

        public Task<Test.IMyClass.OpKStructSReturnValue>
        opKStructSAsync(LinkedList<Test.S> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKStructSReturnValue(i, i));

        public Task<Test.IMyClass.OpQStructSReturnValue>
        opQStructSAsync(Queue<Test.S> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQStructSReturnValue(i, i));

        public Task<Test.IMyClass.OpSStructSReturnValue>
        opSStructSAsync(Stack<Test.S> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSStructSReturnValue(i, i));

        public Task<Test.IMyClass.OpAStructSDReturnValue>
        opAStructSDAsync(Test.SD[] i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpAStructSDReturnValue(i, i));

        public Task<Test.IMyClass.OpLStructSDReturnValue>
        opLStructSDAsync(List<Test.SD> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLStructSDReturnValue(i, i));

        public Task<Test.IMyClass.OpKStructSDReturnValue>
        opKStructSDAsync(LinkedList<Test.SD> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKStructSDReturnValue(i, i));

        public Task<Test.IMyClass.OpQStructSDReturnValue>
        opQStructSDAsync(Queue<Test.SD> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQStructSDReturnValue(i, i));

        public Task<Test.IMyClass.OpSStructSDReturnValue>
        opSStructSDAsync(Stack<Test.SD> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSStructSDReturnValue(i, i));

        public Task<Test.IMyClass.OpACVSReturnValue>
        opACVSAsync(Test.CV[] i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpACVSReturnValue(i, i));

        public Task<Test.IMyClass.OpLCVSReturnValue>
        opLCVSAsync(List<Test.CV> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLCVSReturnValue(i, i));

        public Task<Test.IMyClass.OpAIPrxSReturnValue>
        opAIPrxSAsync(Test.IIPrx[] i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpAIPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpLIPrxSReturnValue>
        opLIPrxSAsync(List<Test.IIPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpLIPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpKIPrxSReturnValue>
        opKIPrxSAsync(LinkedList<Test.IIPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKIPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpQIPrxSReturnValue>
        opQIPrxSAsync(Queue<Test.IIPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpQIPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpSIPrxSReturnValue>
        opSIPrxSAsync(Stack<Test.IIPrx> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSIPrxSReturnValue(i, i));

        public Task<Test.IMyClass.OpACRSReturnValue>
        opACRSAsync(Test.CR[] i, Current current) => Task.FromResult(new Test.IMyClass.OpACRSReturnValue(i, i));

        public Task<Test.IMyClass.OpLCRSReturnValue>
        opLCRSAsync(List<Test.CR> i, Current current) => Task.FromResult(new Test.IMyClass.OpLCRSReturnValue(i, i));

        public Task<Test.IMyClass.OpAEnSReturnValue>
        opAEnSAsync(Test.En[] i, Current current) => Task.FromResult(new Test.IMyClass.OpAEnSReturnValue(i, i));

        public Task<Test.IMyClass.OpLEnSReturnValue>
        opLEnSAsync(List<Test.En> i, Current current) => Task.FromResult(new Test.IMyClass.OpLEnSReturnValue(i, i));

        public Task<Test.IMyClass.OpKEnSReturnValue>
        opKEnSAsync(LinkedList<Test.En> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpKEnSReturnValue(i, i));

        public Task<Test.IMyClass.OpQEnSReturnValue>
        opQEnSAsync(Queue<Test.En> i, Current current) => Task.FromResult(new Test.IMyClass.OpQEnSReturnValue(i, i));

        public Task<Test.IMyClass.OpSEnSReturnValue>
        opSEnSAsync(Stack<Test.En> i, Current current) => Task.FromResult(new Test.IMyClass.OpSEnSReturnValue(i, i));

        public Task<Test.IMyClass.OpCustomIntSReturnValue>
        opCustomIntSAsync(Custom<int> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpCustomIntSReturnValue(i, i));

        public Task<Test.IMyClass.OpCustomCVSReturnValue>
        opCustomCVSAsync(Custom<Test.CV> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpCustomCVSReturnValue(i, i));

        public Task<Test.IMyClass.OpCustomIntSSReturnValue>
        opCustomIntSSAsync(Custom<Custom<int>> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpCustomIntSSReturnValue(i, i));

        public Task<Test.IMyClass.OpCustomCVSSReturnValue>
        opCustomCVSSAsync(Custom<Custom<Test.CV>> i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpCustomCVSSReturnValue(i, i));

        public Task<Test.IMyClass.OpSerialSmallCSharpReturnValue>
        opSerialSmallCSharpAsync(Serialize.Small i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSerialSmallCSharpReturnValue(i, i));

        public Task<Test.IMyClass.OpSerialLargeCSharpReturnValue>
        opSerialLargeCSharpAsync(Serialize.Large i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSerialLargeCSharpReturnValue(i, i));

        public Task<Test.IMyClass.OpSerialStructCSharpReturnValue>
        opSerialStructCSharpAsync(Serialize.Struct i, Current current) =>
            Task.FromResult(new Test.IMyClass.OpSerialStructCSharpReturnValue(i, i));
    }
}
