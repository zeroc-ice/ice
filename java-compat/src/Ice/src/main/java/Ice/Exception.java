// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Base class for Ice local and system exceptions. Those exceptions
 * are not checked so we inherit from java.lang.RuntimeException. User
 * exceptions are checked exceptions and therefore inherit directly
 * from java.lang.Exception.
 **/
public abstract class Exception extends RuntimeException implements Cloneable
{
    public Exception()
    {
    }

    public Exception(Throwable cause)
    {
        super(cause);
    }

    /**
     * Creates a copy of this exception.
     *
     * @return The copy of this exception.
     **/
    @Override
    public Exception clone()
    {
        Exception c = null;

        try
        {
            c = (Exception)super.clone();
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
}
