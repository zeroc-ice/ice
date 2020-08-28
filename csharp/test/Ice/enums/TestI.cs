//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice.Test.Enums
{
    public class TestIntf : ITestIntf
    {
        public (ByteEnum, ByteEnum) OpByte(ByteEnum b1, Current current) => (b1, b1);

        public (ShortEnum, ShortEnum) OpShort(ShortEnum s1, Current current) => (s1, s1);

        public (IntEnum, IntEnum) OpInt(IntEnum i1, Current current) => (i1, i1);

        public (SimpleEnum, SimpleEnum) OpSimple(SimpleEnum s1, Current current) => (s1, s1);

        public (IEnumerable<ByteEnum>, IEnumerable<ByteEnum>) OpByteSeq(ByteEnum[] b1, Current current) =>
            (b1, b1);

        public (IEnumerable<ShortEnum>, IEnumerable<ShortEnum>) OpShortSeq(ShortEnum[] s1, Current current) =>
            (s1, s1);

        public (IEnumerable<IntEnum>, IEnumerable<IntEnum>) OpIntSeq(IntEnum[] i1, Current current) => (i1, i1);

        public (IEnumerable<SimpleEnum>, IEnumerable<SimpleEnum>) OpSimpleSeq(SimpleEnum[] s1, Current current) =>
            (s1, s1);

        public (FLByteEnum ReturnValue, FLByteEnum B2) OpFLByte(FLByteEnum b1, Current current) => (b1, b1);

        public (FLShortEnum ReturnValue, FLShortEnum S2) OpFLShort(FLShortEnum s1, Current current) => (s1, s1);

        public (FLUShortEnum ReturnValue, FLUShortEnum S2) OpFLUShort(FLUShortEnum s1, Current current) => (s1, s1);

        public (FLIntEnum ReturnValue, FLIntEnum I2) OpFLInt(FLIntEnum i1, Current current) => (i1, i1);

        public (FLUIntEnum ReturnValue, FLUIntEnum I2) OpFLUInt(FLUIntEnum i1, Current current) => (i1, i1);

        public (FLSimpleEnum ReturnValue, FLSimpleEnum S2) OpFLSimple(FLSimpleEnum s1, Current current) => (s1, s1);

        public (ReadOnlyMemory<FLByteEnum> ReturnValue, ReadOnlyMemory<FLByteEnum> B2) OpFLByteSeq(
            FLByteEnum[] b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLShortEnum> ReturnValue, ReadOnlyMemory<FLShortEnum> S2) OpFLShortSeq(
            FLShortEnum[] s1,
            Current current) => (s1, s1);

        public (ReadOnlyMemory<FLUShortEnum> ReturnValue, ReadOnlyMemory<FLUShortEnum> S2) OpFLUShortSeq(
            FLUShortEnum[] s1,
            Current current) => (s1, s1);

        public (ReadOnlyMemory<FLIntEnum> ReturnValue, ReadOnlyMemory<FLIntEnum> I2) OpFLIntSeq(
            FLIntEnum[] i1,
            Current current) => (i1, i1);

        public (ReadOnlyMemory<FLUIntEnum> ReturnValue, ReadOnlyMemory<FLUIntEnum> I2) OpFLUIntSeq(
            FLUIntEnum[] i1,
            Current current) => (i1, i1);

        public (ReadOnlyMemory<FLSimpleEnum> ReturnValue, ReadOnlyMemory<FLSimpleEnum> S2) OpFLSimpleSeq(
            FLSimpleEnum[] s1,
            Current current) => (s1, s1);

        public (ByteEnum? ReturnValue, ByteEnum? B2) OpTaggedByte(ByteEnum? b1, Current current) => (b1, b1);

        public (FLByteEnum? ReturnValue, FLByteEnum? B2) OpTaggedFLByte(FLByteEnum? b1, Current current) => (b1, b1);

        public (IEnumerable<ByteEnum>? ReturnValue, IEnumerable<ByteEnum>? B2) OpTaggedByteSeq(
            ByteEnum[]? b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLByteEnum> ReturnValue, ReadOnlyMemory<FLByteEnum> B2) OpTaggedFLByteSeq(
            FLByteEnum[]? b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLIntEnum> ReturnValue, ReadOnlyMemory<FLIntEnum> I2) OpTaggedFLIntSeq(
            FLIntEnum[]? i1,
            Current current) => (i1, i1);

        public (MyFlags ReturnValue, MyFlags F2) OpMyFlags(MyFlags f1, Current current) =>
            (f1, f1);

        public (ReadOnlyMemory<MyFlags> ReturnValue, ReadOnlyMemory<MyFlags> F2) OpMyFlagsSeq(
            MyFlags[] f1,
            Current current) => (f1, f1);

        public (MyFlags? ReturnValue, MyFlags? F2) OpTaggedMyFlags(MyFlags? f1, Current current) => (f1, f1);

        public (ReadOnlyMemory<MyFlags> ReturnValue, ReadOnlyMemory<MyFlags> F2) OpTaggedMyFlagsSeq(
            MyFlags[]? f1,
            Current current) => (f1, f1);

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
    }
}
