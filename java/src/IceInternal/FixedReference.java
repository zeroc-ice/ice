// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class FixedReference extends Reference
{
    public
    FixedReference(Instance inst,
    		   Ice.Identity ident,
		   java.util.Map ctx,
		   String fs,
		   int md,
		   boolean sec,
		   Ice.ConnectionI[] fixedConns)
    {
    	super(inst, ident, ctx, fs, md, sec);
        _fixedConnections = fixedConns;
    }

    public final Ice.ConnectionI[]
    getFixedConnections()
    {
        return _fixedConnections;
    }

    public Endpoint[]
    getEndpoints()
    {
        return new Endpoint[0];
    }

    public boolean
    getCollocationOptimization()
    {
        return false;
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
         Ice.MarshalException ex = new Ice.MarshalException();
	 ex.reason = "Cannot marshal a fixed reference";
	 throw ex;
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder compress)
    {
        Ice.ConnectionI[] filteredConns = filterConnections(_fixedConnections);
	if(filteredConns.length == 0)
	{
	    Ice.NoEndpointException ex = new Ice.NoEndpointException();
	    ex.proxy = toString();
	    throw ex;
	}

	Ice.ConnectionI connection = filteredConns[0];
	assert(connection != null);
	compress.value = connection.endpoint().compress();

	return connection;
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        return this;
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        return this;
    }

    public Reference
    changeDefault()
    {
        return this;
    }

    public Reference
    changeCollocationOptimization(boolean newCollocationOptimization)
    {
        return this;
    }

    public Reference
    changeCompress(boolean newCompress)
    {
        return this;
    }

    public Reference
    changeTimeout(int newTimeout)
    {
	return this;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
	{
	    return true;
	}
	if(!(obj instanceof FixedReference))
	{
	    return false;
	}
        FixedReference rhs = (FixedReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
	return java.util.Arrays.equals(_fixedConnections, rhs._fixedConnections);
    }

    private Ice.ConnectionI _fixedConnections[];
}
