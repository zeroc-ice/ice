// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Base class for exceptions defined in Slice.
/// </summary>
public abstract class UserException : Ice.Exception
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
    /// Parameterless constructor.
    /// </summary>
    protected UserException()
        : base(message: null, innerException: null)
    {
    }

    protected abstract void iceWriteImpl(OutputStream ostr);
    protected abstract void iceReadImpl(InputStream istr);
}
