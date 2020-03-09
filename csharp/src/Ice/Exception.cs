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

            string typeId = Ice.TypeExtensions.GetIceTypeId(v.GetType())!;
            string expectedTypeId = Ice.TypeExtensions.GetIceTypeId(expectedType)!;
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
}

namespace IceInternal
{
    public class RetryException : Exception
    {
        internal RetryException(Ice.LocalException ex) => _ex = ex;

        internal Ice.LocalException Get() => _ex;

        private readonly Ice.LocalException _ex;
    }
}
