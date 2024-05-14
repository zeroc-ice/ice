// Copyright (c) ZeroC, Inc.

#nullable enable

using System.ComponentModel;
using System.Diagnostics;

namespace Ice;

/// <summary>
/// The base interface for servants.
/// </summary>
[SliceTypeId("::Ice::Object")]
public interface Object
{
    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    ///
    /// <param name="s">The type ID of the Slice interface to test against.</param>
    /// <param name="current">The Current object for the dispatch.</param>
    /// <returns>True if this object has the interface
    /// specified by s or derives from the interface specified by s.</returns>
    public bool ice_isA(string s, Current current)
    {
        foreach (Type type in GetType().GetInterfaces())
        {
            if (type.GetSliceTypeId() is string typeId && typeId == s)
            {
                return true;
            }
        }
        return false;
    }

    /// <summary>
    /// Tests whether this object can be reached.
    /// </summary>
    /// <param name="current">The Current object for the dispatch.</param>
    public void ice_ping(Current current)
    {
        // does nothing
    }

    /// <summary>
    /// Returns the Slice type IDs of the interfaces supported by this object.
    /// </summary>
    /// <param name="current">The Current object for the dispatch.</param>
    /// <returns>The Slice type IDs of the interfaces supported by this object, in alphabetical order.</returns>
    public string[] ice_ids(Current current)
    {
        var sortedSet = new SortedSet<string>(StringComparer.Ordinal);
        foreach (Type type in GetType().GetInterfaces())
        {
            if (type.GetSliceTypeId() is string typeId)
            {
                sortedSet.Add(typeId);
            }
        }
        return sortedSet.ToArray();
    }

    /// <summary>
    /// Returns the Slice type ID of the most-derived interface supported by this object.
    /// </summary>
    /// <param name="current">The Current object for the dispatch.</param>
    /// <returns>The Slice type ID of the most-derived interface.</returns>
    public string ice_id(Current current) => throw new NotImplementedException();

    Task<OutputStream>? iceDispatch(Ice.Internal.Incoming inc, Current current);

    public ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request) =>
        throw new NotImplementedException();
}

