//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    [System.Serializable]
    public readonly struct ProtocolVersion : System.IEquatable<ProtocolVersion>
    {
        public readonly byte Major;
        public readonly byte Minor;

        public ProtocolVersion(byte major, byte minor)
        {
            Major = major;
            Minor = minor;
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            IceInternal.HashUtil.HashAdd(ref h_, "::Ice::ProtocolVersion");
            IceInternal.HashUtil.HashAdd(ref h_, Major);
            IceInternal.HashUtil.HashAdd(ref h_, Minor);
            return h_;
        }

        public bool Equals(ProtocolVersion other) => Major.Equals(other.Major) && Minor.Equals(other.Minor);

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is ProtocolVersion value && Equals(value);
        }

        public static bool operator ==(ProtocolVersion lhs, ProtocolVersion rhs) => Equals(lhs, rhs);

        public static bool operator !=(ProtocolVersion lhs, ProtocolVersion rhs) => !Equals(lhs, rhs);

    }

    [System.Serializable]
    public readonly struct EncodingVersion : System.IEquatable<EncodingVersion>
    {
        public readonly byte Major;
        public readonly byte Minor;

        public EncodingVersion(byte major, byte minor)
        {
            Major = major;
            Minor = minor;
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            IceInternal.HashUtil.HashAdd(ref h_, "::Ice::EncodingVersion");
            IceInternal.HashUtil.HashAdd(ref h_, Major);
            IceInternal.HashUtil.HashAdd(ref h_, Minor);
            return h_;
        }

        public bool Equals(EncodingVersion other)
        {
            return Major.Equals(other.Major) &&
                Minor.Equals(other.Minor);
        }

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is EncodingVersion value && Equals(value);
        }

        public static bool operator ==(EncodingVersion lhs, EncodingVersion rhs) => Equals(lhs, rhs);

        public static bool operator !=(EncodingVersion lhs, EncodingVersion rhs) => !Equals(lhs, rhs);
    }
}
