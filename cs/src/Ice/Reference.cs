// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;
using System.Diagnostics;
using System.Text;

namespace IceInternal
{
    public abstract class Reference : ICloneable
    {
	public enum Mode {
			    ModeTwoway,
			    ModeOneway,
			    ModeBatchOneway,
			    ModeDatagram,
			    ModeBatchDatagram,
			    ModeLast=ModeBatchDatagram
			 };

	public Mode getMode()
	{
	    return _mode;
	}

	public Ice.Identity getIdentity()
	{
	    return _identity;
	}

	public Ice.Context getContext()
	{
	    return _context;
	}

	public string getFacet()
	{
	    return _facet;
	}

	public bool getSecure()
	{
	    return _secure;
	}

	public Instance getInstance()
	{
	    return _instance;
	}

	public Reference changeMode(Mode newMode)
	{
	    if(newMode == _mode)
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._mode = newMode;
	    return r;
	}

	public Reference changeIdentity(Ice.Identity newIdentity)
	{
	    if(newIdentity.Equals(_identity))
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._identity = newIdentity;
	    return r;
	}

	public Reference changeContext(Ice.Context newContext)
	{
	    if(newContext.Equals(_context))
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._context = newContext;
	    return r;
	}

	public Reference changeFacet(string newFacet)
	{
	    if(newFacet.Equals(_facet))
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._facet = newFacet;
	    return r;
	}

	public Reference changeSecure(bool newSecure)
	{
	    if(newSecure == _secure)
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._secure = newSecure;
	    return r;
	}

	public override int GetHashCode()
	{
	    //
	    // Derived class must lock.
	    //

	    int h = (int)_mode;

	    int sz = _identity.name.Length;
	    for(int i = 0; i < sz; i++)
	    {   
		h = 5 * h + (int)_identity.name[i];
	    }

	    sz = _identity.category.Length;
	    for(int i = 0; i < sz; i++)
	    {   
		h = 5 * h + (int)_identity.category[i];
	    }

	    h = 5 * h + _context.GetHashCode();

	    sz = _facet.Length;
	    for(int i = 0; i < sz; i++)
	    {   
		h = 5 * h + (int)_facet[i];
	    }

	    h = 5 * h + (_secure ? 1 : 0);

	    return h;
	}

	public override bool Equals(object obj)
	{
	    //
	    // Note: if(this == obj) and type test are performed by each non-abstract derived class.
	    //

	    Reference r = (Reference)obj; // Guaranteed to succeed.

	    if(_mode != r._mode)
	    {
		return false;
	    }

	    if(!_identity.Equals(r._identity))
	    {
		return false;
	    }

	    if(!_context.Equals(r._context))
	    {
		return false;
	    }

	    if(!_facet.Equals(r._facet))
	    {
		return false;
	    }

	    if(_secure != r._secure)
	    {
		return false;
	    }

	    return true;
	}

	//
	// Marshal the reference.
	//
	public virtual void streamWrite(BasicStream s)
	{
	    //
	    // Don't write the identity here. Operations calling streamWrite
	    // write the identity.
	    //

	    //
	    // For compatibility with the old FacetPath.
	    //
	    if(_facet.Length == 0)
	    {
		s.writeStringSeq(null);
	    }
	    else
	    {
		string[] facetPath = { _facet };
		s.writeStringSeq(facetPath);
	    }

	    s.writeByte((byte)_mode);

	    s.writeBool(_secure);

	    // Derived class writes the remainder of the reference.
	}

