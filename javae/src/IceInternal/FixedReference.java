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
    		   Ice.Identity ident,
		   java.util.Map ctx,
		   String fs,
		   int md,
		   Ice.ConnectionI[] fixedConns)
    {
    	super(inst, ident, ctx, fs, md);
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
         Ice.MarshalException ex = new Ice.MarshalException();
	 ex.reason = "Cannot marshal a fixed reference";
	 throw ex;
    }

    public Ice.ConnectionI
    getConnection(Ice.BooleanHolder compress)
    {
	if(_fixedConnections.length == 0)
	{
	    Ice.NoEndpointException ex = new Ice.NoEndpointException();
	    ex.proxy = toString();
	    throw ex;
	}

	//
	// Choose a random connection
	//
	Ice.ConnectionI connection = _fixedConnections[_rand.nextInt(_fixedConnections.length)];
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
    private java.util.Random _rand = new java.util.Random();
}
