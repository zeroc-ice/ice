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
		   java.util.Hashtable ctx,
		   String fs,
		   int md,
		   Ice.Connection[] fixedConns)
    {
    	super(inst, ident, ctx, fs, md);
        _fixedConnections = fixedConns;
    }

    public final Ice.Connection[]
    getFixedConnections()
    {
        return _fixedConnections;
    }

    public boolean
    getSecure()
    {
        return false;
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
    changeTimeout(int newTimeout)
    {
	return this;
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
	 throw new Ice.MarshalException("Cannot marshal a fixed reference");
    }

    public String
    toString()
	throws Ice.MarshalException
    {
	 throw new Ice.MarshalException("Cannot marshal a fixed reference");
    }

    public Ice.Connection
    getConnection()
    {
	//
	// If a reference is secure or the mode is datagram or batch
	// datagram then we throw a NoEndpointException since IceE lacks
	// this support.
	//
	if(getSecure() || getMode() == ModeDatagram || getMode() == ModeBatchDatagram || _fixedConnections.length == 0)
	{
	    if(_fixedConnections.length == 0)
	    {
		Ice.NoEndpointException ex = new Ice.NoEndpointException();
		ex.proxy = toString();
		throw ex;
	    }

	    Ice.FeatureNotSupportedException ex = new Ice.FeatureNotSupportedException();
	    if(getSecure())
	    {
		ex.unsupportedFeature = "ssl";
	    }
	    else if(getMode() == ModeDatagram)
	    {
		ex.unsupportedFeature = "datagram";
	    }
	    else if(getMode() == ModeBatchDatagram)
	    {
		ex.unsupportedFeature = "batch datagram";
	    }
	    throw ex;
	}

	//
	// Choose a random connection
	//
	Ice.Connection connection = _fixedConnections[_rand.nextInt(_fixedConnections.length)];
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(connection != null);
	}

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
	
	return IceUtil.Arrays.equals(_fixedConnections, rhs._fixedConnections);
    }

    protected
    FixedReference()
    {
    }

    protected void
    shallowCopy(FixedReference ref)
    {
	super.shallowCopy(ref);
	ref._fixedConnections = _fixedConnections;
	ref._rand = _rand;
    }

    public java.lang.Object
    ice_clone()
    {
	FixedReference result = new FixedReference();
        shallowCopy(result);
	return result;
    }

    private Ice.Connection _fixedConnections[];
    private java.util.Random _rand = new java.util.Random();
}