	//
	// Convert the reference to its string form.
	//
	public override string ToString()
	{
	    StringBuilder s = new StringBuilder();

	    //
	    // If the encoded identity string contains characters which
	    // the reference parser uses as separators, then we enclose
	    // the identity string in quotes.
	    //
	    string id = Ice.Util.identityToString(_identity);
	    if(IceUtil.StringUtil.findFirstOf(id, " \t\n\r:@") != -1)
	    {
		s.Append('"');
		s.Append(id);
		s.Append('"');
	    }
	    else
	    {
		s.Append(id);
	    }

	    if(_facet.Length > 0)
	    {
		//
		// If the encoded facet string contains characters which
		// the reference parser uses as separators, then we enclose
		// the facet string in quotes.
		//
		s.Append(" -f ");
		string fs = IceUtil.StringUtil.escapeString(_facet, "");
		if(IceUtil.StringUtil.findFirstOf(fs, " \t\n\r:@") != -1)
		{
		    s.Append('"');
		    s.Append(fs);
		    s.Append('"');
		}
		else
		{
		    s.Append(fs);
		}
	    }

	    switch(_mode)
	    {
		case Mode.ModeTwoway:
		{
		    s.Append(" -t");
		    break;
		}

		case Mode.ModeOneway:
		{
		    s.Append(" -o");
		    break;
		}

		case Mode.ModeBatchOneway:
		{
		    s.Append(" -O");
		    break;
		}

		case Mode.ModeDatagram:
		{
		    s.Append(" -d");
		    break;
		}

		case Mode.ModeBatchDatagram:
		{
		    s.Append(" -D");
		    break;
		}
	    }

	    if(_secure)
	    {
		s.Append(" -s");
	    }

	    return s.ToString();

	    // Derived class writes the remainder of the string.
	}

	public Object Clone()
	{
	    return MemberwiseClone();
	}

	private Instance _instance;

	private Mode _mode;
	private Ice.Identity _identity;
	private Ice.Context _context;
	private string _facet;
	private bool _secure;

	protected int _hashValue;
	protected bool _hashInitialized;

	protected Reference(Instance inst,
		  	    Ice.Identity ident,
		  	    Ice.Context ctx,
		  	    string fac,
		  	    Mode md,
		  	    bool sec)
	{
	    //
	    // Validate string arguments.
	    //
	    Debug.Assert(ident.name != null);
	    Debug.Assert(ident.category != null);
	    Debug.Assert(fac != null);

	    _instance = inst;
	    _mode = md;
	    _identity = ident;
	    _context = ctx;
	    _facet = fac;
	    _secure = sec;
	    _hashInitialized = false;
	}

	//
	// Filter endpoints based on criteria from this reference.
	//
	protected Endpoint[] filterEndpoints(Endpoint[] allEndpoints)
	{
	    ArrayList endpoints = new ArrayList();

	    //
	    // Filter out unknown endpoints.
	    //
	    for(int i = 0; i < allEndpoints.Length; i++)
	    {
		if(!allEndpoints[i].unknown())
		{
		    endpoints.Add(allEndpoints[i]);
		}
	    }

	    switch(_mode)
	    {
		case Reference.Mode.ModeTwoway:
		case Reference.Mode.ModeOneway:
		case Reference.Mode.ModeBatchOneway:
		{
		    //
		    // Filter out datagram endpoints.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(Endpoint endpoint in endpoints)
		    {
		    	if(!endpoint.datagram())
			{
			    tmp.Add(endpoint);
			}
		    }
		    endpoints = tmp;
		    break;
		}

		case Reference.Mode.ModeDatagram:
		case Reference.Mode.ModeBatchDatagram:
		{
		    //
		    // Filter out non-datagram endpoints.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(Endpoint endpoint in endpoints)
		    {
		    	if(endpoint.datagram())
			{
			    tmp.Add(endpoint);
			}
		    }
		    endpoints = tmp;
		    break;
		}
	    }

	    //
	    // Randomize the order of endpoints.
	    //
	    for(int i = 0; i < endpoints.Count - 2; ++i)
	    {
		int r = _rand.Next(endpoints.Count - i) + i;
		Debug.Assert(r >= i && r < endpoints.Count);
		if(r != i)
		{
		    object tmp = endpoints[i];
		    endpoints[i] = endpoints[r];
		    endpoints[r] = tmp;
		}
	    }

	    //
	    // If a secure connection is requested, remove all non-secure
	    // endpoints. Otherwise make non-secure endpoints preferred over
	    // secure endpoints by partitioning the endpoint vector, so that
	    // non-secure endpoints come first.
	    //
	    if(_secure)
	    {
		ArrayList tmp = new ArrayList();
		foreach(Endpoint endpoint in endpoints)
		{
		    if(endpoint.secure())
		    {
			tmp.Add(endpoint);
		    }
		}
		endpoints = tmp;
	    }
	    else
	    {
		endpoints.Sort(_endpointComparator);
	    }
	    
	    Endpoint[] arr = new Endpoint[endpoints.Count];
	    if(arr.Length != 0)
	    {
		endpoints.CopyTo(arr);
	    }
	    return arr;
	}

