//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Runtime.Serialization;

namespace IceInternal
{
    internal class Ex
    {
        // UOE = UnknownObjectException as in unknow class. TODO: fix/remove.
        internal static void ThrowUOE(Type expectedType, Ice.AnyClass v)
        {
            // If the object is an unknown sliced object, we didn't find an
            // class factory, in this case raise a NoClassFactoryException
            // instead.
            if (v is Ice.UnknownSlicedClass usv)
            {
                throw new Ice.NoClassFactoryException("", usv.TypeId ?? "");
            }

            string typeId = Ice.TypeIdAttribute.GetTypeId(v.GetType())!;
            string expectedTypeId = Ice.TypeIdAttribute.GetTypeId(expectedType)!;
            throw new Ice.UnexpectedObjectException(
                $"expected element of type `{expectedTypeId}' but received `{typeId}'", typeId, expectedTypeId);
        }

        internal static void ThrowMemoryLimitException(int requested, int maximum)
        {
            throw new Ice.MemoryLimitException(
                $"requested {requested} bytes, maximum allowed is {maximum} bytes (see Ice.MessageSizeMax)");
        }
    }
}

namespace Ice
{
    /// <summary>
    /// Base class for Ice exceptions.
    /// </summary>
    [Serializable]
    public abstract class Exception : System.Exception
    {
        /// <summary>
        /// Creates a default-initialized exception.
        /// </summary>
        public Exception() { }

        /// <summary>
        /// Creates a default-initialized exception and sets the InnerException
        /// property to the passed exception.
        /// </summary>
        /// <param name="ex">The inner exception.</param>
        public Exception(System.Exception ex) : base("", ex) { }

        // TODO: temporary, should be removed:
        public abstract string ice_id();

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected Exception(SerializationInfo info, StreamingContext context) : base(info, context) { }
    }

    /// <summary>
    /// Base class for Ice run-time exceptions.
    /// </summary>
    [Serializable]
    public abstract class LocalException : Exception
    {
        /// <summary>
        /// Creates a default-initialized Ice run-time exception.
        /// </summary>
        public LocalException() { }

        /// <summary>
        /// Creates a default-initialized Ice run-time exception and sets the InnerException
        /// property to the passed exception.
        /// </summary>
        /// <param name="ex">The inner exception.</param>
        public LocalException(System.Exception ex) : base(ex) { }

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected LocalException(SerializationInfo info, StreamingContext context) : base(info, context) { }

    }

    /// <summary>
    /// Base class for Ice system exceptions.
    /// Ice system exceptions are currently Ice internal, non-documented exceptions.
    /// </summary>
    [Serializable]
    public abstract class SystemException : Exception
    {
        /// <summary>
        /// Creates a default-initialized system exception.
        /// </summary>
        public SystemException() { }

        /// <summary>
        /// Creates a default-initialized system exception and sets the InnerException
        /// property to the passed exception.
        /// </summary>
        /// <param name="ex">The inner exception.</param>
        public SystemException(System.Exception ex) : base(ex) { }

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected SystemException(SerializationInfo info, StreamingContext context) : base(info, context) { }
    }

    /// <summary>
    /// Base class for Slice user exceptions.
    /// </summary>
    [Serializable]
    public abstract class UserException : Exception
    {
        protected virtual SlicedData? IceSlicedData
        {
            get => null;
            set => Debug.Assert(false);
        }

        internal SlicedData? SlicedData => IceSlicedData;

        /// <summary>
        /// Creates a default-initialized user exception.
        /// </summary>
        public UserException() { }

        /// <summary>
        /// Creates a default-initialized user exception and sets the InnerException
        /// property to the passed exception.
        /// </summary>
        /// <param name="ex">The inner exception.</param>
        public UserException(System.Exception ex) : base(ex) { }

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected UserException(SerializationInfo info, StreamingContext context) : base(info, context) { }
        public virtual bool IceUsesClasses() => false;

        // TODO: remove during exception refactoring
        public override string ice_id()
        {
            return TypeIdAttribute.GetTypeId(GetType())!;
        }

        // See InputStream.
        protected abstract void IceRead(InputStream istr, bool firstSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);

        // See OuputStream
        protected abstract void IceWrite(OutputStream ostr, bool firstSlice);
        internal void Write(OutputStream ostr) => IceWrite(ostr, true);
    }

    public static class UserExceptionExtensions
    {
        /// <summary>
        /// During unmarshaling, Ice can slice off derived slices that it does not know how to read, and it can
        /// optionally preserve those "unknown" slices. See the Slice preserve metadata directive.
        /// </summary>
        /// <returns>A SlicedData value that provides the list of sliced-off slices.</returns>
        public static SlicedData? GetSlicedData(this UserException ex) => ex.SlicedData;
    }
}

namespace IceInternal
{
    internal class RetryException : Exception
    {
        internal RetryException(Ice.LocalException ex) => _ex = ex;

        internal Ice.LocalException Get() => _ex;

        private readonly Ice.LocalException _ex;
    }
}
