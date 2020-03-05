//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace Ice
{
    [System.Serializable]
    public readonly struct Encoding : System.IEquatable<Encoding>
    {
        // The encodings known to the Ice runtime. Don't reference them from other static variables!

        public static readonly Encoding V1_0 = new Encoding(1, 0);
        public static readonly Encoding V1_1 = new Encoding(1, 1);
        public static readonly Encoding V2_0 = new Encoding(2, 0);

        public readonly byte Major;
        public readonly byte Minor;

        public static Encoding Parse(string str)
        {
            (byte major, byte minor) = Util.ParseMajorMinorVersion(str, throwOnFailure: true)!.Value;
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

        internal void CheckSupported()
        {
            // For now, we claim to support 1.0 and 1.1.
            // TODO: add 2.0, remove 1.0
            if (this != V1_0 && this != V1_1)
            {
                throw new UnsupportedEncodingException("", this, Util.CurrentEncoding);
            }
        }
    }
}