	private class EndpointComparator : IComparer
	{
	    public int Compare(object l, object r)
	    {
		IceInternal.Endpoint le = (IceInternal.Endpoint)l;
		IceInternal.Endpoint re = (IceInternal.Endpoint)r;
		bool ls = le.secure();
		bool rs = re.secure();
		if((ls && rs) || (!ls && !rs))
		{
		    return 0;
		}
		else if(!ls && rs)
		{
		    return -1;
		}
		else
		{
		    return 1;
		}
	    }
	}
	
	private static EndpointComparator _endpointComparator = new EndpointComparator();

	//
	// Filter connections based on criteria from this reference.
	//
	public Ice.ConnectionI[]
	filterConnections(Ice.ConnectionI[] allConnections)
	{
	    ArrayList connections = new ArrayList();

	    switch(_mode)
	    {
		case Reference.Mode.ModeTwoway:
		case Reference.Mode.ModeOneway:
		case Reference.Mode.ModeBatchOneway:
		{
		    //
		    // Filter out datagram endpoints.
		    //
		    for(int i = 0; i < allConnections.Length; ++i)
		    {
			if(!allConnections[i].endpoint().datagram())
			{
			    connections.Add(allConnections[i]);
			}
		    }

		    break;
		}

		case Reference.Mode.ModeDatagram:
		case Reference.Mode.ModeBatchDatagram:
		{
		    //
		    // Filter out non-datagram endpoints.
		    //
		    for(int i = 0; i < allConnections.Length; ++i)
		    {
			if(allConnections[i].endpoint().datagram())
			{
			    connections.Add(allConnections[i]);
			}
		    }

		    break;
		}
	    }

	    //
	    // Randomize the order of connections.
	    //
	    for(int i = 0; i < connections.Count - 2; ++i)
	    {
		int r = _rand.Next(connections.Count - i) + i;
		Debug.Assert(r >= i && r < connections.Count);
		if(r != i)
		{
		    object tmp = connections[i];
		    connections[i] = connections[r];
		    connections[r] = tmp;
		}
	    }

	    //
	    // If a secure connection is requested, remove all non-secure
	    // endpoints. Otherwise make non-secure endpoints preferred over
	    // secure endpoints by partitioning the endpoint vector, so that
	    // non-secure endpoints come first.
	    //
	    if(_secure)
	    {
		ArrayList tmp = new ArrayList();
		foreach(Ice.ConnectionI connection in connections)
		{
		    if(connection.endpoint().secure())
		    {
			tmp.Add(connection);
		    }
		}
		connections = tmp;
	    }
	    else
	    {
		connections.Sort(_connectionComparator);
	    }
	    
	    Ice.ConnectionI[] arr = new Ice.ConnectionI[connections.Count];
	    if(arr.Length != 0)
	    {
		connections.CopyTo(arr);
	    }
	    return arr;
	}

	protected bool compare(object[] arr1, object[] arr2)
	{
	    if(object.ReferenceEquals(arr1, arr2))
	    {
		return true;
	    }

	    if(arr1.Length == arr2.Length)
	    {
		for(int i = 0; i < arr1.Length; i++)
		{
		    if(!arr1[i].Equals(arr2[i]))
		    {
			return false;
		    }
		}

		return true;
	    }

	    return false;
	}

