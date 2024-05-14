// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Globalization;

namespace Ice;

/// <summary>
/// Base class for Ice exceptions.
/// </summary>
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
    /// Returns the type id of this exception.
    /// </summary>
    /// <returns>The type id of this exception.</returns>
    public abstract string ice_id();

    /// <summary>
    /// Returns a string representation of this exception, including
    /// any inner exceptions.
    /// </summary>
    /// <returns>The string representation of this exception.</returns>
    public override string ToString()
    {
        //
        // This prints the exception Java style. That is, the outermost
        // exception, "Caused by:" to the innermost exception. The
        // stack trace is not nicely indented as with Java, but
        // without string parsing (perhaps tokenize on "\n"), it
        // doesn't appear to be possible to reformat it.
        //
        System.IO.StringWriter sw = new System.IO.StringWriter(CultureInfo.CurrentCulture);
        Ice.UtilInternal.OutputBase op = new Ice.UtilInternal.OutputBase(sw);
        op.setUseTab(false);
        op.print(GetType().FullName);
        op.inc();
        Ice.Internal.ValueWriter.write(this, op);
        sw.Write("\n");
        sw.Write(StackTrace);

        System.Exception curr = InnerException;
        while (curr != null)
        {
            sw.Write("\nCaused by: ");
            sw.Write(curr.GetType().FullName);
            if (!(curr is Ice.Exception))
            {
                sw.Write(": ");
                sw.Write(curr.Message);
            }
            sw.Write("\n");
            sw.Write(curr.StackTrace);
            curr = curr.InnerException;
        }

        return sw.ToString();
    }
}

/// <summary>
/// Base class for Ice run-time exceptions.
/// </summary>
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
}

/// <summary>
/// Base class for Slice user exceptions.
/// </summary>
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

    public virtual bool iceUsesClasses()
    {
        return false;
    }

    protected abstract void iceWriteImpl(OutputStream ostr);
    protected abstract void iceReadImpl(InputStream istr);
}