/// <summary>
/// Base class for all Slice classes.
/// </summary>
public abstract class ObjectImpl : Object
{
    /// <summary>
    /// Instantiates an Ice object.
    /// </summary>
    public ObjectImpl()
    {
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public static Task<OutputStream>? iceD_ice_isA(Object obj, Ice.Internal.Incoming inS, Current current)
    {
        InputStream istr = inS.startReadParams();
        var id = istr.readString();
        inS.endReadParams();
        var ret = obj.ice_isA(id, current);
        var ostr = inS.startWriteParams();
        ostr.writeBool(ret);
        inS.endWriteParams(ostr);
        inS.setResult(ostr);
        return null;
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public static Task<OutputStream>? iceD_ice_ping(Object obj, Ice.Internal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        obj.ice_ping(current);
        inS.setResult(inS.writeEmptyParams());
        return null;
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    public static Task<OutputStream>? iceD_ice_ids(Object obj, Ice.Internal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        var ret = obj.ice_ids(current);
        var ostr = inS.startWriteParams();
        ostr.writeStringSeq(ret);
        inS.endWriteParams(ostr);
        inS.setResult(ostr);
        return null;
    }

    /// <summary>
    /// Returns the Slice type ID of the most-derived interface supported by this object.
    /// </summary>
    /// <param name="current">The Current object for the dispatch.</param>
    /// <returns>The return value is always ::Ice::Object.</returns>
    public virtual string ice_id(Current current) => ice_staticId();

    [EditorBrowsable(EditorBrowsableState.Never)]
    public static Task<OutputStream>? iceD_ice_id(Object obj, Ice.Internal.Incoming inS, Current current)
    {
        inS.readEmptyParams();
        var ret = obj.ice_id(current);
        var ostr = inS.startWriteParams();
        ostr.writeString(ret);
        inS.endWriteParams(ostr);
        inS.setResult(ostr);
        return null;
    }

    /// <summary>
    /// Returns the Slice type ID of the interface supported by this object.
    /// </summary>
    /// <returns>The return value is always ::Ice::Object.</returns>
    public static string ice_staticId() => "::Ice::Object";

    private static readonly string[] _all = new string[]
    {
        "ice_id", "ice_ids", "ice_isA", "ice_ping"
    };

    [EditorBrowsable(EditorBrowsableState.Never)]
    public virtual Task<OutputStream>? iceDispatch(Ice.Internal.Incoming inc, Current current)
    {
        int pos = Array.BinarySearch(_all, current.operation);
        if (pos < 0)
        {
            throw new OperationNotExistException(current.id, current.facet, current.operation);
        }

        switch (pos)
        {
            case 0:
            {
                return iceD_ice_id(this, inc, current);
            }
            case 1:
            {
                return iceD_ice_ids(this, inc, current);
            }
            case 2:
            {
                return iceD_ice_isA(this, inc, current);
            }
            case 3:
            {
                return iceD_ice_ping(this, inc, current);
            }
        }

        Debug.Assert(false);
        throw new OperationNotExistException(current.id, current.facet, current.operation);
    }

    private static string operationModeToString(OperationMode mode)
    {
        if (mode == OperationMode.Normal)
        {
            return "::Ice::Normal";
        }
        if (mode == OperationMode.Nonmutating)
        {
            return "::Ice::Nonmutating";
        }

        if (mode == OperationMode.Idempotent)
        {
            return "::Ice::Idempotent";
        }

        return "???";
    }

    public static void iceCheckMode(OperationMode expected, OperationMode received)
    {
        Debug.Assert(expected != OperationMode.Nonmutating); // We never expect Nonmutating
        if (expected != received)
        {
            if (expected == OperationMode.Idempotent && received == OperationMode.Nonmutating)
            {
                // Fine: typically an old client still using the deprecated nonmutating keyword
            }
            else
            {
                MarshalException ex = new MarshalException();
                ex.reason = "unexpected operation mode. expected = " + operationModeToString(expected) +
                    " received = " + operationModeToString(received);
                throw ex;
            }
        }
    }
}

/// <summary>
/// Base class for dynamic dispatch servants. A server application
/// derives a concrete servant class from Blobject that
/// implements the Blobject.ice_invoke method.
/// </summary>
public abstract class Blobject : ObjectImpl
{
    /// <summary>
    /// Dispatch an incoming request.
    /// </summary>
    /// <param name="inParams">The encoded in-parameters for the operation.</param>
    /// <param name="outParams">The encoded out-parameters and return value
    /// for the operation. The return value follows any out-parameters.</param>
    /// <param name="current">The Current object to pass to the operation.</param>
    /// <returns>If the operation completed successfully, the return value
    /// is true. If the operation raises a user exception,
    /// the return value is false; in this case, outParams
    /// must contain the encoded user exception. If the operation raises an
    /// Ice run-time exception, it must throw it directly.</returns>
    public abstract bool ice_invoke(byte[] inParams, out byte[] outParams, Current current);

    [EditorBrowsable(EditorBrowsableState.Never)]
    public override Task<OutputStream>? iceDispatch(Ice.Internal.Incoming inS, Current current)
    {
        byte[] inEncaps = inS.readParamEncaps();
        byte[] outEncaps;
        bool ok = ice_invoke(inEncaps, out outEncaps, current);
        inS.setResult(inS.writeParamEncaps(inS.getAndClearCachedOutputStream(), outEncaps, ok));
        return null;
    }
}

public abstract class BlobjectAsync : ObjectImpl
{
    public abstract Task<Ice.Object_Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current);

    [EditorBrowsable(EditorBrowsableState.Never)]
    public override Task<Ice.OutputStream> iceDispatch(Ice.Internal.Incoming inS, Current current)
    {
        byte[] inEncaps = inS.readParamEncaps();
        var task = ice_invokeAsync(inEncaps, current);
        var cached = inS.getAndClearCachedOutputStream();
        return task.ContinueWith((Task<Object_Ice_invokeResult> t) =>
        {
            var ret = t.GetAwaiter().GetResult();
            return Task.FromResult(inS.writeParamEncaps(cached, ret.outEncaps, ret.returnValue));
        }).Unwrap();
    }
}
