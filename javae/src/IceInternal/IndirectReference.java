// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class IndirectReference extends RoutableReference
{
    public
    IndirectReference(Instance inst,
		      Ice.Communicator com,
    		      Ice.Identity ident,
                      java.util.Hashtable context,
		      String fs,
		      int md,
		      boolean sec,
		      String adptid,
		      RouterInfo rtrInfo,
		      LocatorInfo locInfo)
    {
    	super(inst, com, ident, context, fs, md, sec, rtrInfo);
        _adapterId = adptid;
	_locatorInfo = locInfo;
    }

    public final String
    getAdapterId()
    {
        return _adapterId;
    }

    public final LocatorInfo
    getLocatorInfo()
    {
        return _locatorInfo;
    }

    public Endpoint[]
    getEndpoints()
    {
        return new Endpoint[0];
    }

    public Reference
    changeAdapterId(String newAdapterId)
    {
        if(_adapterId.equals(newAdapterId))
        {
            return this;
        }
        IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
        r._adapterId = newAdapterId;
        return r;       
    }

    public Reference
    changeLocator(Ice.LocatorPrx newLocator)
    {
	LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
	if(_locatorInfo != null && newLocatorInfo != null && newLocatorInfo.equals(_locatorInfo))
	{
	    return this;
	}
	IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	r._locatorInfo = newLocatorInfo;
	return r;
    }

    public void
    streamWrite(BasicStream s)
	throws Ice.MarshalException
    {
        super.streamWrite(s);

	s.writeSize(0);
	s.writeString(_adapterId);
    }

    public String
    toString()
    {
	//
	// WARNING: Certain features, such as proxy validation in Glacier2,
	// depend on the format of proxy strings. Changes to toString() and
	// methods called to generate parts of the reference string could break
	// these features. Please review for all features that depend on the
	// format of proxyToString() before changing this and related code.
	//
	String result = super.toString();

	if(_adapterId.length() == 0)
	{
	    return result;
	}

	StringBuffer s = new StringBuffer();
	s.append(result);
	s.append(" @ ");

	//
	// If the encoded adapter id string contains characters which
	// the reference parser uses as separators, then we enclose
	// the adapter id string in quotes.
	//
	String a = IceUtil.StringUtil.escapeString(_adapterId, null);
	if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
	{
	    s.append('"');
	    s.append(a);
	    s.append('"');
	}
	else
	{
	    s.append(a);
	}
	return s.toString();
    }

    public Ice.Connection
    getConnection()
    {
	Ice.Connection connection;

	while(true)
	{
	    Endpoint[] endpts = super.getRoutedEndpoints();
	    Ice.BooleanHolder cached = new Ice.BooleanHolder(false);
	    if(endpts.length == 0 && _locatorInfo != null)
	    {
	        endpts = _locatorInfo.getEndpoints(this, cached);
	    }

	    applyOverrides(endpts);

	    Endpoint[] filteredEndpoints = filterEndpoints(endpts);
	    if(filteredEndpoints.length == 0)
	    {
	        Ice.NoEndpointException ex = new Ice.NoEndpointException();
		ex.proxy = toString();
		throw ex;
	    }

	    try
	    {
		OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
		connection = factory.create(filteredEndpoints);
		if(IceUtil.Debug.ASSERT)
		{
		    IceUtil.Debug.Assert(connection != null);
		}
	    }
	    catch(Ice.LocalException ex)
	    {
		if(getRouterInfo() == null)
		{
		    if(IceUtil.Debug.ASSERT)
		    {
			IceUtil.Debug.Assert(_locatorInfo != null);
		    }
		    _locatorInfo.clearCache(this);

		    if(cached.value)
		    {
			TraceLevels traceLevels = getInstance().traceLevels();
			if(traceLevels.retry >= 2)
			{
			    String s = "connection to cached endpoints failed\n" +
				       "removing endpoints from cache and trying one more time\n" + ex;
			    getInstance().initializationData().logger.trace(traceLevels.retryCat, s);
			}
			
			continue;
		    }
		}

		throw ex;
	    }

	    break;
	}

	//
	// If we have a router, set the object adapter for this router
	// (if any) to the new connection, so that callbacks from the
	// router can be received over this new connection.
	//
	if(getRouterInfo() != null)
	{
	    connection.setAdapter(getRouterInfo().getAdapter());
	}

	if(IceUtil.Debug.ASSERT)
	{
	    IceUtil.Debug.Assert(connection != null);
	}
	return connection;
    }

    public synchronized int
    hashCode()
    {
        if(_hashInitialized)
	{
	    return _hashValue;
	}
	super.hashCode();             // Initializes _hashValue.
	int sz = _adapterId.length(); // Add hash of adapter ID to base hash.
	for(int i = 0; i < sz; i++)
	{
	    _hashValue = 5 * _hashValue + (int)_adapterId.charAt(i);
	}
	return _hashValue;
    }

    public boolean
    equals(java.lang.Object obj)
    {
        if(this == obj)
	{
	    return true;
	}
	if(!(obj instanceof IndirectReference))
	{
	    return false;
	}
        IndirectReference rhs = (IndirectReference)obj;
        if(!super.equals(rhs))
        {
            return false;
        }
	if(!_adapterId.equals(rhs._adapterId))
	{
	   return false;
	}
	return _locatorInfo == null ? rhs._locatorInfo == null : _locatorInfo.equals(rhs._locatorInfo);
    }

    protected
    IndirectReference()
    {
    }

    protected void
    shallowCopy(IndirectReference ref)
    {
	super.shallowCopy(ref);
	ref._adapterId = _adapterId;
	ref._locatorInfo = _locatorInfo;
    }

    public java.lang.Object
    ice_clone()
    {
	IndirectReference result = new IndirectReference();
        shallowCopy(result);
	return result;
    }
    
    private String _adapterId;
    private LocatorInfo _locatorInfo;
}
