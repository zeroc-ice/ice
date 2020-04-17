//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.enums
{
    public class TestIntf : Test.ITestIntf
    {
        public (Test.ByteEnum, Test.ByteEnum)
        opByte(Test.ByteEnum b1, Current current) => (b1, b1);

        public (Test.ShortEnum, Test.ShortEnum)
        opShort(Test.ShortEnum s1, Current current) => (s1, s1);

        public (Test.IntEnum, Test.IntEnum)
        opInt(Test.IntEnum i1, Current current) => (i1, i1);

        public (Test.SimpleEnum, Test.SimpleEnum)
        opSimple(Test.SimpleEnum s1, Current current) => (s1, s1);

        public (IEnumerable<Test.ByteEnum>, IEnumerable<Test.ByteEnum>)
        opByteSeq(Test.ByteEnum[] b1, Current current) => (b1, b1);

        public (IEnumerable<Test.ShortEnum>, IEnumerable<Test.ShortEnum>)
        opShortSeq(Test.ShortEnum[] s1, Current current) => (s1, s1);

        public (IEnumerable<Test.IntEnum>, IEnumerable<Test.IntEnum>)
        opIntSeq(Test.IntEnum[] i1, Current current) => (i1, i1);

        public (IEnumerable<Test.SimpleEnum>, IEnumerable<Test.SimpleEnum>)
        opSimpleSeq(Test.SimpleEnum[] s1, Current current) => (s1, s1);

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
