//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A collection of Active Connection Management configuration settings.
 **/
public class ACM implements java.lang.Cloneable
{
    /**
     * A timeout value in seconds.
     **/
    public int timeout;

    /**
     * The close semantics.
     **/
    public ACMClose close;

    /**
     * The heartbeat semantics.
     **/
    public ACMHeartbeat heartbeat;

    public ACM()
    {
        this.close = ACMClose.CloseOff;
        this.heartbeat = ACMHeartbeat.HeartbeatOff;
    }

    public ACM(int timeout, ACMClose close, ACMHeartbeat heartbeat)
    {
        this.timeout = timeout;
        this.close = close;
        this.heartbeat = heartbeat;
    }

    public boolean equals(java.lang.Object rhs)
    {
        if(this == rhs)
        {
            return true;
        }
        ACM r = null;
        if(rhs instanceof ACM)
        {
            r = (ACM)rhs;
        }

        if(r != null)
        {
            if(this.timeout != r.timeout)
            {
                return false;
            }
            if(this.close != r.close)
            {
                if(this.close == null || r.close == null || !this.close.equals(r.close))
                {
                    return false;
                }
            }
            if(this.heartbeat != r.heartbeat)
            {
                if(this.heartbeat == null || r.heartbeat == null || !this.heartbeat.equals(r.heartbeat))
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
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, "::Ice::ACM");
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, timeout);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, close);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, heartbeat);
        return h_;
    }

    public ACM clone()
    {
        ACM c = null;
        try
        {
            c = (ACM)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    /** @hidden */
    public static final long serialVersionUID = -969527045191053991L;
}
