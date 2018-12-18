// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class for Slice user exceptions.
 **/
public abstract class UserException extends java.lang.Exception implements Cloneable
{
    public UserException()
    {
    }

    public UserException(Throwable cause)
    {
        super(cause);
    }

    /**
     * Creates a copy of this exception.
     *
     * @return The copy of this exception.
     **/
    @Override
    public UserException clone()
    {
        UserException c = null;

        try
        {
            c = (UserException)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false;
        }
        return c;
    }

    /**
     * Returns the name of this exception.
     *
     * @return The name of this exception.
     *
     * @deprecated ice_name() is deprecated, use ice_id() instead.
     **/
    @Deprecated
    public String
    ice_name()
    {
        return ice_id().substring(2);
    }

    /**
     * Returns the type id of this exception.
     *
     * @return The type id of this exception.
     **/
    public abstract String
    ice_id();

    /**
     * Returns the sliced data if the exception has a preserved-slice base class and has been sliced during
     * un-marshaling, null is returned otherwise.
     *
     * @return The sliced data or null.
     **/
    public Ice.SlicedData
    ice_getSlicedData()
    {
        return null;
    }

    /**
     * Returns a string representation of this exception.
     *
     * @return A string representation of this exception.
     **/
    @Override
    public String
    toString()
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
        out.setUseTab(false);
        out.print(getClass().getName());
        out.inc();
        IceInternal.ValueWriter.write(this, out);
        pw.flush();
        return sw.toString();
    }

    public void
    _write(OutputStream os)
    {
        os.startException(null);
        _writeImpl(os);
        os.endException();
    }

    public void
    _read(InputStream is)
    {
        is.startException();
        _readImpl(is);
        is.endException(false);
    }

    public boolean
    _usesClasses()
    {
        return false;
    }

    protected abstract void
    _writeImpl(OutputStream os);

    protected abstract void
    _readImpl(InputStream is);
}
