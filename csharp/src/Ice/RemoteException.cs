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
    public class RemoteException : System.Exception
    {
        /// <summary>When true, if this exception is thrown from the implementation of an operation, Ice will convert
        /// it into an Ice.UnhandledException. When false, Ice marshals this remote exception as-is. true is the
        /// default for exceptions unmarshaled by Ice, while false is the default for exceptions that did not originate
        /// in a remote server.</summary>
        public bool ConvertToUnhandled { get; set; } = false;

        protected SlicedData? IceSlicedData { get; set; }
        internal SlicedData? SlicedData => IceSlicedData;

        internal RemoteException(SlicedData slicedData) => IceSlicedData = slicedData;

        /// <summary>Creates a default-initialized remote exception.</summary>
        protected RemoteException()
        {
        }

        /// <summary>Creates a default-initialized remote exception and sets the InnerException property to the passed
        /// exception.</summary>
        /// <param name="innerException">The inner exception.</param>
        protected RemoteException(System.Exception innerException)
            : base("", innerException)
        {
        }

        /// <summary>Initializes a new instance of the remote exception with serialized data.</summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected RemoteException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }

        // See InputStream.
        protected virtual void IceRead(InputStream istr, bool firstSlice)
        {
            // If this exception is a plain RemoteException, IceSlicedData is set using the internal RemoteException
            // constructor. As a result, this base method implementation is never called.
            Debug.Assert(false);
        }
        internal void Read(InputStream istr) => IceRead(istr, true);

        // See OutputStream
        protected virtual void IceWrite(OutputStream ostr, bool firstSlice)
        {
            Debug.Assert(firstSlice);
            // This implementation can only be called on a plain RemoteException with IceSlicedData set.
            Debug.Assert(IceSlicedData.HasValue);
            if (!ostr.WriteSlicedData(IceSlicedData.Value))
            {
                 // Most derived type Id for this exception
                string typeId = IceSlicedData.Value.Slices[0].TypeId!;
                throw new MarshalException(
                    $"Failed to marshal a fully sliced {nameof(RemoteException)} with type ID `{typeId}'");
            }
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