	private class ConnectionComparator : IComparer
	{
	    public int Compare(object l, object r)
	    {
		Ice.ConnectionI lc = (Ice.ConnectionI)l;
		Ice.ConnectionI rc = (Ice.ConnectionI)r;
		bool ls = lc.endpoint().secure();
		bool rs = rc.endpoint().secure();
		if((ls && rs) || (!ls && !rs))
		{
		    return 0;
		}
		else if(!ls && rs)
		{
		    return -1;
		}
		else
		{
		    return 1;
		}
	    }
	}
	
	private static ConnectionComparator _connectionComparator = new ConnectionComparator();

	private static System.Random _rand = new System.Random(unchecked((int)System.DateTime.Now.Ticks));

	public abstract Endpoint[] getEndpoints();
	public abstract bool getCollocationOptimization();
	public abstract Reference changeRouter(Ice.RouterPrx newRouter);
	public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
	public abstract Reference changeDefault();
	public abstract Reference changeCompress(bool newCompress);
	public abstract Reference changeTimeout(int newTimeout);
	public abstract Reference changeCollocationOptimization(bool newCollocationOptimization);
	public abstract Ice.ConnectionI getConnection(out bool comp);
    }

    public class FixedReference : Reference
    {
	public FixedReference(Instance inst,
		       	      Ice.Identity ident,
		       	      Ice.Context ctx,
		       	      string fs,
		       	      Reference.Mode md,
		       	      bool sec,
		       	      Ice.ConnectionI[] fixedConns)
	    : base(inst, ident, ctx, fs, md, sec)
	{
	    _fixedConnections = fixedConns;
	}

	public Ice.ConnectionI[] getFixedConnections()
	{
	    return _fixedConnections;
	}

	public override Endpoint[] getEndpoints()
	{
	    return new Endpoint[0];
	}

	public override bool getCollocationOptimization()
	{
	    return false;
	}

	public override void streamWrite(BasicStream s)
	{
	     Ice.MarshalException ex = new Ice.MarshalException();
	     ex.reason = "Cannot marshal a fixed reference";
	     throw ex;
	}

	public override Ice.ConnectionI getConnection(out bool compress)
	{
	    Ice.ConnectionI[] filteredConns = filterConnections(_fixedConnections);
	    if(filteredConns.Length == 0)
	    {
		Ice.NoEndpointException ex = new Ice.NoEndpointException();
		ex.proxy = ToString();
		throw ex;
	    }

	    Ice.ConnectionI connection = filteredConns[0];
	    Debug.Assert(connection != null);
	    compress = connection.endpoint().compress();

	    return connection;
	}

	public override Reference changeRouter(Ice.RouterPrx newRouter)
	{
	    return this;
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    return this;
	}

	public override Reference changeDefault()
	{
	    return this;
	}

	public override Reference changeCollocationOptimization(bool newCollocationOptimization)
	{
	    return this;
	}

	public override Reference changeCompress(bool newCompress)
	{
	    return this;
	}

	public override Reference changeTimeout(int newTimeout)
	{
	    return this;
	}

	public override int GetHashCode()
	{
	    lock(this)
	    {
		if(_hashInitialized)
		{
		    return _hashValue;
		}

		_hashInitialized = true;
		_hashValue = base._hashValue * 5 + _fixedConnections.GetHashCode();
		return _hashValue;
	    }
	}

	public override bool Equals(object obj)
	{
	    if(object.ReferenceEquals(this, obj))
	    {
		return true;
	    }
	    if(!(obj is FixedReference))
	    {
		return false;
	    }
	    FixedReference rhs = (FixedReference)obj;
	    if(!base.Equals(rhs))
	    {
		return false;
	    }
	    return compare(_fixedConnections, rhs._fixedConnections);
	}

	private Ice.ConnectionI[] _fixedConnections;
    }

    public abstract class RoutableReference : Reference
    {
	public RouterInfo getRouterInfo()
	{
	    return _routerInfo;
	}

