//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Globalization;
using System.Runtime.Serialization;

namespace IceInternal
{
    public class Ex
    {
        public static void throwUOE(Type expectedType, Ice.AnyClass v)
        {
            //
            // If the object is an unknown sliced object, we didn't find an
            // class factory, in this case raise a NoClassFactoryException
            // instead.
            //
            if (v is Ice.UnknownSlicedClass)
            {
                Ice.UnknownSlicedClass usv = (Ice.UnknownSlicedClass)v;
                throw new Ice.NoClassFactoryException("", usv.ice_id());
            }

            string type = v.ice_id();
            string expected;
            try
            {
                expected = (string)expectedType.GetMethod("ice_staticId").Invoke(null, null);
            }
            catch (Exception)
            {
                expected = "";
                Debug.Assert(false);
            }

            throw new Ice.UnexpectedObjectException("expected element of type `" + expected + "' but received `" +
                                                    type + "'", type, expected);
        }

        public static void throwMemoryLimitException(int requested, int maximum)
        {
            throw new Ice.MemoryLimitException("requested " + requested + " bytes, maximum allowed is " + maximum +
                                               " bytes (see Ice.MessageSizeMax)");
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

        /// <summary>
        /// Initializes a new instance of the exception with serialized data.
        /// </summary>
        /// <param name="info">Holds the serialized object data about the exception being thrown.</param>
        /// <param name="context">Contains contextual information about the source or destination.</param>
        protected Exception(SerializationInfo info, StreamingContext context) : base(info, context) { }

        /// <summary>
        /// Returns the type id of this exception.
        /// </summary>
        /// <returns>The type id of this exception.</returns>
        public abstract string ice_id();
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

        protected virtual IReadOnlyList<SliceInfo>? IceSlicedData
        {
            get => null;
            set => Debug.Assert(false);
        }
        internal IReadOnlyList<SliceInfo>? SlicedData => IceSlicedData;

        public virtual void iceWrite(OutputStream ostr)
        {
            ostr.StartException(null);
            iceWriteImpl(ostr);
            ostr.EndException();
        }

        public virtual bool iceUsesClasses()
        {
            return false;
        }

        protected abstract void iceWriteImpl(OutputStream ostr);

        // Read all the fields of this exception from the stream.
        protected abstract void IceRead(InputStream istr, bool firstSlice);
        internal void Read(InputStream istr) => IceRead(istr, true);
    }

    public static class UserExceptionExtensions
    {
        /// <summary>
        /// During unmarshaling, Ice can slice off derived slices that it does not know how to read, and it can
        /// optionally preserve those "unknown" slices. See the Slice preserve metadata directive.
        /// </summary>
        /// <returns>The list of preserved sliced-off slices.</returns>
        public static IReadOnlyList<SliceInfo>? GetSlicedData(this UserException ex)
        {
            return ex.SlicedData;
        }
    }
}

namespace IceInternal
{
    public class RetryException : Exception
    {
        public RetryException(Ice.LocalException ex)
        {
            _ex = ex;
        }

        public Ice.LocalException get()
        {
            return _ex;
        }

        private Ice.LocalException _ex;
    }
}
