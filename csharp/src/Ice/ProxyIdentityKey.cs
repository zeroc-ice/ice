// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// This class allows a proxy to be used as the key for a hashed collection.
/// The GetHashCode, Equals, and Compare methods are based on the object identity
/// of the proxy.
/// </summary>
public class ProxyIdentityKey : System.Collections.IEqualityComparer, System.Collections.IComparer
{
    /// <summary>
    /// Computes a hash value based on the object identity of the proxy.
    /// </summary>
    /// <param name="obj">The proxy whose hash value to compute.</param>
    /// <returns>The hash value for the proxy based on the identity.</returns>
    public int GetHashCode(object obj) => ((ObjectPrx)obj).ice_getIdentity().GetHashCode();

    /// <summary>
    /// Compares two proxies for equality.
    /// </summary>
    /// <param name="x">The first proxy to compare.</param>
    /// <param name="y">The second proxy to compare.</param>
    /// <returns>True if the passed proxies have the same object identity; false, otherwise.</returns>
    public new bool Equals(object? x, object? y)
    {
        try
        {
            return Compare(x, y) == 0;
        }
        catch (System.Exception)
        {
            return false;
        }
    }

    /// <summary>
    /// Compares two proxies using the object identity for comparison.
    /// </summary>
    /// <param name="x">The first proxy to compare.</param>
    /// <param name="y">The second proxy to compare.</param>
    /// <returns>Less than zero if x is less than y; greater than zero if x is greater than y; otherwise zero.</returns>
    public int Compare(object? x, object? y)
    {
        var proxy1 = x as ObjectPrx;
        if (x is not null && proxy1 is null)
        {
            throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", nameof(x));
        }

        var proxy2 = y as ObjectPrx;
        if (y is not null && proxy2 is null)
        {
            throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", nameof(y));
        }
        return Util.proxyIdentityCompare(proxy1, proxy2);
    }
}

/// <summary>
/// This class allows a proxy to be used as the key for a hashed collection.
/// The GetHashCode, Equals, and Compare methods are based on the object identity and
/// the facet of the proxy.
/// </summary>
public class ProxyIdentityFacetKey : System.Collections.IEqualityComparer, System.Collections.IComparer
{
    /// <summary>
    /// Computes a hash value based on the object identity and facet of the proxy.
    /// </summary>
    /// <param name="obj">The proxy whose hash value to compute.</param>
    /// <returns>The hash value for the proxy based on the identity and facet.</returns>
    public int GetHashCode(object obj)
    {
        var o = (ObjectPrx)obj;
        return HashCode.Combine(o.ice_getIdentity(), o.ice_getFacet());
    }

    /// <summary>
    /// Compares two proxies for equality.
    /// </summary>
    /// <param name="x">The first proxy to compare.</param>
    /// <param name="y">The second proxy to compare.</param>
    /// <returns>True if the passed proxies have the same object
    /// identity and facet; false, otherwise.</returns>
    public new bool Equals(object? x, object? y)
    {
        try
        {
            return Compare(x, y) == 0;
        }
        catch (System.Exception)
        {
            return false;
        }
    }

    /// <summary>
    /// Compares two proxies using the object identity and facet for comparison.
    /// </summary>
    /// <param name="x">The first proxy to compare.</param>
    /// <param name="y">The second proxy to compare.</param>
    /// <returns>&lt; 0 if x is less than y; &gt; 0 if x is greater than y;
    /// 0, otherwise.</returns>
    public int Compare(object? x, object? y)
    {
        var proxy1 = x as ObjectPrx;
        if (x is not null && proxy1 is null)
        {
            throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", nameof(x));
        }

        var proxy2 = y as ObjectPrx;
        if (y is not null && proxy2 is null)
        {
            throw new ArgumentException("Argument must be derived from Ice.ObjectPrx", nameof(y));
        }
        return Util.proxyIdentityAndFacetCompare(proxy1, proxy2);
    }
}
