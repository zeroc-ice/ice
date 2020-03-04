//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    [System.Serializable]
    public readonly struct Encoding : System.IEquatable<Encoding>
    {
        public readonly byte Major;
        public readonly byte Minor;

        public static Encoding Parse(string str)
        {
            (byte major, byte minor) = Util.ParseMajorMinorVersion(str, throwOnFailure: true).Value;
            return new Encoding(major, minor);
        }
        public static bool TryParse(string str, out Encoding encoding)
        {
            var result = Util.ParseMajorMinorVersion(str, throwOnFailure: false);
            if (result != null)
            {
                encoding = new Encoding(result.Value.Major, result.Value.Minor);
            }
            else
            {
                encoding = default;
            }
            return result != null;
        }

        public static bool operator ==(Encoding lhs, Encoding rhs) => Equals(lhs, rhs);
        public static bool operator !=(Encoding lhs, Encoding rhs) => !Equals(lhs, rhs);

        public Encoding(byte major, byte minor)
        {
            Major = major;
            Minor = minor;
        }

        public override int GetHashCode() => System.HashCode.Combine(Major, Minor);

        public bool Equals(Encoding other) => Major.Equals(other.Major) && Minor.Equals(other.Minor);

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is Encoding value && Equals(value);
        }

        public override string ToString() => string.Format("{0}.{1}", Major, Minor);
    }
}
