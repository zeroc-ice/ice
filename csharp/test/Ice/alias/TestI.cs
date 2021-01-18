// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Alias
{
    public sealed class Interface2 : IInterface2
    {
        public void Shutdown(ZeroC.Ice.Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();

        public (bool R1, bool R2, bool? R3)
        OpBool(bool p1, bool p2, bool? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (byte R1, byte R2, byte? R3)
        OpByte(byte p1, byte p2, byte? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (short R1, short R2, short? R3)
        OpShort(short p1, short p2, short? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (ushort R1, ushort R2, ushort? R3)
        OpUshort(ushort p1, ushort p2, ushort? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (int R1, int R2, int? R3)
        OpInt(int p1, int p2, int? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (uint R1, uint R2, uint? R3)
        OpUint(uint p1, uint p2, uint? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (int R1, int R2, int? R3)
        OpVarint(int p1, int p2, int? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (uint R1, uint R2, uint? R3)
        OpVaruint(uint p1, uint p2, uint? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (long R1, long R2, long? R3)
        OpLong(long p1, long p2, long? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (ulong R1, ulong R2, ulong? R3)
        OpUlong(ulong p1, ulong p2, ulong? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (long R1, long R2, long? R3)
        OpVarlong(long p1, long p2, long? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (ulong R1, ulong R2, ulong? R3)
        OpVarulong(ulong p1, ulong p2, ulong? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (float R1, float R2, float? R3)
        OpFloat(float p1, float p2, float? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (double R1, double R2, double? R3)
        OpDouble(double p1, double p2, double? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (string R1, string R2, string? R3)
        OpString(string p1, string p2, string? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (Struct1 R1, Struct1 R2, Struct1? R3)
        OpStruct1(Struct1 p1, Struct1 p2, Struct1? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (Class1 R1, Class1 R2, Class1? R3)
        OpClass1(Class1 p1, Class1 p2, Class1? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (IInterface1Prx R1, IInterface1Prx R2, IInterface1Prx? R3)
        OpInterface1(IInterface1Prx p1, IInterface1Prx p2, IInterface1Prx? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (Enum1 R1, Enum1 R2, Enum1? R3)
        OpEnum1(Enum1 p1, Enum1 p2, Enum1? p3, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3);

        public (IEnumerable<string> R1, IEnumerable<string> R2, IEnumerable<string> R3, IEnumerable<string>? R4)
        OpStringSeq(string[] p1, string[] p2, string[] p3, string[]? p4, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3, p4);

        public (IReadOnlyDictionary<string, int> R1, IReadOnlyDictionary<string, int> R2, IReadOnlyDictionary<string, int> R3, IReadOnlyDictionary<string, int>? R4)
        OpStringIntDict(Dictionary<string, int> p1, Dictionary<string, int> p2, Dictionary<string, int> p3, Dictionary<string, int>? p4, ZeroC.Ice.Current current, CancellationToken cancel)
            => (p1, p2, p3, p4);
    }
}
