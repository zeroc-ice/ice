// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

// TODO: should we simply generate this code from a Slice enum?

namespace ZeroC.Ice
{
    /// <summary>The proxy's invocation mode.</summary>
    public enum InvocationMode : byte
    {
        /// <summary>This is the default invocation mode, a request using this mode always expects a response.</summary>
        Twoway,
        /// <summary>A request using oneway mode returns control to the application code as soon as it has been accepted
        /// by the local transport.</summary>
        Oneway,
        /// <summary>The batch oneway invocation mode is no longer supported, it was supported with Ice versions up to
        /// 3.7.</summary>
        [Obsolete("The batch oneway invocation mode is no longer supported")]
        BatchOneway,
        /// <summary>Invocation mode used by datagram based transports.</summary>
        Datagram,
        /// <summary>The batch datagram invocation mode is no longer supported, it was supported with Ice versions up to
        /// 3.7.</summary>
        [Obsolete("The batch datagram invocation mode is no longer supported")]
        BatchDatagram,
    }

    public static class InvocationModeHelper
    {
        public static InvocationMode AsInvocationMode(this byte value) =>
            0 <= value && value <= 4 ? (InvocationMode)value :
                throw new InvalidDataException($"invalid enumerator value `{value}' for Ice.InvocationMode");

        public static InvocationMode ReadInvocationMode(this InputStream istr) =>
            AsInvocationMode(istr.ReadByte());

    }
}
