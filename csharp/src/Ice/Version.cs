//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    [System.Serializable]
    public struct ProtocolVersion : System.IEquatable<ProtocolVersion>
    {
        public byte major;
        public byte minor;

        public ProtocolVersion(byte major, byte minor)
        {
            this.major = major;
            this.minor = minor;
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            IceInternal.HashUtil.hashAdd(ref h_, "::Ice::ProtocolVersion");
            IceInternal.HashUtil.hashAdd(ref h_, major);
            IceInternal.HashUtil.hashAdd(ref h_, minor);
            return h_;
        }

        public bool Equals(ProtocolVersion other)
        {
            return major.Equals(other.major) &&
                minor.Equals(other.minor);
        }

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is ProtocolVersion value && this.Equals(value);
        }

        public static bool operator ==(ProtocolVersion lhs, ProtocolVersion rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(ProtocolVersion lhs, ProtocolVersion rhs)
        {
            return !Equals(lhs, rhs);
        }

    }

    [System.Serializable]
    public struct EncodingVersion : System.IEquatable<EncodingVersion>
    {
        public byte major;
        public byte minor;

        public EncodingVersion(byte major, byte minor)
        {
            this.major = major;
            this.minor = minor;
        }

        public override int GetHashCode()
        {
            int h_ = 5381;
            IceInternal.HashUtil.hashAdd(ref h_, "::Ice::EncodingVersion");
            IceInternal.HashUtil.hashAdd(ref h_, major);
            IceInternal.HashUtil.hashAdd(ref h_, minor);
            return h_;
        }

        public bool Equals(EncodingVersion other)
        {
            return major.Equals(other.major) &&
                minor.Equals(other.minor);
        }

        public override bool Equals(object? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is EncodingVersion value && this.Equals(value);
        }

        public static bool operator ==(EncodingVersion lhs, EncodingVersion rhs)
        {
            return Equals(lhs, rhs);
        }

        public static bool operator !=(EncodingVersion lhs, EncodingVersion rhs)
        {
            return !Equals(lhs, rhs);
        }
    }
}
