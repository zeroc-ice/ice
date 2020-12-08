// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice.Test.Enums
{
    public class TestIntf : ITestIntf
    {
        public (ByteEnum, ByteEnum) OpByte(ByteEnum b1, Current current, CancellationToken cancel) => (b1, b1);

        public (ShortEnum, ShortEnum) OpShort(ShortEnum s1, Current current, CancellationToken cancel) => (s1, s1);

        public (IntEnum, IntEnum) OpInt(IntEnum i1, Current current, CancellationToken cancel) => (i1, i1);

        public (SimpleEnum, SimpleEnum) OpSimple(SimpleEnum s1, Current current, CancellationToken cancel) => (s1, s1);

        public (IEnumerable<ByteEnum>, IEnumerable<ByteEnum>) OpByteSeq(
            ByteEnum[] b1,
            Current current,
            CancellationToken cancel) =>
            (b1, b1);

        public (IEnumerable<ShortEnum>, IEnumerable<ShortEnum>) OpShortSeq(
            ShortEnum[] s1,
            Current current,
            CancellationToken cancel) =>
            (s1, s1);

        public (IEnumerable<IntEnum>, IEnumerable<IntEnum>) OpIntSeq(
            IntEnum[] i1,
            Current current,
            CancellationToken cancel) =>
            (i1, i1);

        public (IEnumerable<SimpleEnum>, IEnumerable<SimpleEnum>) OpSimpleSeq(
            SimpleEnum[] s1,
            Current current,
            CancellationToken cancel) =>
            (s1, s1);

        public (FLByteEnum, FLByteEnum) OpFLByte(FLByteEnum b1, Current current, CancellationToken cancel) => (b1, b1);

        public (FLShortEnum, FLShortEnum) OpFLShort(FLShortEnum s1, Current current, CancellationToken cancel) =>
            (s1, s1);

        public (FLUShortEnum, FLUShortEnum) OpFLUShort(FLUShortEnum s1, Current current, CancellationToken cancel) =>
            (s1, s1);

        public (FLIntEnum, FLIntEnum) OpFLInt(FLIntEnum i1, Current current, CancellationToken cancel) => (i1, i1);

        public (FLUIntEnum, FLUIntEnum) OpFLUInt(FLUIntEnum i1, Current current, CancellationToken cancel) => (i1, i1);

        public (FLSimpleEnum, FLSimpleEnum) OpFLSimple(FLSimpleEnum s1, Current current, CancellationToken cancel) =>
            (s1, s1);

        public (ReadOnlyMemory<FLByteEnum> R1, ReadOnlyMemory<FLByteEnum> R2) OpFLByteSeq(
            FLByteEnum[] b1,
            Current current,
            CancellationToken cancel) =>
            (b1, b1);

        public (ReadOnlyMemory<FLShortEnum> R1, ReadOnlyMemory<FLShortEnum> R2) OpFLShortSeq(
            FLShortEnum[] s1,
            Current current,
            CancellationToken cancel) =>
            (s1, s1);

        public (ReadOnlyMemory<FLUShortEnum> R1, ReadOnlyMemory<FLUShortEnum> R2) OpFLUShortSeq(
            FLUShortEnum[] s1,
            Current current, CancellationToken cancel) =>
            (s1, s1);

        public (ReadOnlyMemory<FLIntEnum> R1, ReadOnlyMemory<FLIntEnum> R2) OpFLIntSeq(
            FLIntEnum[] i1,
            Current current,
            CancellationToken cancel) =>
            (i1, i1);

        public (ReadOnlyMemory<FLUIntEnum> R1, ReadOnlyMemory<FLUIntEnum> R2) OpFLUIntSeq(
            FLUIntEnum[] i1,
            Current current,
            CancellationToken cancel) => (i1, i1);

        public (ReadOnlyMemory<FLSimpleEnum> R1, ReadOnlyMemory<FLSimpleEnum> R2) OpFLSimpleSeq(
            FLSimpleEnum[] s1,
            Current current,
            CancellationToken cancel) => (s1, s1);

        public (ByteEnum? R1, ByteEnum? R2) OpTaggedByte(ByteEnum? b1, Current current, CancellationToken cancel) =>
            (b1, b1);

        public (FLByteEnum? R1, FLByteEnum? R2) OpTaggedFLByte(
            FLByteEnum? b1,
            Current current,
            CancellationToken cancel) => (b1, b1);

        public (IEnumerable<ByteEnum>? R1, IEnumerable<ByteEnum>? R2) OpTaggedByteSeq(
            ByteEnum[]? b1,
            Current current,
            CancellationToken cancel) => (b1, b1);

        public (ReadOnlyMemory<FLByteEnum> R1, ReadOnlyMemory<FLByteEnum> R2) OpTaggedFLByteSeq(
            FLByteEnum[]? b1,
            Current current,
            CancellationToken cancel) => (b1, b1);

        public (ReadOnlyMemory<FLIntEnum> R1, ReadOnlyMemory<FLIntEnum> R2) OpTaggedFLIntSeq(
            FLIntEnum[]? i1,
            Current current,
            CancellationToken cancel) => (i1, i1);

        public (MyFlags R1, MyFlags R2) OpMyFlags(MyFlags f1, Current current, CancellationToken cancel) =>
            (f1, f1);

        public (ReadOnlyMemory<MyFlags> R1, ReadOnlyMemory<MyFlags> R2) OpMyFlagsSeq(
            MyFlags[] f1,
            Current current,
            CancellationToken cancel) =>
            (f1, f1);

        public (MyFlags? R1, MyFlags? R2) OpTaggedMyFlags(MyFlags? f1, Current current, CancellationToken cancel) =>
            (f1, f1);

        public (ReadOnlyMemory<MyFlags> R1, ReadOnlyMemory<MyFlags> R2) OpTaggedMyFlagsSeq(
            MyFlags[]? f1,
            Current current,
            CancellationToken cancel) =>
            (f1, f1);

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