	public Endpoint[] getRoutedEndpoints()
	{
	    if(_routerInfo != null)
	    {
		//
		// If we route, we send everything to the router's client
		// proxy endpoints.
		//
		Ice.ObjectPrx clientProxy = _routerInfo.getClientProxy();
		return ((Ice.ObjectPrxHelperBase)clientProxy).__reference().getEndpoints();
	    }
	    return new Endpoint[0];
	}

	public override bool getCollocationOptimization()
	{
	    return _collocationOptimization;
	}

	public override Reference changeRouter(Ice.RouterPrx newRouter)
	{
	    RouterInfo newRouterInfo = getInstance().routerManager().get(newRouter);
	    if(object.ReferenceEquals(newRouterInfo, _routerInfo))
	    {
		return this;
	    }
	    RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	    r._routerInfo = newRouterInfo;
	    return r;
	}

	public override Reference changeDefault()
	{
	    RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	    r._routerInfo = getInstance().routerManager().get(getInstance().referenceFactory().getDefaultRouter());
	    return r;
	}

	public override Reference changeCollocationOptimization(bool newCollocationOptimization)
	{
	    if(newCollocationOptimization == _collocationOptimization)
	    {
		return this;
	    }
	    RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	    r._collocationOptimization = newCollocationOptimization;
	    return r;
	}

	public override int GetHashCode()
	{
	    //
	    // Derived class must lock.
	    //

	    if(_routerInfo != null)
	    {
		_hashValue = _hashValue * 5 + _routerInfo.GetHashCode();
	    }
	    _hashValue = _hashValue * 2 + _collocationOptimization.GetHashCode();
	    return _hashValue;
	}

	public override bool Equals(object obj)
	{
	    //
	    // Note: if(this == obj) and type test are performed by each non-abstract derived class.
	    //

	    if(!base.Equals(obj))
	    {
		return false;
	    }
	    RoutableReference rhs = (RoutableReference)obj; // Guaranteed to succeed.
	    if(_collocationOptimization != rhs._collocationOptimization)
	    {
		return false;
	    }
	    return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.Equals(rhs._routerInfo);
	}

	protected RoutableReference(Instance inst,
			            Ice.Identity ident,
			            Ice.Context ctx,
			            string fac,
			            Reference.Mode md,
			            bool sec,
			            RouterInfo rtrInfo,
			            bool collocationOpt)
	    : base(inst, ident, ctx, fac, md, sec)
	{
	    _routerInfo = rtrInfo;
	    _collocationOptimization = collocationOpt;
	}

	private RouterInfo _routerInfo; // Null if no router is used.
	private bool _collocationOptimization;
    }

    public class DirectReference : RoutableReference
    {
	public DirectReference(Instance inst,
			       Ice.Identity ident,
			       Ice.Context ctx,
			       string fs,
			       Reference.Mode md,
			       bool sec,
			       Endpoint[] endpts,
			       RouterInfo rtrInfo,
			       bool collocationOpt)
	    : base(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt)
	{
	    _endpoints = endpts;
	}

	public override Endpoint[] getEndpoints()
	{
	    return _endpoints;
	}

	public Reference changeEndpoints(Endpoint[] newEndpoints)
	{
	    if(compare(newEndpoints, _endpoints))
	    {
		return this;
	    }
	    DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
	    r._endpoints = newEndpoints;
	    return r;
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    return this;
	}

	public override Reference changeDefault()
	{
	    return this;
	}

	public override Reference changeCompress(bool newCompress)
	{
	    DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
	    Endpoint[] newEndpoints = new Endpoint[_endpoints.Length];
	    for(int i = 0; i < _endpoints.Length; i++)
	    {
		newEndpoints[i] = _endpoints[i].compress(newCompress);
	    }
	    r._endpoints = newEndpoints;
	    return r;
	}

