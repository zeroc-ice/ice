//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    [System.Serializable]
    public readonly struct Protocol : System.IEquatable<Protocol>
    {
        public readonly byte Major;
        public readonly byte Minor;

        public Protocol(byte major, byte minor)
        {
            Major = major;
            Minor = minor;
        }

        public override int GetHashCode() => System.HashCode.Combine(Major, Minor);

        public bool Equals(Protocol other) => Major.Equals(other.Major) && Minor.Equals(other.Minor);

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is Protocol value && Equals(value);
        }

        public static bool operator ==(Protocol lhs, Protocol rhs) => Equals(lhs, rhs);

        public static bool operator !=(Protocol lhs, Protocol rhs) => !Equals(lhs, rhs);

        // TODO: move these static methods somewhere else (or remove them).

        internal static void CheckSupportedProtocol(Protocol v)
        {
            if (v.Major != Ice1Definitions.ProtocolMajor || v.Minor > Ice1Definitions.ProtocolMinor)
            {
                throw new UnsupportedProtocolException("", v, Util.CurrentProtocol);
            }
        }

        internal static void CheckSupportedProtocolEncoding(Encoding v)
        {
            if (v.Major != Ice1Definitions.ProtocolEncodingMajor || v.Minor > Ice1Definitions.ProtocolEncodingMinor)
            {
                throw new UnsupportedEncodingException("", v, Util.CurrentProtocolEncoding);
            }
        }

        internal static bool IsSupported(Protocol version, Protocol supported) =>
            version.Major == supported.Major && version.Minor <= supported.Minor;
    }
}
