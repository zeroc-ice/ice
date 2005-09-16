// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
		   Ice.Communicator com,
    		   Ice.Identity ident,
		   java.util.Map ctx,
		   String fs,
		   int md,
		   Ice.ConnectionI[] fixedConns)
    {
    	super(inst, com, ident, ctx, fs, md);
        _fixedConnections = fixedConns;
    }

    public final Ice.ConnectionI[]
    getFixedConnections()
    {
        return _fixedConnections;
    }

    public boolean
    getSecure()
    {
	return false;
    }

    public String
    getAdapterId()
    {
	return "";
    }

    public EndpointI[]
    getEndpoints()
    {
        return new EndpointI[0];
    }

    public boolean
    getCollocationOptimization()
    {
        return false;
    }

    public Reference
    changeSecure(boolean sec)
    {
	return this;
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
    changeCollocationOptimization(boolean newCollocationOptimization)
    {
        return this;
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
        return this;
    }

    public Reference
    changeEndpoints(EndpointI[] newEndpoints)
    {
        return this;
    }

    public Reference
    changeCompress(boolean newCompress)
    {
	// TODO: FixedReferences should probably have a _compress flag,
	// that gets its default from the fixed connection this reference
	// refers to. This should be changable with changeCompress(), and
	// reset in changeDefault().
        return this;
    }

    public Reference
    changeTimeout(int newTimeout)
    {
	return this;
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
	 throw new Ice.MarshalException("Cannot marshal a fixed proxy");
    }

    public String
    toString()
	throws Ice.MarshalException
    {
	 throw new Ice.MarshalException("Cannot marshal a fixed proxy");
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
