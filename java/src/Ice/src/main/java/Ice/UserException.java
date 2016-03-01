// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    __write(OutputStream os)
    {
        os.startException(null);
        __writeImpl(os);
        os.endException();
    }

    public void
    __read(InputStream is)
    {
        is.startException();
        __readImpl(is);
        is.endException(false);
    }

    public boolean
    __usesClasses()
    {
        return false;
    }

    protected abstract void
    __writeImpl(OutputStream os);

    protected abstract void
    __readImpl(InputStream is);
}
