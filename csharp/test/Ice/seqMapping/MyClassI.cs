//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace seqMapping
    {
        public sealed class MyClassI : Test.MyClass
        {
            public void shutdown(Ice.Current current)
            {
                current.Adapter.Communicator.shutdown();
            }

            public Test.MyClass.OpAByteSReturnValue
            opAByteS(byte[] i, Current current) => new Test.MyClass.OpAByteSReturnValue(i, i);

            public Test.MyClass.OpLByteSReturnValue
            opLByteS(List<byte> i, Current current) => new Test.MyClass.OpLByteSReturnValue(i, i);

            public Test.MyClass.OpKByteSReturnValue
            opKByteS(LinkedList<byte> i, Current current) => new Test.MyClass.OpKByteSReturnValue(i, i);

            public Test.MyClass.OpQByteSReturnValue
            opQByteS(Queue<byte> i, Current current) => new Test.MyClass.OpQByteSReturnValue(i, i);

            public Test.MyClass.OpSByteSReturnValue
            opSByteS(Stack<byte> i, Current current) => new Test.MyClass.OpSByteSReturnValue(i, i);

            public Test.MyClass.OpABoolSReturnValue
            opABoolS(bool[] i, Current current) => new Test.MyClass.OpABoolSReturnValue(i, i);

            public Test.MyClass.OpLBoolSReturnValue
            opLBoolS(List<bool> i, Current current) => new Test.MyClass.OpLBoolSReturnValue(i, i);

            public Test.MyClass.OpKBoolSReturnValue
            opKBoolS(LinkedList<bool> i, Current current) => new Test.MyClass.OpKBoolSReturnValue(i, i);

            public Test.MyClass.OpQBoolSReturnValue
            opQBoolS(Queue<bool> i, Current current) => new Test.MyClass.OpQBoolSReturnValue(i, i);

            public Test.MyClass.OpSBoolSReturnValue
            opSBoolS(Stack<bool> i, Current current) => new Test.MyClass.OpSBoolSReturnValue(i, i);

            public Test.MyClass.OpAShortSReturnValue
            opAShortS(short[] i, Current current) => new Test.MyClass.OpAShortSReturnValue(i, i);

            public Test.MyClass.OpLShortSReturnValue
            opLShortS(List<short> i, Current current) => new Test.MyClass.OpLShortSReturnValue(i, i);

            public Test.MyClass.OpKShortSReturnValue
            opKShortS(LinkedList<short> i, Current current) => new Test.MyClass.OpKShortSReturnValue(i, i);

            public Test.MyClass.OpQShortSReturnValue
            opQShortS(Queue<short> i, Current current) => new Test.MyClass.OpQShortSReturnValue(i, i);

            public Test.MyClass.OpSShortSReturnValue
            opSShortS(Stack<short> i, Current current) => new Test.MyClass.OpSShortSReturnValue(i, i);

            public Test.MyClass.OpAIntSReturnValue
            opAIntS(int[] i, Current current) => new Test.MyClass.OpAIntSReturnValue(i, i);

            public Test.MyClass.OpLIntSReturnValue
            opLIntS(List<int> i, Current current) => new Test.MyClass.OpLIntSReturnValue(i, i);

            public Test.MyClass.OpKIntSReturnValue
            opKIntS(LinkedList<int> i, Current current) => new Test.MyClass.OpKIntSReturnValue(i, i);

            public Test.MyClass.OpQIntSReturnValue
            opQIntS(Queue<int> i, Current current) => new Test.MyClass.OpQIntSReturnValue(i, i);

            public Test.MyClass.OpSIntSReturnValue
            opSIntS(Stack<int> i, Current current) => new Test.MyClass.OpSIntSReturnValue(i, i);

            public Test.MyClass.OpALongSReturnValue
            opALongS(long[] i, Current current) => new Test.MyClass.OpALongSReturnValue(i, i);

            public Test.MyClass.OpLLongSReturnValue
            opLLongS(List<long> i, Current current) => new Test.MyClass.OpLLongSReturnValue(i, i);

            public Test.MyClass.OpKLongSReturnValue
            opKLongS(LinkedList<long> i, Current current) => new Test.MyClass.OpKLongSReturnValue(i, i);

            public Test.MyClass.OpQLongSReturnValue
            opQLongS(Queue<long> i, Current current) => new Test.MyClass.OpQLongSReturnValue(i, i);

            public Test.MyClass.OpSLongSReturnValue
            opSLongS(Stack<long> i, Current current) => new Test.MyClass.OpSLongSReturnValue(i, i);

            public Test.MyClass.OpAFloatSReturnValue
            opAFloatS(float[] i, Current current) => new Test.MyClass.OpAFloatSReturnValue(i, i);

            public Test.MyClass.OpLFloatSReturnValue
            opLFloatS(List<float> i, Current current) => new Test.MyClass.OpLFloatSReturnValue(i, i);

            public Test.MyClass.OpKFloatSReturnValue
            opKFloatS(LinkedList<float> i, Current current) => new Test.MyClass.OpKFloatSReturnValue(i, i);

            public Test.MyClass.OpQFloatSReturnValue
            opQFloatS(Queue<float> i, Current current) => new Test.MyClass.OpQFloatSReturnValue(i, i);

            public Test.MyClass.OpSFloatSReturnValue
            opSFloatS(Stack<float> i, Current current) => new Test.MyClass.OpSFloatSReturnValue(i, i);

            public Test.MyClass.OpADoubleSReturnValue
            opADoubleS(double[] i, Current current) => new Test.MyClass.OpADoubleSReturnValue(i, i);

            public Test.MyClass.OpLDoubleSReturnValue
            opLDoubleS(List<double> i, Current current) => new Test.MyClass.OpLDoubleSReturnValue(i, i);

            public Test.MyClass.OpKDoubleSReturnValue
            opKDoubleS(LinkedList<double> i, Current current) => new Test.MyClass.OpKDoubleSReturnValue(i, i);

            public Test.MyClass.OpQDoubleSReturnValue
            opQDoubleS(Queue<double> i, Current current) => new Test.MyClass.OpQDoubleSReturnValue(i, i);

            public Test.MyClass.OpSDoubleSReturnValue
            opSDoubleS(Stack<double> i, Current current) => new Test.MyClass.OpSDoubleSReturnValue(i, i);

            public Test.MyClass.OpAStringSReturnValue
            opAStringS(string[] i, Current current) => new Test.MyClass.OpAStringSReturnValue(i, i);

            public Test.MyClass.OpLStringSReturnValue
            opLStringS(List<string> i, Current current) => new Test.MyClass.OpLStringSReturnValue(i, i);

            public Test.MyClass.OpKStringSReturnValue
            opKStringS(LinkedList<string> i, Current current) => new Test.MyClass.OpKStringSReturnValue(i, i);

            public Test.MyClass.OpQStringSReturnValue
            opQStringS(Queue<string> i, Current current) => new Test.MyClass.OpQStringSReturnValue(i, i);

            public Test.MyClass.OpSStringSReturnValue
            opSStringS(Stack<string> i, Current current) => new Test.MyClass.OpSStringSReturnValue(i, i);

            public Test.MyClass.OpAObjectSReturnValue
            opAObjectS(Value[] i, Current current) => new Test.MyClass.OpAObjectSReturnValue(i, i);

            public Test.MyClass.OpLObjectSReturnValue
            opLObjectS(List<Value> i, Current current) => new Test.MyClass.OpLObjectSReturnValue(i, i);

            public Test.MyClass.OpAObjectPrxSReturnValue
            opAObjectPrxS(IObjectPrx[] i, Current current) => new Test.MyClass.OpAObjectPrxSReturnValue(i, i);

            public Test.MyClass.OpLObjectPrxSReturnValue
            opLObjectPrxS(List<IObjectPrx> i, Current current) => new Test.MyClass.OpLObjectPrxSReturnValue(i, i);

            public Test.MyClass.OpKObjectPrxSReturnValue
            opKObjectPrxS(LinkedList<IObjectPrx> i, Current current) => new Test.MyClass.OpKObjectPrxSReturnValue(i, i);

            public Test.MyClass.OpQObjectPrxSReturnValue
            opQObjectPrxS(Queue<IObjectPrx> i, Current current) => new Test.MyClass.OpQObjectPrxSReturnValue(i, i);

            public Test.MyClass.OpSObjectPrxSReturnValue
            opSObjectPrxS(Stack<IObjectPrx> i, Current current) => new Test.MyClass.OpSObjectPrxSReturnValue(i, i);

            public Test.MyClass.OpAStructSReturnValue
            opAStructS(Test.S[] i, Current current) => new Test.MyClass.OpAStructSReturnValue(i, i);

            public Test.MyClass.OpLStructSReturnValue
            opLStructS(List<Test.S> i, Current current) => new Test.MyClass.OpLStructSReturnValue(i, i);

            public Test.MyClass.OpKStructSReturnValue
            opKStructS(LinkedList<Test.S> i, Current current) => new Test.MyClass.OpKStructSReturnValue(i, i);

            public Test.MyClass.OpQStructSReturnValue
            opQStructS(Queue<Test.S> i, Current current) => new Test.MyClass.OpQStructSReturnValue(i, i);

            public Test.MyClass.OpSStructSReturnValue
            opSStructS(Stack<Test.S> i, Current current) => new Test.MyClass.OpSStructSReturnValue(i, i);

            public Test.MyClass.OpAStructSDReturnValue
            opAStructSD(Test.SD[] i, Current current) => new Test.MyClass.OpAStructSDReturnValue(i, i);

            public Test.MyClass.OpLStructSDReturnValue
            opLStructSD(List<Test.SD> i, Current current) => new Test.MyClass.OpLStructSDReturnValue(i, i);

            public Test.MyClass.OpKStructSDReturnValue
            opKStructSD(LinkedList<Test.SD> i, Current current) => new Test.MyClass.OpKStructSDReturnValue(i, i);

            public Test.MyClass.OpQStructSDReturnValue
            opQStructSD(Queue<Test.SD> i, Current current) => new Test.MyClass.OpQStructSDReturnValue(i, i);

            public Test.MyClass.OpSStructSDReturnValue
            opSStructSD(Stack<Test.SD> i, Current current) => new Test.MyClass.OpSStructSDReturnValue(i, i);

            public Test.MyClass.OpACVSReturnValue
            opACVS(Test.CV[] i, Current current) => new Test.MyClass.OpACVSReturnValue(i, i);

            public Test.MyClass.OpLCVSReturnValue
            opLCVS(List<Test.CV> i, Current current) => new Test.MyClass.OpLCVSReturnValue(i, i);

            public Test.MyClass.OpACRSReturnValue
            opACRS(Test.CR[] i, Current current) => new Test.MyClass.OpACRSReturnValue(i, i);

            public Test.MyClass.OpLCRSReturnValue
            opLCRS(List<Test.CR> i, Current current) => new Test.MyClass.OpLCRSReturnValue(i, i);

            public Test.MyClass.OpAEnSReturnValue
            opAEnS(Test.En[] i, Current current) => new Test.MyClass.OpAEnSReturnValue(i, i);

            public Test.MyClass.OpLEnSReturnValue
            opLEnS(List<Test.En> i, Current current) => new Test.MyClass.OpLEnSReturnValue(i, i);

            public Test.MyClass.OpKEnSReturnValue
            opKEnS(LinkedList<Test.En> i, Current current) => new Test.MyClass.OpKEnSReturnValue(i, i);

            public Test.MyClass.OpQEnSReturnValue
            opQEnS(Queue<Test.En> i, Current current) => new Test.MyClass.OpQEnSReturnValue(i, i);

            public Test.MyClass.OpSEnSReturnValue
            opSEnS(Stack<Test.En> i, Current current) => new Test.MyClass.OpSEnSReturnValue(i, i);

            public Test.MyClass.OpAIPrxSReturnValue
            opAIPrxS(Test.IPrx[] i, Current current) => new Test.MyClass.OpAIPrxSReturnValue(i, i);

            public Test.MyClass.OpLIPrxSReturnValue
            opLIPrxS(List<Test.IPrx> i, Current current) => new Test.MyClass.OpLIPrxSReturnValue(i, i);

            public Test.MyClass.OpKIPrxSReturnValue
            opKIPrxS(LinkedList<Test.IPrx> i, Current current) => new Test.MyClass.OpKIPrxSReturnValue(i, i);

            public Test.MyClass.OpQIPrxSReturnValue
            opQIPrxS(Queue<Test.IPrx> i, Current current) => new Test.MyClass.OpQIPrxSReturnValue(i, i);

            public Test.MyClass.OpSIPrxSReturnValue
            opSIPrxS(Stack<Test.IPrx> i, Current current) => new Test.MyClass.OpSIPrxSReturnValue(i, i);

            public Test.MyClass.OpCustomIntSReturnValue
            opCustomIntS(Custom<int> i, Current current) => new Test.MyClass.OpCustomIntSReturnValue(i, i);

            public Test.MyClass.OpCustomCVSReturnValue
            opCustomCVS(Custom<Test.CV> i, Current current) => new Test.MyClass.OpCustomCVSReturnValue(i, i);

            public Test.MyClass.OpCustomIntSSReturnValue
            opCustomIntSS(Custom<Custom<int>> i, Current current) => new Test.MyClass.OpCustomIntSSReturnValue(i, i);

            public Test.MyClass.OpCustomCVSSReturnValue
            opCustomCVSS(Custom<Custom<Test.CV>> i, Current current) => new Test.MyClass.OpCustomCVSSReturnValue(i, i);

            public Test.MyClass.OpSerialSmallCSharpReturnValue
            opSerialSmallCSharp(Serialize.Small i, Current current) => new Test.MyClass.OpSerialSmallCSharpReturnValue(i, i);

            public Test.MyClass.OpSerialLargeCSharpReturnValue
            opSerialLargeCSharp(Serialize.Large i, Current current) => new Test.MyClass.OpSerialLargeCSharpReturnValue(i, i);

            public Test.MyClass.OpSerialStructCSharpReturnValue
            opSerialStructCSharp(Serialize.Struct i, Current current) => new Test.MyClass.OpSerialStructCSharpReturnValue(i, i);
        }
    }
}
