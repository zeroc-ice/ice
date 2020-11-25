// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>This enumeration describes under what conditions a non-secure connection is allowed.</summary>
    public enum NonSecure
    {
        /// <summary>Never allow non secure connections. This is the default.</summary>
        Never,

        /// <summary>Always allow non secure connections.</summary>
        Always,

        /// <summary>Only allow non secure connections when the peer resides on the same host. What constitutes "same
        /// host" is determined by the transport.</summary>
        SameHost,

        /// <summary>Only allow non secure connections when the peer is a trusted host. What constitutes a "trusted
        /// host" is determined by the transport.</summary>
        TrustedHost
    }
}