	public override Reference changeTimeout(int newTimeout)
	{
	    DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
	    Endpoint[] newEndpoints = new Endpoint[_endpoints.Length];
	    for(int i = 0; i < _endpoints.Length; i++)
	    {
		newEndpoints[i] = _endpoints[i].timeout(newTimeout);
	    }
	    r._endpoints = newEndpoints;
	    return r;
	}

	public override void streamWrite(BasicStream s)
	{
	    base.streamWrite(s);

	    s.writeSize(_endpoints.Length);
	    for(int i = 0; i < _endpoints.Length; i++)
	    {
		_endpoints[i].streamWrite(s);
	    }
	}

	public override string ToString()
	{
	    StringBuilder s = new StringBuilder();
	    s.Append(base.ToString());

	    for(int i = 0; i < _endpoints.Length; i++)
	    {
		string endp = _endpoints[i].ToString();
		if(endp != null && endp.Length > 0)
		{
		    s.Append(':');
		    s.Append(endp);
		}
	    }
	    return s.ToString();
	}

	public override Ice.ConnectionI getConnection(out bool comp)
	{
	    Endpoint[] endpts = base.getRoutedEndpoints();
	    if(endpts.Length == 0)
	    {
		endpts =_endpoints;
	    }
	    Endpoint[] filteredEndpoints = filterEndpoints(endpts);
	    if(filteredEndpoints.Length == 0)
	    {
		Ice.NoEndpointException ex = new Ice.NoEndpointException();
		ex.proxy = ToString();
		throw ex;
	    }

	    OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
	    Ice.ConnectionI connection = factory.create(filteredEndpoints, out comp);

	    //
	    // If we have a router, set the object adapter for this router
	    // (if any) to the new connection, so that callbacks from the
	    // router can be received over this new connection.
	    //
	    if(getRouterInfo() != null)
	    {
		connection.setAdapter(getRouterInfo().getAdapter());
	    }

	    Debug.Assert(connection != null);
	    return connection;
	}

	public override int GetHashCode()
	{
	    lock(this)
	    {
		if(_hashInitialized)
		{
		    return _hashValue;
		}

		_hashInitialized = true;
		_hashValue = base._hashValue * 5 + _endpoints.GetHashCode();
		return _hashValue;
	    }
	}

	public override bool Equals(object obj)
	{
	    if(Object.ReferenceEquals(this, obj))
	    {
		return true;
	    }
	    if(!(obj is DirectReference))
	    {
		return false;
	    }
	    DirectReference rhs = (DirectReference)obj;
	    if(!base.Equals(rhs))
	    {
		return false;
	    }
	    return compare(_endpoints, rhs._endpoints);
	}

	private Endpoint[] _endpoints;
    }

    public class IndirectReference : RoutableReference
    {
	public IndirectReference(Instance inst,
			         Ice.Identity ident,
			         Ice.Context ctx,
			         string fs,
			         Reference.Mode md,
			         bool sec,
			         string adptid,
			         RouterInfo rtrInfo,
			         LocatorInfo locInfo,
			         bool collocationOpt)
	    : base(inst, ident, ctx, fs, md, sec, rtrInfo, collocationOpt)
	{
	    _adapterId = adptid;
	    _locatorInfo = locInfo;
	}

	public string getAdapterId()
	{
	    return _adapterId;
	}

	public LocatorInfo getLocatorInfo()
	{
	    return _locatorInfo;
	}

	public override Endpoint[] getEndpoints()
	{
	    return new Endpoint[0];
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);

	    if(object.ReferenceEquals(newLocatorInfo, _locatorInfo) ||
		(_locatorInfo != null && newLocatorInfo != null && newLocatorInfo.Equals(_locatorInfo)))
	    {
		return this;
	    }
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    r._locatorInfo = newLocatorInfo;
	    return this;
	}

	public override Reference changeDefault()
	{
	    IndirectReference r = (IndirectReference)base.changeDefault();
	    r._locatorInfo = getInstance().locatorManager().get(getInstance().referenceFactory().getDefaultLocator());
	    return r;
	}

