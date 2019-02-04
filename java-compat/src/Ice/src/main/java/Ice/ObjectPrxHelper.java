//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package Ice;

/**
 * Base class for all proxy helpers.
 **/
public class ObjectPrxHelper extends ObjectPrxHelperBase
{
    /**
     * Casts a proxy to {@link Ice.ObjectPrx}. This call contacts
     * the server and will throw an Ice run-time exception if the target
     * object does not exist or the server cannot be reached.
     *
     * @param b The proxy to cast to @{link Ice.ObjectPrx}.
     * @return <code>b</code>.
     **/
    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b)
    {
        return b;
    }

    /**
     * Casts a proxy to {@link Ice.ObjectPrx}. This call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist or the server cannot be reached.
     *
     * @param b The proxy to cast to {@link Ice.ObjectPrx}.
     * @param ctx The <code>Context</code> map for the invocation.
     * @return <code>b</code>.
     **/
    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, java.util.Map<String, String> ctx)
    {
        return b;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist, the specified facet does not exist, or the server cannot be reached.
     *
     * @param b The proxy to cast to {@link Ice.ObjectPrx}.
     * @param f The facet for the new proxy.
     * @return The new proxy with the specified facet.
     **/
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
                h._copyFrom(bb);
                d = h;
            }
            catch(Ice.FacetNotExistException ex)
            {
            }
        }
        return d;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call contacts
     * the server and throws an Ice run-time exception if the target
     * object does not exist, the specified facet does not exist, or the server cannot be reached.
     *
     * @param b The proxy to cast to {@link Ice.ObjectPrx}.
     * @param f The facet for the new proxy.
     * @param ctx The <code>Context</code> map for the invocation.
     * @return The new proxy with the specified facet.
     **/
    public static ObjectPrx
    checkedCast(Ice.ObjectPrx b, String f, java.util.Map<String, String> ctx)
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
                h._copyFrom(bb);
                d = h;
            }
            catch(Ice.FacetNotExistException ex)
            {
            }
        }
        return d;
    }

    /**
     * Casts a proxy to {@link Ice.ObjectPrx}. This call does
     * not contact the server and always succeeds.
     *
     * @param b The proxy to cast to {@link Ice.ObjectPrx}.
     * @return <code>b</code>.
     **/
    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b)
    {
        return b;
    }

    /**
     * Creates a new proxy that is identical to the passed proxy, except
     * for its facet. This call does not contact the server and always succeeds.
     *
     * @param b The proxy to cast to {@link Ice.ObjectPrx}.
     * @param f The facet for the new proxy.
     * @return The new proxy with the specified facet.
     **/
    public static ObjectPrx
    uncheckedCast(Ice.ObjectPrx b, String f)
    {
        ObjectPrx d = null;
        if(b != null)
        {
            Ice.ObjectPrx bb = b.ice_facet(f);
            ObjectPrxHelper h = new ObjectPrxHelper();
            h._copyFrom(bb);
            d = h;
        }
        return d;
    }

    /**
     * Returns the Slice type id of the interface or class associated
     * with this proxy class.
     *
     * @return the type id, "::Ice::Object"
     **/
    public static String
    ice_staticId()
    {
        return Ice.ObjectImpl.ice_staticId();
    }
}
