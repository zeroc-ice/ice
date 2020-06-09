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

        public (FLByteEnum ReturnValue, FLByteEnum b2) OpFLByte(FLByteEnum b1, Current current) => (b1, b1);

        public (FLShortEnum ReturnValue, FLShortEnum s2) OpFLShort(FLShortEnum s1, Current current) => (s1, s1);

        public (FLUShortEnum ReturnValue, FLUShortEnum s2) OpFLUShort(FLUShortEnum s1, Current current) => (s1, s1);

        public (FLIntEnum ReturnValue, FLIntEnum i2) OpFLInt(FLIntEnum i1, Current current) => (i1, i1);

        public (FLUIntEnum ReturnValue, FLUIntEnum i2) OpFLUInt(FLUIntEnum i1, Current current) => (i1, i1);

        public (FLSimpleEnum ReturnValue, FLSimpleEnum s2) OpFLSimple(FLSimpleEnum s1, Current current) => (s1, s1);

        public (ReadOnlyMemory<FLByteEnum> ReturnValue, ReadOnlyMemory<FLByteEnum> b2) OpFLByteSeq(
            FLByteEnum[] b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLShortEnum> ReturnValue, ReadOnlyMemory<FLShortEnum> s2) OpFLShortSeq(
            FLShortEnum[] s1,
            Current current) => (s1, s1);

        public (ReadOnlyMemory<FLUShortEnum> ReturnValue, ReadOnlyMemory<FLUShortEnum> s2) OpFLUShortSeq(
            FLUShortEnum[] s1,
            Current current) => (s1, s1);

        public (ReadOnlyMemory<FLIntEnum> ReturnValue, ReadOnlyMemory<FLIntEnum> i2) OpFLIntSeq(
            FLIntEnum[] i1,
            Current current) => (i1, i1);

        public (ReadOnlyMemory<FLUIntEnum> ReturnValue, ReadOnlyMemory<FLUIntEnum> i2) OpFLUIntSeq(
            FLUIntEnum[] i1,
            Current current) => (i1, i1);

        public (ReadOnlyMemory<FLSimpleEnum> ReturnValue, ReadOnlyMemory<FLSimpleEnum> s2) OpFLSimpleSeq(
            FLSimpleEnum[] s1,
            Current current) => (s1, s1);

        public (ByteEnum? ReturnValue, ByteEnum? b2) OpTaggedByte(ByteEnum? b1, Current current) => (b1, b1);

        public (FLByteEnum? ReturnValue, FLByteEnum? b2) OpTaggedFLByte(FLByteEnum? b1, Current current) => (b1, b1);

        public (IEnumerable<ByteEnum>? ReturnValue, IEnumerable<ByteEnum>? b2) OpTaggedByteSeq(
            ByteEnum[]? b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLByteEnum> ReturnValue, ReadOnlyMemory<FLByteEnum> b2) OpTaggedFLByteSeq(
            FLByteEnum[]? b1,
            Current current) => (b1, b1);

        public (ReadOnlyMemory<FLIntEnum> ReturnValue, ReadOnlyMemory<FLIntEnum> i2) OpTaggedFLIntSeq(
            FLIntEnum[]? i1,
            Current current) => (i1, i1);

        public void Shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
