//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * A version structure for the encoding version.
 **/
public class EncodingVersion implements java.lang.Cloneable,
                                        java.io.Serializable
{
    public byte major;
    public byte minor;

    public EncodingVersion()
    {
    }

    public EncodingVersion(byte major, byte minor)
    {
        this.major = major;
        this.minor = minor;
    }

    public boolean equals(java.lang.Object rhs)
    {
        if(this == rhs)
        {
            return true;
        }
        EncodingVersion r = null;
        if(rhs instanceof EncodingVersion)
        {
            r = (EncodingVersion)rhs;
        }

        if(r != null)
        {
            if(this.major != r.major)
            {
                return false;
            }
            if(this.minor != r.minor)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    public int hashCode()
    {
        int h_ = 5381;
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, "::Ice::EncodingVersion");
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, major);
        h_ = com.zeroc.IceInternal.HashUtil.hashAdd(h_, minor);
        return h_;
    }

    public EncodingVersion clone()
    {
        EncodingVersion c = null;
        try
        {
            c = (EncodingVersion)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // impossible
        }
        return c;
    }

    public void ice_writeMembers(OutputStream ostr)
    {
        ostr.writeByte(this.major);
        ostr.writeByte(this.minor);
    }

    public void ice_readMembers(InputStream istr)
    {
        this.major = istr.readByte();
        this.minor = istr.readByte();
    }

    static public void ice_write(OutputStream ostr, EncodingVersion v)
    {
        if(v == null)
        {
            _nullMarshalValue.ice_writeMembers(ostr);
        }
        else
        {
            v.ice_writeMembers(ostr);
        }
    }

    static public EncodingVersion ice_read(InputStream istr)
    {
        EncodingVersion v = new EncodingVersion();
        v.ice_readMembers(istr);
        return v;
    }

    private static final EncodingVersion _nullMarshalValue = new EncodingVersion();

    /** @hidden */
    public static final long serialVersionUID = 4211648668174918225L;
}
