using IceInternal;
using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    using Context = Dictionary<string, string>;

    // TODO: replace IceInternal.ReplyStatus
    /*
    public enum ReplyStatus : byte // Ice1 reply status
    {
        Ok = 0,
        UserException = 1,
        ObjectNotExistException = 2,
        FacetNotExistException = 3,
        OperationNotExistException = 4,
        UnknownLocalException = 5,
        UnknownUserException = 6,
        UnknownException = 7
    }
    */

    public sealed class IncomingResponseFrame : InputStream
    {
        public byte ReplyStatus { get; } = 0; // only meaningful with Ice1
        public Context? Context { get; } // The response context, always null with Ice1

        internal IncomingResponseFrame(Communicator communicator, EncodingVersion encoding)
            : base(communicator, encoding)
        {
        }
    }
}
