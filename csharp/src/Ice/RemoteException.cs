// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;

namespace ZeroC.Ice
{
    /// <summary>Base class for exceptions that can be transmitted in responses to Ice requests. The derived exception
    /// classes are generated from exceptions defined in Slice.</summary>
    public class RemoteException : Exception
    {
        /// <inheritdoc/>
        public override string Message => _hasCustomMessage || DefaultMessage == null ? base.Message : DefaultMessage;

        /// <summary>When true, if this exception is thrown from the implementation of an operation, Ice will convert
        /// it into an Ice.UnhandledException. When false, Ice marshals this remote exception as-is. true is the
        /// default for exceptions unmarshaled by Ice, while false is the default for exceptions that did not originate
        /// in a remote server.</summary>
        public bool ConvertToUnhandled { get; set; }

        /// <summary>When DefaultMessage is not null and the application does construct the exception with a constructor
        /// that takes a message parameter, Message returns DefaultMessage. This property should be overridden in
        /// derived partial exception classes that provide a custom default message.</summary>
        protected virtual string? DefaultMessage => null;

        /// <summary>Returns the sliced data if the exception has a preserved-slice base exception and has been sliced during
        /// un-marshaling, <c>null</c> is returned otherwise.</summary>
        protected SlicedData? IceSlicedData { get; set; }
        internal SlicedData? SlicedData => IceSlicedData;

        private readonly bool _hasCustomMessage;

        /// <summary>Constructs a remote exception with the provided message.</summary>
        /// <param name="message">Message that describes the exception.</param>
        protected internal RemoteException(string? message)
            : base(message) => _hasCustomMessage = message != null;

        /// <summary>Constructs a remote exception with the default system message.</summary>
        protected RemoteException()
        {
        }

        /// <summary>Constructs a remote exception with the provided message and inner exception.</summary>
        /// <param name="message">Message that describes the exception.</param>
        /// <param name="innerException">The inner exception.</param>
        protected RemoteException(string? message, Exception? innerException)
            : base(message, innerException) => _hasCustomMessage = message != null;

        /// <summary>Unmarshals a remote exception from the <see cref="InputStream"/>. This base implementation is only
        /// called on a plain RemoteException.</summary>
        /// <param name="istr">The <see cref="InputStream"/> to read from.</param>
        /// <param name="firstSlice"><c>True</c> if the exception corresponds to the first Slice, <c>False</c>
        /// otherwise.</param>
        protected virtual void IceRead(InputStream istr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            IceSlicedData = istr.SlicedData;
            ConvertToUnhandled = true;
        }

        internal void Read(InputStream istr) => IceRead(istr, true);

        /// <summary>Marshal a remote exception to the <see cref="OutputStream"/>. This implementation can only be
        /// called on a plain RemoteException with IceSlicedData set.</summary>
        /// <param name="ostr">The <see cref="OutputStream"/> to marshal the exception.</param>
        /// <param name="firstSlice"><c>True</c> if the exception corresponds to the first Slice, <c>False</c>
        /// otherwise.</param>
        protected virtual void IceWrite(OutputStream ostr, bool firstSlice) =>
            ostr.WriteSlicedData(IceSlicedData!.Value, Array.Empty<string>(), Message);

        internal void Write(OutputStream ostr) => IceWrite(ostr, true);
    }

    /// <summary>Provides public extensions methods for RemoteException instances.</summary>
    public static class RemoteExceptionExtensions
    {
        /// <summary>During unmarshaling, Ice slices off derived slices that it does not know how to read, and preserves
        /// these "unknown" slices.</summary>
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this RemoteException ex) => ex.SlicedData;
    }
}
