//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * Information about the current method invocation for servers. Each
 * operation on the server has a <code>Current</code> as its implicit final
 * parameter. <code>Current</code> is mostly used for Ice services. Most
 * applications ignore this parameter.
 **/
public class Current implements java.lang.Cloneable
{
    /**
     * The object adapter.
     **/
    public ObjectAdapter adapter;

    /**
     * Information about the connection over which the current method
     * invocation was received. If the invocation is direct due to
     * collocation optimization, this value is set to null.
     **/
    public Connection con;

    /**
     * The Ice object identity.
     **/
    public Identity id;

    /**
     * The facet.
     **/
    public String facet;

    /**
     * The operation name.
     **/
    public String operation;

    /**
     * The mode of the operation.
     **/
    public OperationMode mode;

    /**
     * The request context, as received from the client.
     **/
    public java.util.Map<java.lang.String, java.lang.String> ctx;

    /**
     * The request id unless oneway (0).
     **/
    public int requestId;

    /**
     * The encoding version used to encode the input and output parameters.
     **/
    public EncodingVersion encoding;

    public Current()
    {
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
        this.mode = OperationMode.Normal;
        this.encoding = new EncodingVersion();
    }

    public Current(ObjectAdapter adapter, Connection con, Identity id, String facet, String operation, OperationMode mode, java.util.Map<java.lang.String, java.lang.String> ctx, int requestId, EncodingVersion encoding)
    {
        this.adapter = adapter;
        this.con = con;
        this.id = id;
        this.facet = facet;
        this.operation = operation;
        this.mode = mode;
        this.ctx = ctx;
        this.requestId = requestId;
        this.encoding = encoding;
    }

    public boolean equals(java.lang.Object rhs)
    {
        if(this == rhs)
        {
            return true;
        }
        Current r = null;
        if(rhs instanceof Current)
        {
            r = (Current)rhs;
        }

        if(r != null)
        {
            if(this.adapter != r.adapter)
            {
                if(this.adapter == null || r.adapter == null || !this.adapter.equals(r.adapter))
                {
                    return false;
                }
            }
            if(this.con != r.con)
            {
                if(this.con == null || r.con == null || !this.con.equals(r.con))
                {
                    return false;
                }
            }
            if(this.id != r.id)
            {
                if(this.id == null || r.id == null || !this.id.equals(r.id))
                {
                    return false;
                }
            }
            if(this.facet != r.facet)
            {
                if(this.facet == null || r.facet == null || !this.facet.equals(r.facet))
                {
                    return false;
                }
            }
            if(this.operation != r.operation)
            {
                if(this.operation == null || r.operation == null || !this.operation.equals(r.operation))
                {
                    return false;
                }
            }
            if(this.mode != r.mode)
            {
                if(this.mode == null || r.mode == null || !this.mode.equals(r.mode))
                {
                    return false;
                }
            }
            if(this.ctx != r.ctx)
            {
                if(this.ctx == null || r.ctx == null || !this.ctx.equals(r.ctx))
                {
                    return false;
                }
            }
            if(this.requestId != r.requestId)
            {
                return false;
            }
            if(this.encoding != r.encoding)
            {
                if(this.encoding == null || r.encoding == null || !this.encoding.equals(r.encoding))
                {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    public int hashCode()
    {
        int h_ = 5381;
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, "::Ice::Current");
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, adapter);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, con);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, id);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, facet);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, operation);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, mode);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, ctx);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, requestId);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, encoding);
        return h_;
    }

    public Current clone()
    {
        Current c = null;
        try
        {
            c = (Current)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    /** @hidden */
    public static final long serialVersionUID = -6178374122886477055L;
}
