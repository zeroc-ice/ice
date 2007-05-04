// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class FixedReference extends Reference
{
    public
    FixedReference(Instance inst,
		   Ice.Communicator com,
    		   Ice.Identity ident,
                    java.util.Hashtable context,
		   String fs,
		   int md,
		   Ice.Connection[] fixedConns)
    {
    	super(inst, com, ident, context, fs, md, false);
        _fixedConnections = fixedConns;
    }

    public Endpoint[]
    getEndpoints()
    {
        return new Endpoint[0];
    }

    public String
    getAdapterId()
    {
        return "";
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeRouter(Ice.RouterPrx newRouter)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
        throw new Ice.FixedProxyException();
    }

    public Reference
    changeTimeout(int newTimeout)
    {
        throw new Ice.FixedProxyException();
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public String
    toString()
	throws Ice.MarshalException
    {
        throw new Ice.FixedProxyException();
    }

    public Ice.Connection
    getConnection()
    {
        java.util.Vector connections = new java.util.Vector();
        switch(getMode())
        {
            case Reference.ModeTwoway:
            case Reference.ModeOneway:
            case Reference.ModeBatchOneway:
            {
                //
                // Filter out datagram connections.
                //
                for(int i = 0; i < _fixedConnections.length; ++i)
                {
                    if(!_fixedConnections[i].endpoint().datagram())
                    {
                        connections.addElement(_fixedConnections[i]);
                    }
                }

                break;
            }

            case Reference.ModeDatagram:
            case Reference.ModeBatchDatagram:
            {
                //
                // Filter out non-datagram connections.
                //
                for(int i = 0; i < _fixedConnections.length; i++)
                {
                    if(_fixedConnections[i].endpoint().datagram())
                    {
                        connections.addElement(_fixedConnections[i]);
                    }
                }

                break;
            }
        }

        if(connections.size() > 1)
        {
            //
            // Randomize the order of the connections.
            //
            java.util.Vector randomizedConnections = new java.util.Vector();
            randomizedConnections.setSize(connections.size());
            java.util.Random r = new java.util.Random();
            java.util.Enumeration e = connections.elements();
            while(e.hasMoreElements())
            {
                int index;
                do
                {
                    index = Math.abs(r.nextInt() % connections.size());
                }   
                while(randomizedConnections.elementAt(index) != null);
                randomizedConnections.setElementAt(e.nextElement(), index);
            }
            connections = randomizedConnections;
        }

        if(connections.size() > 1)
        {
            //
            // If a secure connection is requested or secure overrides is
            // set, remove all non-secure connections. Otherwise if preferSecure is set
            // make secure connections prefered. By default make non-secure
            // connections preferred over secure connections.
            //
            java.util.Vector secureConnections = new java.util.Vector();

	    for(int i = connections.size(); i > 0; --i)
	    {
		if(((Endpoint)connections.elementAt(i - 1)).secure())
		{
		    secureConnections.addElement(connections.elementAt(i - 1));
		    connections.removeElementAt(i - 1);
		}
	    }
	    
            if(getSecure())
            {
                connections = secureConnections;
            }
            else
            {
		java.util.Enumeration e = secureConnections.elements();
		while(e.hasMoreElements())
		{
		    connections.addElement(e.nextElement());
		}
            }
        }
        else if(connections.size() == 1)
        {
            Ice.Connection connection = (Ice.Connection)connections.elementAt(0);
            if(getSecure() && !connection.endpoint().secure())
            {
                connections.removeElementAt(0);
            }
        }

        if(connections.size() == 0)
        {
            Ice.NoEndpointException ex = new Ice.NoEndpointException();
            ex.proxy = ""; // No stringified representation for fixed proxies.
            throw ex;
        }
	
	Ice.Connection connection = (Ice.Connection)connections.elementAt(0);
	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(connection != null);
	}
	connection.throwException();
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