	public override Reference changeCompress(bool newCompress)
	{
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    if(_locatorInfo != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
						    _locatorInfo.getLocator().ice_compress(newCompress));
		r._locatorInfo = getInstance().locatorManager().get(newLocator);
	    }
	    return r;
	}

	public override Reference changeTimeout(int newTimeout)
	{
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    if(_locatorInfo != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
						    _locatorInfo.getLocator().ice_timeout(newTimeout));
		r._locatorInfo = getInstance().locatorManager().get(newLocator);
	    }
	    return r;
	}

	public override void streamWrite(BasicStream s)
	{
	    base.streamWrite(s);

	    s.writeSize(0);
	    s.writeString(_adapterId);
	}

	public override string ToString()
	{
	    string result = base.ToString();

	    if(_adapterId.Length == 0)
	    {
		return result;
	    }

	    StringBuilder s = new StringBuilder();
	    s.Append(result);
	    s.Append(" @ ");

	    //
	    // If the encoded adapter id string contains characters which
	    // the reference parser uses as separators, then we enclose
	    // the adapter id string in quotes.
	    //
	    string a = IceUtil.StringUtil.escapeString(_adapterId, null);
	    if(IceUtil.StringUtil.findFirstOf(a, " \t\n\r") != -1)
	    {
		s.Append('"');
		s.Append(a);
		s.Append('"');
	    }
	    else
	    {
		s.Append(a);
	    }
	    return s.ToString();
	}

	public override Ice.ConnectionI getConnection(out bool comp)
	{
	    Ice.ConnectionI connection;

	    while(true)
	    {
		Endpoint[] endpts = base.getRoutedEndpoints();
		bool cached = false;
		if(endpts.Length == 0 && _locatorInfo != null)
		{
		    endpts = _locatorInfo.getEndpoints(this, out cached);
		}
		Endpoint[] filteredEndpoints = filterEndpoints(endpts);
		if(filteredEndpoints.Length == 0)
		{
		    Ice.NoEndpointException ex = new Ice.NoEndpointException();
		    ex.proxy = ToString();
		    throw ex;
		}

		try
		{
		    OutgoingConnectionFactory factory = getInstance().outgoingConnectionFactory();
		    connection = factory.create(filteredEndpoints, out comp);
		    Debug.Assert(connection != null);
		}
		catch(Ice.LocalException ex)
		{
		    if(getRouterInfo() == null)
		    {
			Debug.Assert(_locatorInfo != null);
			_locatorInfo.clearCache(this);

			if(cached)
			{
			    TraceLevels traceLevels = getInstance().traceLevels();
			    Ice.Logger logger = getInstance().logger();
			    
			    if(traceLevels.retry >= 2)
			    {
				String s = "connection to cached endpoints failed\n" +
					   "removing endpoints from cache and trying one more time\n" + ex;
				logger.trace(traceLevels.retryCat, s);
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

	    Debug.Assert(connection != null);
	    return connection;
	}

	public override int GetHashCode()
	{
	    lock(this)
	    {
		if(_hashInitialized)
		{
		    return _hashValue;
		}

		_hashInitialized = true;
		_hashValue = base._hashValue * 5 + _adapterId.GetHashCode();
		if(_locatorInfo != null)
		{
		    _hashValue = _hashValue * 5 + _locatorInfo.GetHashCode();
		}
		return _hashValue;
	    }
	}

	public override bool Equals(object obj)
	{
	    if(object.ReferenceEquals(this, obj))
	    {
		return true;
	    }
	    if(!(obj is IndirectReference))
	    {
		return false;
	    }
	    IndirectReference rhs = (IndirectReference)obj;
	    if(!base.Equals(rhs))
	    {
		return false;
	    }
	    return _adapterId.Equals(rhs._adapterId) && _locatorInfo.Equals(rhs._locatorInfo);
	}

	private string _adapterId;
	private LocatorInfo _locatorInfo;
    }
}
