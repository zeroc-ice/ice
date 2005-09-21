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
	    return mode_;
	}

	public Ice.Identity getIdentity()
	{
	    return identity_;
	}

	public string getFacet()
	{
	    return facet_;
	}

	public Instance getInstance()
	{
	    return instance_;
	}

	public Ice.Context getContext()
	{
	    return context_;
	}

	public Reference defaultContext()
	{
	    Reference r = instance_.referenceFactory().copy(this);
	    r.context_ = instance_.getDefaultContext();
	    return r;
	}

	public Ice.Communicator getCommunicator()
	{
	    return communicator_;
	}
	public abstract bool getSecure();
	public abstract string getAdapterId();
	public abstract EndpointI[] getEndpoints();
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
	    Reference r = instance_.referenceFactory().copy(this);
	    if(newContext.Count == 0)
	    {
		r.context_ = _emptyContext;
	    }
	    else
	    {
	        r.context_ = (Ice.Context)newContext.Clone();
	    }
	    return r;
	}

	public Reference changeMode(Mode newMode)
	{
	    if(newMode == mode_)
	    {
		return this;
	    }
	    Reference r = instance_.referenceFactory().copy(this);
	    r.mode_ = newMode;
	    return r;
	}

	public Reference changeIdentity(Ice.Identity newIdentity)
	{
	    if(newIdentity.Equals(identity_))
	    {
		return this;
	    }
	    Reference r = instance_.referenceFactory().copy(this);
	    r.identity_ = newIdentity; // Identity is a value type, therefore a copy of newIdentity is made.
	    return r;
	}

	public Reference changeFacet(string newFacet)
	{
	    if(newFacet.Equals(facet_))
	    {
		return this;
	    }
	    Reference r = instance_.referenceFactory().copy(this);
	    r.facet_ = newFacet;
	    return r;
	}

	public virtual Reference changeDefault()
	{
	    Reference r = instance_.referenceFactory().copy(this);
	    r.mode_ = Mode.ModeTwoway;
	    r.context_ = instance_.getDefaultContext();
	    r.facet_ = "";
	    return r;
	}

	public abstract Reference changeSecure(bool newSecure);
	public abstract Reference changeRouter(Ice.RouterPrx newRouter);
	public abstract Reference changeLocator(Ice.LocatorPrx newLocator);
	public abstract Reference changeCompress(bool newCompress);
	public abstract Reference changeTimeout(int newTimeout);
	public abstract Reference changeCollocationOptimization(bool newCollocationOptimization);
	public abstract Reference changeAdapterId(string newAdapterId);
	public abstract Reference changeEndpoints(EndpointI[] newEndpoints);

	public override int GetHashCode()
	{
	    lock(this)
	    {
		if(hashInitialized_)
		{
		    return hashValue_;
		}

		int h = (int)mode_;

		int sz = identity_.name.Length;
		for(int i = 0; i < sz; i++)
		{   
		    h = 5 * h + (int)identity_.name[i];
		}

		sz = identity_.category.Length;
		for(int i = 0; i < sz; i++)
		{   
		    h = 5 * h + (int)identity_.category[i];
		}

		h = 5 * h + context_.GetHashCode();

		sz = facet_.Length;
		for(int i = 0; i < sz; i++)
		{   
		    h = 5 * h + (int)facet_[i];
		}

		h = 5 * h + (getSecure() ? 1 : 0);

		hashValue_ = h;
		hashInitialized_ = true;

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
	    if(facet_.Length == 0)
	    {
		s.writeStringSeq(null);
	    }
	    else
	    {
		string[] facetPath = { facet_ };
		s.writeStringSeq(facetPath);
	    }

	    s.writeByte((byte)mode_);

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
	    string id = Ice.Util.identityToString(identity_);
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

	    if(facet_.Length > 0)
	    {
		//
		// If the encoded facet string contains characters which
		// the reference parser uses as separators, then we enclose
		// the facet string in quotes.
		//
		s.Append(" -f ");
		string fs = IceUtil.StringUtil.escapeString(facet_, "");
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

	    switch(mode_)
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

	    if(mode_ != r.mode_)
	    {
		return false;
	    }

	    if(!identity_.Equals(r.identity_))
	    {
		return false;
	    }

	    if(!context_.Equals(r.context_))
	    {
		return false;
	    }

	    if(!facet_.Equals(r.facet_))
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

	private Instance instance_;
	private Ice.Communicator communicator_;
	private Mode mode_;
	private Ice.Identity identity_;
	private Ice.Context context_;
	private static Ice.Context _emptyContext = new Ice.Context();
	private string facet_;

	protected int hashValue_;
	protected bool hashInitialized_;

	protected Reference(Instance inst,
			    Ice.Communicator com,
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

	    instance_ = inst;
	    communicator_ = com;
	    mode_ = md;
	    identity_ = ident;
	    context_ = ctx == null ? _emptyContext : ctx;
	    facet_ = fac;
	    hashInitialized_ = false;
	}

	//
	// Filter endpoints based on criteria from this reference.
	//
	protected EndpointI[] filterEndpoints(EndpointI[] allEndpoints)
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

	    switch(mode_)
	    {
		case Reference.Mode.ModeTwoway:
		case Reference.Mode.ModeOneway:
		case Reference.Mode.ModeBatchOneway:
		{
		    //
		    // Filter out datagram endpoints.
		    //
		    ArrayList tmp = new ArrayList();
		    foreach(EndpointI endpoint in endpoints)
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
		    foreach(EndpointI endpoint in endpoints)
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
		foreach(EndpointI endpoint in endpoints)
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
	    
	    EndpointI[] arr = new EndpointI[endpoints.Count];
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
		IceInternal.EndpointI le = (IceInternal.EndpointI)l;
		IceInternal.EndpointI re = (IceInternal.EndpointI)r;
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

	    switch(mode_)
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
			      Ice.Communicator com,
		       	      Ice.Identity ident,
		       	      Ice.Context ctx,
		       	      string fs,
		       	      Reference.Mode md,
		       	      Ice.ConnectionI[] fixedConns)
	    : base(inst, com, ident, ctx, fs, md)
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

	public override string getAdapterId()
        {
	    return "";
	}

	public override EndpointI[] getEndpoints()
	{
	    return new EndpointI[0];
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

	public override Reference changeAdapterId(string newAdapterId)
        {
	    return this;
	}

	public override Reference changeEndpoints(EndpointI[] newEndpoints)
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
	     ex.reason = "Cannot marshal a fixed proxy";
	     throw ex;
	}

	public override string ToString()
	{
	     Ice.MarshalException ex = new Ice.MarshalException();
	     ex.reason = "Cannot marshal a fixed proxy";
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
	    return IceUtil.Arrays.Equals(_fixedConnections, rhs._fixedConnections);
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

	public EndpointI[] getRoutedEndpoints()
	{
	    if(_routerInfo != null)
	    {
		//
		// If we route, we send everything to the router's client
		// proxy endpoints.
		//
		Ice.ObjectPrx clientProxy = _routerInfo.getClientProxy();
		return ((Ice.ObjectPrxHelperBase)clientProxy).reference__().getEndpoints();
	    }
	    return new EndpointI[0];
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
				    Ice.Communicator com,
			            Ice.Identity ident,
			            Ice.Context ctx,
			            string fac,
			            Reference.Mode md,
			            bool sec,
			            RouterInfo rtrInfo,
			            bool collocationOpt)
	    : base(inst, com, ident, ctx, fac, md)
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
			       Ice.Communicator com,
			       Ice.Identity ident,
			       Ice.Context ctx,
			       string fs,
			       Reference.Mode md,
			       bool sec,
			       EndpointI[] endpts,
			       RouterInfo rtrInfo,
			       bool collocationOpt)
	    : base(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt)
	{
	    _endpoints = endpts;
	}

	public override string getAdapterId()
        {
	    return "";
	}

	public override EndpointI[] getEndpoints()
	{
	    return _endpoints;
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
		return getInstance().referenceFactory().create(
		    getIdentity(), getInstance().getDefaultContext(), "", Mode.ModeTwoway, false, "", null,
		    newLocatorInfo, getInstance().defaultsAndOverrides().defaultCollocationOptimization);
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
		return getInstance().referenceFactory().create(
		    getIdentity(), getContext(), getFacet(), getMode(), getSecure(), "", null, newLocatorInfo,
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
	    EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
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
	    EndpointI[] newEndpoints = new EndpointI[_endpoints.Length];
	    for(int i = 0; i < _endpoints.Length; i++)
	    {
		newEndpoints[i] = _endpoints[i].timeout(newTimeout);
	    }
	    r._endpoints = newEndpoints;
	    return r;
	}

	public override Reference changeAdapterId(string newAdapterId)
        {
	    if(newAdapterId == null || newAdapterId.Length == 0)
	    {
		return this;
	    }
	    LocatorInfo locatorInfo = 
		getInstance().locatorManager().get(getInstance().referenceFactory().getDefaultLocator());
	    return getInstance().referenceFactory().create(
		getIdentity(), getContext(), getFacet(), getMode(), getSecure(), newAdapterId, getRouterInfo(),
		locatorInfo, getCollocationOptimization());
	}

	public override Reference changeEndpoints(EndpointI[] newEndpoints)
	{
	    if(Array.Equals(newEndpoints, _endpoints))
	    {
		return this;
	    }
	    DirectReference r = (DirectReference)getInstance().referenceFactory().copy(this);
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
	    EndpointI[] endpts = base.getRoutedEndpoints();
	    if(endpts.Length == 0)
	    {
		endpts =_endpoints;
	    }
	    EndpointI[] filteredEndpoints = filterEndpoints(endpts);
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
	    return IceUtil.Arrays.Equals(_endpoints, rhs._endpoints);
	}

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
        }

	private EndpointI[] _endpoints;
    }

    public class IndirectReference : RoutableReference
    {
	public IndirectReference(Instance inst,
				 Ice.Communicator com,
			         Ice.Identity ident,
			         Ice.Context ctx,
			         string fs,
			         Reference.Mode md,
			         bool sec,
			         string adptid,
			         RouterInfo rtrInfo,
			         LocatorInfo locInfo,
			         bool collocationOpt)
	    : base(inst, com, ident, ctx, fs, md, sec, rtrInfo, collocationOpt)
	{
	    adapterId_ = adptid;
	    locatorInfo_ = locInfo;
	}

	public LocatorInfo getLocatorInfo()
	{
	    return locatorInfo_;
	}

	public override string getAdapterId()
	{
	    return adapterId_;
	}

	public override EndpointI[] getEndpoints()
	{
	    return new EndpointI[0];
	}

	public override Reference changeDefault()
	{
	    //
	    // Return a direct reference if no default locator is defined.
	    //
	    Ice.LocatorPrx loc = getInstance().referenceFactory().getDefaultLocator();
	    if(loc == null)
	    {
		return getInstance().referenceFactory().create(
		    getIdentity(), getInstance().getDefaultContext(), "", Mode.ModeTwoway, false, new EndpointI[0],
		    getRouterInfo(), getInstance().defaultsAndOverrides().defaultCollocationOptimization);
	    }
	    else
	    {
		IndirectReference r = (IndirectReference)base.changeDefault();
		r.locatorInfo_ = getInstance().locatorManager().get(loc);
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
		return getInstance().referenceFactory().create(
		    getIdentity(), getContext(), getFacet(), getMode(), getSecure(), new EndpointI[0], getRouterInfo(),
		    getCollocationOptimization());
	    }
	    else
	    {
		LocatorInfo newLocatorInfo = getInstance().locatorManager().get(newLocator);

		if(object.ReferenceEquals(newLocatorInfo, locatorInfo_) ||
		    (locatorInfo_ != null && newLocatorInfo != null && newLocatorInfo.Equals(locatorInfo_)))
		{
		    return this;
		}
		IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
		r.locatorInfo_ = newLocatorInfo;
		return this;
	    }
	}

	public override Reference changeCompress(bool newCompress)
	{
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    if(locatorInfo_ != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
						    locatorInfo_.getLocator().ice_compress(newCompress));
		r.locatorInfo_ = getInstance().locatorManager().get(newLocator);
	    }
	    return r;
	}

	public override Reference changeTimeout(int newTimeout)
	{
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    if(locatorInfo_ != null)
	    {
		Ice.LocatorPrx newLocator = Ice.LocatorPrxHelper.uncheckedCast(
						    locatorInfo_.getLocator().ice_timeout(newTimeout));
		r.locatorInfo_ = getInstance().locatorManager().get(newLocator);
	    }
	    return r;
	}

	public override Reference changeAdapterId(string newAdapterId)
        {
	    if(adapterId_.Equals(newAdapterId))
	    {
		return this;
	    }
	    IndirectReference r = (IndirectReference)getInstance().referenceFactory().copy(this);
	    r.adapterId_ = newAdapterId;
	    return r;
	}

	public override Reference changeEndpoints(EndpointI[] newEndpoints)
        {
	    if(newEndpoints == null || newEndpoints.Length == 0)
	    {
		return this;
	    }
	    return getInstance().referenceFactory().create(
		getIdentity(), getContext(), getFacet(), getMode(), getSecure(), newEndpoints, getRouterInfo(),
		getCollocationOptimization());
	}

	public override void streamWrite(BasicStream s)
	{
	    base.streamWrite(s);

	    s.writeSize(0);
	    s.writeString(adapterId_);
	}

	public override string ToString()
	{
	    string result = base.ToString();

	    if(adapterId_.Length == 0)
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
	    string a = IceUtil.StringUtil.escapeString(adapterId_, null);
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
		EndpointI[] endpts = base.getRoutedEndpoints();
		bool cached = false;
		if(endpts.Length == 0 && locatorInfo_ != null)
		{
		    endpts = locatorInfo_.getEndpoints(this, out cached);
		}
		EndpointI[] filteredEndpoints = filterEndpoints(endpts);
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
			Debug.Assert(locatorInfo_ != null);
			locatorInfo_.clearCache(this);

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
	    if(!adapterId_.Equals(rhs.adapterId_))
	    {
		return false;
	    }
	    return object.ReferenceEquals(locatorInfo_, rhs.locatorInfo_) ||
		(locatorInfo_ != null && rhs.locatorInfo_ != null && rhs.locatorInfo_.Equals(locatorInfo_));
	}

        //
        // If we override Equals, we must also override GetHashCode.
        //
	public override int GetHashCode()
	{
            return base.GetHashCode();
        }

	private string adapterId_;
	private LocatorInfo locatorInfo_;
    }
}
