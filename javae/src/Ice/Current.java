// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class Current
{
    public ObjectAdapter adapter;
    public Connection con;
    public Identity id;
    public String facet;
    public String operation;
    public OperationMode mode;
    public java.util.Hashtable ctx;
    public int requestId;

    public Current()
    {
    }

    public Current(ObjectAdapter adapter, Connection con, Identity id, String facet, String operation,
		   OperationMode mode, java.util.Hashtable ctx, int requestId)
    {
	this.adapter = adapter;
	this.con = con;
	this.id = id;
	this.facet = facet;
	this.operation = operation;
	this.mode = mode;
	this.ctx = ctx;
	this.requestId = requestId;
    }

    public boolean
    equals(java.lang.Object rhs)
    {
	Current _r = null;
	try
	{
	    _r = (Current)rhs;
	}
	catch(ClassCastException ex)
	{
	}

	if(_r != null)
	{
	    if(adapter != _r.adapter && adapter != null && !adapter.equals(_r.adapter))
	    {
		return false;
	    }
	    if(con != _r.con && con != null && !con.equals(_r.con))
	    {
		return false;
	    }
	    if(id != _r.id && id != null && !id.equals(_r.id))
	    {
		return false;
	    }
	    if(facet != _r.facet && facet != null && !facet.equals(_r.facet))
	    {
		return false;
	    }
	    if(operation != _r.operation && operation != null && !operation.equals(_r.operation))
	    {
		return false;
	    }
	    if(mode != _r.mode && mode != null && !mode.equals(_r.mode))
	    {
		return false;
	    }
	    if(ctx != _r.ctx && ctx != null && !IceUtil.Hashtable.equals(ctx, _r.ctx))
	    {
		return false;
	    }
	    if(requestId != _r.requestId)
	    {
		return false;
	    }

	    return true;
	}

	return false;
    }

    public java.lang.Object
    ice_clone()
	throws IceUtil.CloneException
    {
	return new Current(adapter, con, id, facet, operation, mode, ctx, requestId);
    }
}
