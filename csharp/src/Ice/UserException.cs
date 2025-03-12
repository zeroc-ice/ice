// Copyright (c) ZeroC, Inc.

#nullable enable

using System.ComponentModel;

namespace Ice;

/// <summary>
/// Base class for exceptions defined in Slice.
/// </summary>
public abstract class UserException : Ice.Exception
{
    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual void iceWrite(OutputStream ostr)
    {
        ostr.startException();
        iceWriteImpl(ostr);
        ostr.endException();
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual void iceRead(InputStream istr)
    {
        istr.startException();
        iceReadImpl(istr);
        istr.endException();
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual bool iceUsesClasses() => false;

    /// <summary>
    /// Initializes a new instance of the <see cref="UserException"/> class.
    /// </summary>
    protected UserException()
        : base(message: null, innerException: null)
    {
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected abstract void iceWriteImpl(OutputStream ostr);

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected abstract void iceReadImpl(InputStream istr);
}
