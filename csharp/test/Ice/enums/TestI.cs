//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace ZeroC.Ice.Test.Enums
{
    public class TestIntf : ITestIntf
    {
        public (ByteEnum, ByteEnum)
        opByte(ByteEnum b1, Current current) => (b1, b1);

        public (ShortEnum, ShortEnum)
        opShort(ShortEnum s1, Current current) => (s1, s1);

        public (IntEnum, IntEnum)
        opInt(IntEnum i1, Current current) => (i1, i1);

        public (SimpleEnum, SimpleEnum)
        opSimple(SimpleEnum s1, Current current) => (s1, s1);

        public (IEnumerable<ByteEnum>, IEnumerable<ByteEnum>)
        opByteSeq(ByteEnum[] b1, Current current) => (b1, b1);

        public (IEnumerable<ShortEnum>, IEnumerable<ShortEnum>)
        opShortSeq(ShortEnum[] s1, Current current) => (s1, s1);

        public (IEnumerable<IntEnum>, IEnumerable<IntEnum>)
        opIntSeq(IntEnum[] i1, Current current) => (i1, i1);

        public (IEnumerable<SimpleEnum>, IEnumerable<SimpleEnum>)
        opSimpleSeq(SimpleEnum[] s1, Current current) => (s1, s1);

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
