//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.enums
{
    public class TestIntf : Test.ITestIntf
    {
        public Test.ITestIntf.OpByteReturnValue
        opByte(Test.ByteEnum b1, Current current) => new Test.ITestIntf.OpByteReturnValue(b1, b1);

        public Test.ITestIntf.OpShortReturnValue
        opShort(Test.ShortEnum s1, Current current) => new Test.ITestIntf.OpShortReturnValue(s1, s1);

        public Test.ITestIntf.OpIntReturnValue
        opInt(Test.IntEnum i1, Current current) => new Test.ITestIntf.OpIntReturnValue(i1, i1);

        public Test.ITestIntf.OpSimpleReturnValue
        opSimple(Test.SimpleEnum s1, Current current) => new Test.ITestIntf.OpSimpleReturnValue(s1, s1);

        public Test.ITestIntf.OpByteSeqReturnValue
        opByteSeq(Test.ByteEnum[] b1, Current current) => new Test.ITestIntf.OpByteSeqReturnValue(b1, b1);

        public Test.ITestIntf.OpShortSeqReturnValue
        opShortSeq(Test.ShortEnum[] s1, Current current) => new Test.ITestIntf.OpShortSeqReturnValue(s1, s1);

        public Test.ITestIntf.OpIntSeqReturnValue
        opIntSeq(Test.IntEnum[] i1, Current current) => new Test.ITestIntf.OpIntSeqReturnValue(i1, i1);

        public Test.ITestIntf.OpSimpleSeqReturnValue
        opSimpleSeq(Test.SimpleEnum[] s1, Current current) => new Test.ITestIntf.OpSimpleSeqReturnValue(s1, s1);

        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
    }
}
