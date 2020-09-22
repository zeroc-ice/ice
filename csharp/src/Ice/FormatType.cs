// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>This enumeration describes the possible formats for classes and exceptions. As of Ice 4.0,
    /// exceptions are always marshaled in the Sliced format, but can be received in the Compact format from
    /// applications using earlier versions of Ice.</summary>
    public enum FormatType
    {
        /// <summary>The Compact format assumes the sender and receiver have the same Slice definitions for classes
        /// and exceptions. If an application receives a derived class or exception it does not know, it is
        /// not capable to unmarshal it into a known base class or exception because there is not enough information
        /// in the encoded payload. The Compact format is the default for classes.</summary>
        Compact,

        /// <summary>The Sliced format allows slicing of unknown slices by the receiver. If an application receives
        /// a derived class or exception it does not know, it can slice off the derived bits and create a base class
        /// or exception. Exceptions are always marshaled using the Sliced format.</summary>
        Sliced
    }
}
