// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for exceptions defined in Slice.
/// </summary>
public abstract class UserException : Ice.Exception
{
    /// <summary>
    /// Writes the exception to the output stream.
    /// </summary>
    /// <param name="ostr">The output stream to write to.</param>
    public virtual void iceWrite(OutputStream ostr)
    {
        ostr.startException();
        iceWriteImpl(ostr);
        ostr.endException();
    }

    /// <summary>
    /// Reads the exception from the input stream.
    /// </summary>
    /// <param name="istr">The input stream to read from.</param>
    public virtual void iceRead(InputStream istr)
    {
        istr.startException();
        iceReadImpl(istr);
        istr.endException();
    }

    /// <summary>
    /// Returns true if the exception uses classes.
    /// </summary>
    /// <returns><c>true</c> if the exception utilizes classes; otherwise, <c>false</c>.</returns>
    public virtual bool iceUsesClasses() => false;

    /// <summary>
    /// Initializes a new instance of the <see cref="UserException"/> class.
    /// </summary>
    protected UserException()
        : base(message: null, innerException: null)
    {
    }

    protected abstract void iceWriteImpl(OutputStream ostr);

    protected abstract void iceReadImpl(InputStream istr);
}
