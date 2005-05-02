// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

	public string getFacet()
	{
	    return _facet;
	}

	public Instance getInstance()
	{
	    return _instance;
	}

	public Ice.Context getContext()
	{
	    return _hasContext ? _context : _instance.getDefaultContext();
	}

	public Reference defaultContext()
	{
	    if(!_hasContext)
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._hasContext = false;
	    r._context = _emptyContext;
	    return r;
	}

	public abstract bool getSecure();
	public abstract Endpoint[] getEndpoints();
	public abstract bool getCollocationOptimization();

	//
	// The change* methods (here and in derived classes) create
	// a new reference based on the existing one, with the
	// corresponding value changed.
	//
	public Reference changeContext(Ice.Context newContext)
	{
	    if(newContext == null)
	    {
		newContext = _emptyContext;
	    }
	    if(_hasContext && newContext.Equals(_context))
	    {
		return this;
	    }
	    Reference r = _instance.referenceFactory().copy(this);
	    r._hasContext = true;
	    if(newContext.Count == 0)
	    {
		r._context = _emptyContext;
	    }
	    else
	    {
	        r._context = (Ice.Context)newContext.Clone();
	    }
	    return r;
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
	    r._identity = newIdentity; // Identity is a value type, therefore a copy of newIdentity is made.
	    return r;
	}

	public bool hasContext()
	{
	    return _hasContext;
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

	public virtual Reference changeDefault()
	{
	    Reference r = _instance.referenceFactory().copy(this);
	    r._mode = Mode.ModeTwoway;
	    r._hasContext = false;
	    r._context = _emptyContext;
	    r._facet = "";
	    return r;
	}

	public abstract Reference changeSecure(bool newSecure);
	public abstract Reference changeRouter(Ice.RouterPrx newRouter);
	public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
	public abstract Reference changeCompress(bool newCompress);
	public abstract Reference changeTimeout(int newTimeout);
	public abstract Reference changeCollocationOptimization(bool newCollocationOptimization);

	public override int GetHashCode()
	{
	    lock(this)
	    {
		if(_hashInitialized)
		{
		    return _hashValue;
		}

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

		if(_hasContext)
		{
		    h = 5 * h + _context.GetHashCode();
		}

		sz = _facet.Length;
		for(int i = 0; i < sz; i++)
		{   
		    h = 5 * h + (int)_facet[i];
		}

		h = 5 * h + (getSecure() ? 1 : 0);

		_hashValue = h;
		_hashInitialized = true;

		return h;
	    }
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

	    s.writeBool(getSecure());

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

	    if(getSecure())
	    {
		s.Append(" -s");
	    }

	    return s.ToString();

	    // Derived class writes the remainder of the string.
	}

	public abstract Ice.ConnectionI getConnection(out bool comp);

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

	    if(_hasContext != r._hasContext)
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

	    return true;
	}

	public Object Clone()
	{
	    //
	    // A member-wise copy is safe because the members are immutable.
	    //
	    return MemberwiseClone();
	}

	private Instance _instance;

	private Mode _mode;
	private Ice.Identity _identity;
	private bool _hasContext;
	private Ice.Context _context;
	private static Ice.Context _emptyContext = new Ice.Context();
	private string _facet;

	protected int _hashValue;
	protected bool _hashInitialized;

	protected Reference(Instance inst,
		  	    Ice.Identity ident,
		  	    Ice.Context ctx,
		  	    string fac,
		  	    Mode md)
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
	    _hasContext = ctx != null && ctx.Count > 0;
	    _context = ctx == null ? _emptyContext : ctx;
	    _facet = fac;
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
	    if(getSecure())
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
	    if(getSecure())
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
    }

    public class FixedReference : Reference
    {
	public FixedReference(Instance inst,
		       	      Ice.Identity ident,
		       	      Ice.Context ctx,
		       	      string fs,
		       	      Reference.Mode md,
		       	      Ice.ConnectionI[] fixedConns)
	    : base(inst, ident, ctx, fs, md)
	{
	    _fixedConnections = fixedConns;
	}

	public Ice.ConnectionI[] getFixedConnections()
	{
	    return _fixedConnections;
	}

	public override bool getSecure()
	{
	    return false;
	}

	public override Endpoint[] getEndpoints()
	{
	    return new Endpoint[0];
	}

	public override bool getCollocationOptimization()
	{
	    return false;
	}

	public override Reference changeSecure(bool sec)
	{
	    return this;
	}

	public override Reference changeRouter(Ice.RouterPrx newRouter)
	{
	    return this;
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    return this;
	}

	public override Reference changeCollocationOptimization(bool newCollocationOptimization)
	{
	    return this;
	}

	public override Reference changeCompress(bool newCompress)
	{
	    // TODO: FixedReferences should probably have a _compress flag,
	    // that gets its default from the fixed connection this reference
	    // refers to. This should be changable with changeCompress(), and
	    // reset in changeDefault().
	    return this;
	}

	public override Reference changeTimeout(int newTimeout)
	{
	    return this;
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

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
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

	public override bool getSecure()
	{
	    return _secure;
	}

	public override bool getCollocationOptimization()
	{
	    return _collocationOptimization;
	}

	public override Reference changeDefault()
	{
	    RoutableReference r = (RoutableReference)base.changeDefault();
	    r._secure = false;
	    r._routerInfo = getInstance().routerManager().get(getInstance().referenceFactory().getDefaultRouter());
	    r._collocationOptimization = false;
	    return r;
	}

	public override Reference changeSecure(bool newSecure)
	{
	    if(newSecure == _secure)
	    {
		return this;
	    }
	    RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	    r._secure = newSecure;
	    return r;
	}

	public override Reference changeRouter(Ice.RouterPrx newRouter)
	{
	    RouterInfo newRouterInfo = getInstance().routerManager().get(newRouter);
	    if(object.ReferenceEquals(newRouterInfo, _routerInfo) ||
		    (newRouterInfo != null && _routerInfo != null && newRouterInfo.Equals(_routerInfo)))
	    {
		return this;
	    }
	    RoutableReference r = (RoutableReference)getInstance().referenceFactory().copy(this);
	    r._routerInfo = newRouterInfo;
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
	    if(_secure != rhs._secure)
	    {
		return false;
	    }
	    if(_collocationOptimization != rhs._collocationOptimization)
	    {
		return false;
	    }
	    return _routerInfo == null ? rhs._routerInfo == null : _routerInfo.Equals(rhs._routerInfo);
	}

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
        }

	protected RoutableReference(Instance inst,
			            Ice.Identity ident,
			            Ice.Context ctx,
			            string fac,
			            Reference.Mode md,
			            bool sec,
			            RouterInfo rtrInfo,
			            bool collocationOpt)
	    : base(inst, ident, ctx, fac, md)
	{
	    _secure = sec;
	    _routerInfo = rtrInfo;
	    _collocationOptimization = collocationOpt;
	}

	private bool _secure;
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

	public override Reference changeDefault()
	{
	    //
	    // Return an indirect reference if a default locator is set.
	    //
	    Ice.LocatorPrx loc = getInstance().referenceFactory().getDefaultLocator();
	    if(loc != null)
	    {
		LocatorInfo newLocatorInfo = getInstance().locatorManager().get(loc);
		return getInstance().referenceFactory().create(getIdentity(), null, "", Mode.ModeTwoway, false, "",
                                                               null, newLocatorInfo, false);
	    }
	    else
	    {
		return base.changeDefault();
	    }
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    if(newLocator != null)
	    {
		LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);
		return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
							       getSecure(), "", null, newLocatorInfo,
							       getCollocationOptimization());
	    }
	    else
	    {
		return this;
	    }
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
	    if(_endpoints.Length > 0)
	    {
		for(int i = 0; i < _endpoints.Length; i++)
		{
		    _endpoints[i].streamWrite(s);
		}
	    }
	    else
	    {
		s.writeString(""); // Adapter id.
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

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
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

	public override Reference changeDefault()
	{
	    //
	    // Return a direct reference if no default locator is defined.
	    //
	    Ice.LocatorPrx loc = getInstance().referenceFactory().getDefaultLocator();
	    if(loc == null)
	    {
		return getInstance().referenceFactory().create(getIdentity(), null, "", Mode.ModeTwoway, false,
							       new Endpoint[0], getRouterInfo(), false);
	    }
	    else
	    {
		IndirectReference r = (IndirectReference)base.changeDefault();
		r._locatorInfo = getInstance().locatorManager().get(loc);
		return r;
	    }
	}

	public override Reference changeLocator(Ice.LocatorPrx newLocator)
	{
	    //
	    // Return a direct reference if a null locator is given.
	    //
	    if(newLocator == null)
	    {
		return getInstance().referenceFactory().create(getIdentity(), getContext(), getFacet(), getMode(),
							       getSecure(), new Endpoint[0], getRouterInfo(),
							       getCollocationOptimization());
	    }
	    else
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
	    if(!_adapterId.Equals(rhs._adapterId))
	    {
		return false;
	    }
	    return object.ReferenceEquals(_locatorInfo, rhs._locatorInfo) ||
		(_locatorInfo != null && rhs._locatorInfo != null && rhs._locatorInfo.Equals(_locatorInfo));
	}

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
        }

	private string _adapterId;
	private LocatorInfo _locatorInfo;
    }
}
