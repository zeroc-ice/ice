//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace enums
    {
        public class TestI : Test.TestIntf
        {
            public Test.TestIntf.OpByteReturnValue
            opByte(Test.ByteEnum b1, Current current) => new Test.TestIntf.OpByteReturnValue(b1, b1);

            public Test.TestIntf.OpShortReturnValue
            opShort(Test.ShortEnum s1, Current current) => new Test.TestIntf.OpShortReturnValue(s1, s1);

            public Test.TestIntf.OpIntReturnValue
            opInt(Test.IntEnum i1, Current current) => new Test.TestIntf.OpIntReturnValue(i1, i1);

            public Test.TestIntf.OpSimpleReturnValue
            opSimple(Test.SimpleEnum s1, Current current) => new Test.TestIntf.OpSimpleReturnValue(s1, s1);

            public Test.TestIntf.OpByteSeqReturnValue
            opByteSeq(Test.ByteEnum[] b1, Current current) => new Test.TestIntf.OpByteSeqReturnValue(b1, b1);

            public Test.TestIntf.OpShortSeqReturnValue
            opShortSeq(Test.ShortEnum[] s1, Current current) => new Test.TestIntf.OpShortSeqReturnValue(s1, s1);

            public Test.TestIntf.OpIntSeqReturnValue
            opIntSeq(Test.IntEnum[] i1, Current current) => new Test.TestIntf.OpIntSeqReturnValue(i1, i1);

            public Test.TestIntf.OpSimpleSeqReturnValue
            opSimpleSeq(Test.SimpleEnum[] s1, Current current) => new Test.TestIntf.OpSimpleSeqReturnValue(s1, s1);

            public void shutdown(Current current) => current.Adapter.Communicator.shutdown();
        }
    }
}
