// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public class ObjectPrxHelper extends ObjectPrxHelperBase
{
    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b)
    {
        return b;
    }

    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, java.util.Map ctx)
    {
        return b;
    }

    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, String f)
    {
        ObjectPrx d = null;
        if(b != null)
        {
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                boolean ok = bb.ice_isA("::Ice::Object");
                assert(ok);
                ObjectPrxHelper h = new ObjectPrxHelper();
                h.__copyFrom(bb);
                d = h;
            }
            catch(Ice.FacetNotExistException ex)
            {
            }
        }
        return d;
    }

    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, String f, java.util.Map ctx)
    {
        ObjectPrx d = null;
        if(b != null)
        {
            Ice.ObjectPrx bb = b.ice_facet(f);
            try
            {
                boolean ok = bb.ice_isA("::Ice::Object", ctx);
                assert(ok);
                ObjectPrxHelper h = new ObjectPrxHelper();
                h.__copyFrom(bb);
                d = h;
            }
            catch(Ice.FacetNotExistException ex)
            {
            }
        }
        return d;
    }

    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b)
    {
        return b;
    }

    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b, String f)
    {
        ObjectPrx d = null;
        if(b != null)
        {
            Ice.ObjectPrx bb = b.ice_facet(f);
            ObjectPrxHelper h = new ObjectPrxHelper();
            h.__copyFrom(bb);
            d = h;
        }
        return d;
    }
}
