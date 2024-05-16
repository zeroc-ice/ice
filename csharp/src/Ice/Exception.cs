// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for Ice exceptions.
/// </summary>
public abstract class Exception : System.Exception
{
    /// <summary>
    /// Returns the type id of this exception.
    /// </summary>
    /// <returns>The type id of this exception.</returns>
    public abstract string ice_id();

    /// <summary>
    /// Constructs an Ice exception with an optional inner exception.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    protected Exception(System.Exception? innerException = null) : base("", innerException) { }
}

/// <summary>
/// Base class for Ice run-time exceptions.
/// </summary>
public abstract class LocalException : Exception
{
    /// <summary>
    /// Constructs an Ice local exception with an optional inner exception.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    protected LocalException(System.Exception? innerException = null) : base(innerException) { }
}

/// <summary>
/// Base class for Slice user exceptions.
/// </summary>
public abstract class UserException : Exception
{
    public virtual void iceWrite(OutputStream ostr)
    {
        ostr.startException();
        iceWriteImpl(ostr);
        ostr.endException();
    }

    public virtual void iceRead(InputStream istr)
    {
        istr.startException();
        iceReadImpl(istr);
        istr.endException();
    }

    public virtual bool iceUsesClasses() => false;

    /// <summary>
    /// Constructs an Ice user exception with an optional inner exception.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    protected UserException(System.Exception? innerException = null) : base(innerException) { }

    protected abstract void iceWriteImpl(OutputStream ostr);
    protected abstract void iceReadImpl(InputStream istr);
}
