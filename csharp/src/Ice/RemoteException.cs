//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace Ice
{
    /// <summary>Base class for exceptions that can be transmitted in responses to Ice requests. The derived exception
    /// classes are generated from exceptions defined in Slice.</summary>
    [Serializable]
    public class RemoteException : Exception
    {
        public override string Message => _hasCustomMessage || DefaultMessage == null ? base.Message : DefaultMessage;

        /// <summary>When true, if this exception is thrown from the implementation of an operation, Ice will convert
        /// it into an Ice.UnhandledException. When false, Ice marshals this remote exception as-is. true is the
        /// default for exceptions unmarshaled by Ice, while false is the default for exceptions that did not originate
        /// in a remote server.</summary>
        public bool ConvertToUnhandled { get; set; } = false;

        /// <summary>When DefaultMessage is not null and the application does construct the exception with a constructor
        /// that takes a message parameter, Message returns DefaultMessage. This property should be overridden in
        /// derived partial exception classes that provide a custom default message.</summary>
        protected virtual string? DefaultMessage => null;

        protected SlicedData? IceSlicedData { get; set; }
        internal SlicedData? SlicedData => IceSlicedData;

        private readonly bool _hasCustomMessage = false;

        internal RemoteException(SlicedData slicedData)
        {
            IceSlicedData = slicedData;
            ConvertToUnhandled = true;
        }

        /// <summary>Constructs a remote exception with the default system message.</summary>
        protected RemoteException()
        {
        }

        /// <summary>Constructs a remote exception with the provided message.</summary>
        /// <param name="message">Message that describes the exception.</param>
        protected RemoteException(string message)
            : base(message) => _hasCustomMessage = true;

        /// <summary>Constructs a remote exception with the provided message and inner exception.</summary>
        /// <param name="message">Message that describes the exception.</param>
        /// <param name="innerException">The inner exception.</param>
        protected RemoteException(string message, Exception innerException)
            : base(message, innerException) => _hasCustomMessage = true;

        /// <summary>Initializes a new instance of the remote exception with serialized data.</summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected RemoteException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }

        // See OutputStream
        protected virtual void IceWrite(OutputStream ostr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            // This implementation can only be called on a plain RemoteException with IceSlicedData set.
            Debug.Assert(IceSlicedData.HasValue);
            ostr.WriteSlicedData(IceSlicedData.Value);
        }
        internal void Write(OutputStream ostr) => IceWrite(ostr, true);
    }

    public static class RemoteExceptionExtensions
    {
        /// <summary>During unmarshaling, Ice slices off derived slices that it does not know how to read, and preserves
        /// these "unknown" slices.</summary>
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this RemoteException ex) => ex.SlicedData;
    }
}
