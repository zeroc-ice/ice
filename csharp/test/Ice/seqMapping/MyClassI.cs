//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.seqMapping
{
    public sealed class MyClass : Test.IMyClass
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public Test.IMyClass.OpAByteSReturnValue
        opAByteS(byte[] i, Current current) => new Test.IMyClass.OpAByteSReturnValue(i, i);

        public Test.IMyClass.OpLByteSReturnValue
        opLByteS(List<byte> i, Current current) => new Test.IMyClass.OpLByteSReturnValue(i, i);

        public Test.IMyClass.OpKByteSReturnValue
        opKByteS(LinkedList<byte> i, Current current) => new Test.IMyClass.OpKByteSReturnValue(i, i);

        public Test.IMyClass.OpQByteSReturnValue
        opQByteS(Queue<byte> i, Current current) => new Test.IMyClass.OpQByteSReturnValue(i, i);

        public Test.IMyClass.OpSByteSReturnValue
        opSByteS(Stack<byte> i, Current current) => new Test.IMyClass.OpSByteSReturnValue(i, i);

        public Test.IMyClass.OpABoolSReturnValue
        opABoolS(bool[] i, Current current) => new Test.IMyClass.OpABoolSReturnValue(i, i);

        public Test.IMyClass.OpLBoolSReturnValue
        opLBoolS(List<bool> i, Current current) => new Test.IMyClass.OpLBoolSReturnValue(i, i);

        public Test.IMyClass.OpKBoolSReturnValue
        opKBoolS(LinkedList<bool> i, Current current) => new Test.IMyClass.OpKBoolSReturnValue(i, i);

        public Test.IMyClass.OpQBoolSReturnValue
        opQBoolS(Queue<bool> i, Current current) => new Test.IMyClass.OpQBoolSReturnValue(i, i);

        public Test.IMyClass.OpSBoolSReturnValue
        opSBoolS(Stack<bool> i, Current current) => new Test.IMyClass.OpSBoolSReturnValue(i, i);

        public Test.IMyClass.OpAShortSReturnValue
        opAShortS(short[] i, Current current) => new Test.IMyClass.OpAShortSReturnValue(i, i);

        public Test.IMyClass.OpLShortSReturnValue
        opLShortS(List<short> i, Current current) => new Test.IMyClass.OpLShortSReturnValue(i, i);

        public Test.IMyClass.OpKShortSReturnValue
        opKShortS(LinkedList<short> i, Current current) => new Test.IMyClass.OpKShortSReturnValue(i, i);

        public Test.IMyClass.OpQShortSReturnValue
        opQShortS(Queue<short> i, Current current) => new Test.IMyClass.OpQShortSReturnValue(i, i);

        public Test.IMyClass.OpSShortSReturnValue
        opSShortS(Stack<short> i, Current current) => new Test.IMyClass.OpSShortSReturnValue(i, i);

        public Test.IMyClass.OpAIntSReturnValue
        opAIntS(int[] i, Current current) => new Test.IMyClass.OpAIntSReturnValue(i, i);

        public Test.IMyClass.OpLIntSReturnValue
        opLIntS(List<int> i, Current current) => new Test.IMyClass.OpLIntSReturnValue(i, i);

        public Test.IMyClass.OpKIntSReturnValue
        opKIntS(LinkedList<int> i, Current current) => new Test.IMyClass.OpKIntSReturnValue(i, i);

        public Test.IMyClass.OpQIntSReturnValue
        opQIntS(Queue<int> i, Current current) => new Test.IMyClass.OpQIntSReturnValue(i, i);

        public Test.IMyClass.OpSIntSReturnValue
        opSIntS(Stack<int> i, Current current) => new Test.IMyClass.OpSIntSReturnValue(i, i);

        public Test.IMyClass.OpALongSReturnValue
        opALongS(long[] i, Current current) => new Test.IMyClass.OpALongSReturnValue(i, i);

        public Test.IMyClass.OpLLongSReturnValue
        opLLongS(List<long> i, Current current) => new Test.IMyClass.OpLLongSReturnValue(i, i);

        public Test.IMyClass.OpKLongSReturnValue
        opKLongS(LinkedList<long> i, Current current) => new Test.IMyClass.OpKLongSReturnValue(i, i);

        public Test.IMyClass.OpQLongSReturnValue
        opQLongS(Queue<long> i, Current current) => new Test.IMyClass.OpQLongSReturnValue(i, i);

        public Test.IMyClass.OpSLongSReturnValue
        opSLongS(Stack<long> i, Current current) => new Test.IMyClass.OpSLongSReturnValue(i, i);

        public Test.IMyClass.OpAFloatSReturnValue
        opAFloatS(float[] i, Current current) => new Test.IMyClass.OpAFloatSReturnValue(i, i);

        public Test.IMyClass.OpLFloatSReturnValue
        opLFloatS(List<float> i, Current current) => new Test.IMyClass.OpLFloatSReturnValue(i, i);

        public Test.IMyClass.OpKFloatSReturnValue
        opKFloatS(LinkedList<float> i, Current current) => new Test.IMyClass.OpKFloatSReturnValue(i, i);

        public Test.IMyClass.OpQFloatSReturnValue
        opQFloatS(Queue<float> i, Current current) => new Test.IMyClass.OpQFloatSReturnValue(i, i);

        public Test.IMyClass.OpSFloatSReturnValue
        opSFloatS(Stack<float> i, Current current) => new Test.IMyClass.OpSFloatSReturnValue(i, i);

        public Test.IMyClass.OpADoubleSReturnValue
        opADoubleS(double[] i, Current current) => new Test.IMyClass.OpADoubleSReturnValue(i, i);

        public Test.IMyClass.OpLDoubleSReturnValue
        opLDoubleS(List<double> i, Current current) => new Test.IMyClass.OpLDoubleSReturnValue(i, i);

        public Test.IMyClass.OpKDoubleSReturnValue
        opKDoubleS(LinkedList<double> i, Current current) => new Test.IMyClass.OpKDoubleSReturnValue(i, i);

        public Test.IMyClass.OpQDoubleSReturnValue
        opQDoubleS(Queue<double> i, Current current) => new Test.IMyClass.OpQDoubleSReturnValue(i, i);

        public Test.IMyClass.OpSDoubleSReturnValue
        opSDoubleS(Stack<double> i, Current current) => new Test.IMyClass.OpSDoubleSReturnValue(i, i);

        public Test.IMyClass.OpAStringSReturnValue
        opAStringS(string[] i, Current current) => new Test.IMyClass.OpAStringSReturnValue(i, i);

        public Test.IMyClass.OpLStringSReturnValue
        opLStringS(List<string> i, Current current) => new Test.IMyClass.OpLStringSReturnValue(i, i);

        public Test.IMyClass.OpKStringSReturnValue
        opKStringS(LinkedList<string> i, Current current) => new Test.IMyClass.OpKStringSReturnValue(i, i);

        public Test.IMyClass.OpQStringSReturnValue
        opQStringS(Queue<string> i, Current current) => new Test.IMyClass.OpQStringSReturnValue(i, i);

        public Test.IMyClass.OpSStringSReturnValue
        opSStringS(Stack<string> i, Current current) => new Test.IMyClass.OpSStringSReturnValue(i, i);

        public Test.IMyClass.OpAObjectSReturnValue
        opAObjectS(AnyClass[] i, Current current) => new Test.IMyClass.OpAObjectSReturnValue(i, i);

        public Test.IMyClass.OpLObjectSReturnValue
        opLObjectS(List<AnyClass> i, Current current) => new Test.IMyClass.OpLObjectSReturnValue(i, i);

        public Test.IMyClass.OpAObjectPrxSReturnValue
        opAObjectPrxS(IObjectPrx[] i, Current current) => new Test.IMyClass.OpAObjectPrxSReturnValue(i, i);

        public Test.IMyClass.OpLObjectPrxSReturnValue
        opLObjectPrxS(List<IObjectPrx> i, Current current) => new Test.IMyClass.OpLObjectPrxSReturnValue(i, i);

        public Test.IMyClass.OpKObjectPrxSReturnValue
        opKObjectPrxS(LinkedList<IObjectPrx> i, Current current) => new Test.IMyClass.OpKObjectPrxSReturnValue(i, i);

        public Test.IMyClass.OpQObjectPrxSReturnValue
        opQObjectPrxS(Queue<IObjectPrx> i, Current current) => new Test.IMyClass.OpQObjectPrxSReturnValue(i, i);

        public Test.IMyClass.OpSObjectPrxSReturnValue
        opSObjectPrxS(Stack<IObjectPrx> i, Current current) => new Test.IMyClass.OpSObjectPrxSReturnValue(i, i);

        public Test.IMyClass.OpAStructSReturnValue
        opAStructS(Test.S[] i, Current current) => new Test.IMyClass.OpAStructSReturnValue(i, i);

        public Test.IMyClass.OpLStructSReturnValue
        opLStructS(List<Test.S> i, Current current) => new Test.IMyClass.OpLStructSReturnValue(i, i);

        public Test.IMyClass.OpKStructSReturnValue
        opKStructS(LinkedList<Test.S> i, Current current) => new Test.IMyClass.OpKStructSReturnValue(i, i);

        public Test.IMyClass.OpQStructSReturnValue
        opQStructS(Queue<Test.S> i, Current current) => new Test.IMyClass.OpQStructSReturnValue(i, i);

        public Test.IMyClass.OpSStructSReturnValue
        opSStructS(Stack<Test.S> i, Current current) => new Test.IMyClass.OpSStructSReturnValue(i, i);

        public Test.IMyClass.OpAStructSDReturnValue
        opAStructSD(Test.SD[] i, Current current) => new Test.IMyClass.OpAStructSDReturnValue(i, i);

        public Test.IMyClass.OpLStructSDReturnValue
        opLStructSD(List<Test.SD> i, Current current) => new Test.IMyClass.OpLStructSDReturnValue(i, i);

        public Test.IMyClass.OpKStructSDReturnValue
        opKStructSD(LinkedList<Test.SD> i, Current current) => new Test.IMyClass.OpKStructSDReturnValue(i, i);

        public Test.IMyClass.OpQStructSDReturnValue
        opQStructSD(Queue<Test.SD> i, Current current) => new Test.IMyClass.OpQStructSDReturnValue(i, i);

        public Test.IMyClass.OpSStructSDReturnValue
        opSStructSD(Stack<Test.SD> i, Current current) => new Test.IMyClass.OpSStructSDReturnValue(i, i);

        public Test.IMyClass.OpACVSReturnValue
        opACVS(Test.CV[] i, Current current) => new Test.IMyClass.OpACVSReturnValue(i, i);

        public Test.IMyClass.OpLCVSReturnValue
        opLCVS(List<Test.CV> i, Current current) => new Test.IMyClass.OpLCVSReturnValue(i, i);

        public Test.IMyClass.OpACRSReturnValue
        opACRS(Test.CR[] i, Current current) => new Test.IMyClass.OpACRSReturnValue(i, i);

        public Test.IMyClass.OpLCRSReturnValue
        opLCRS(List<Test.CR> i, Current current) => new Test.IMyClass.OpLCRSReturnValue(i, i);

        public Test.IMyClass.OpAEnSReturnValue
        opAEnS(Test.En[] i, Current current) => new Test.IMyClass.OpAEnSReturnValue(i, i);

        public Test.IMyClass.OpLEnSReturnValue
        opLEnS(List<Test.En> i, Current current) => new Test.IMyClass.OpLEnSReturnValue(i, i);

        public Test.IMyClass.OpKEnSReturnValue
        opKEnS(LinkedList<Test.En> i, Current current) => new Test.IMyClass.OpKEnSReturnValue(i, i);

        public Test.IMyClass.OpQEnSReturnValue
        opQEnS(Queue<Test.En> i, Current current) => new Test.IMyClass.OpQEnSReturnValue(i, i);

        public Test.IMyClass.OpSEnSReturnValue
        opSEnS(Stack<Test.En> i, Current current) => new Test.IMyClass.OpSEnSReturnValue(i, i);

        public Test.IMyClass.OpAIPrxSReturnValue
        opAIPrxS(Test.IIPrx[] i, Current current) => new Test.IMyClass.OpAIPrxSReturnValue(i, i);

        public Test.IMyClass.OpLIPrxSReturnValue
        opLIPrxS(List<Test.IIPrx> i, Current current) => new Test.IMyClass.OpLIPrxSReturnValue(i, i);

        public Test.IMyClass.OpKIPrxSReturnValue
        opKIPrxS(LinkedList<Test.IIPrx> i, Current current) => new Test.IMyClass.OpKIPrxSReturnValue(i, i);

        public Test.IMyClass.OpQIPrxSReturnValue
        opQIPrxS(Queue<Test.IIPrx> i, Current current) => new Test.IMyClass.OpQIPrxSReturnValue(i, i);

        public Test.IMyClass.OpSIPrxSReturnValue
        opSIPrxS(Stack<Test.IIPrx> i, Current current) => new Test.IMyClass.OpSIPrxSReturnValue(i, i);

        public Test.IMyClass.OpCustomIntSReturnValue
        opCustomIntS(Custom<int> i, Current current) => new Test.IMyClass.OpCustomIntSReturnValue(i, i);

        public Test.IMyClass.OpCustomCVSReturnValue
        opCustomCVS(Custom<Test.CV> i, Current current) => new Test.IMyClass.OpCustomCVSReturnValue(i, i);

        public Test.IMyClass.OpCustomIntSSReturnValue
        opCustomIntSS(Custom<Custom<int>> i, Current current) => new Test.IMyClass.OpCustomIntSSReturnValue(i, i);

        public Test.IMyClass.OpCustomCVSSReturnValue
        opCustomCVSS(Custom<Custom<Test.CV>> i, Current current) => new Test.IMyClass.OpCustomCVSSReturnValue(i, i);

        public Test.IMyClass.OpSerialSmallCSharpReturnValue
        opSerialSmallCSharp(Serialize.Small i, Current current) => new Test.IMyClass.OpSerialSmallCSharpReturnValue(i, i);

        public Test.IMyClass.OpSerialLargeCSharpReturnValue
        opSerialLargeCSharp(Serialize.Large i, Current current) => new Test.IMyClass.OpSerialLargeCSharpReturnValue(i, i);

        public Test.IMyClass.OpSerialStructCSharpReturnValue
        opSerialStructCSharp(Serialize.Struct i, Current current) => new Test.IMyClass.OpSerialStructCSharpReturnValue(i, i);
    }
}
