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
    /// Dispatches an incoming request and returns the corresponding outgoing response.
    /// </summary>
    /// <param name="request">The incoming request.</param>
    /// <returns>A value task that holds the outgoing response.</returns>
    /// <remarks>Ice marshals any exception thrown by this method into the response.</remarks>
    ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request);

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
    /// Gets the Slice type IDs of the interfaces supported by this object.
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
    /// Gets the Slice type ID of the most-derived interface supported by this object.
    /// </summary>
    /// <param name="current">The Current object for the dispatch.</param>
    /// <returns>The Slice type ID of the most-derived interface.</returns>
    public string ice_id(Current current) => throw new NotImplementedException();

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected static ValueTask<OutgoingResponse> iceD_ice_isAAsync(Object obj, IncomingRequest request)
    {
        InputStream istr = request.inputStream;
        istr.startEncapsulation();
        string iceP_id = istr.readString();
        istr.endEncapsulation();
        bool ret = obj.ice_isA(iceP_id, request.current);
        return new(request.current.createOutgoingResponse(ret, static (ostr, ret) => ostr.writeBool(ret)));
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected static ValueTask<OutgoingResponse> iceD_ice_pingAsync(Object obj, IncomingRequest request)
    {
        InputStream istr = request.inputStream;
        istr.skipEmptyEncapsulation();
        obj.ice_ping(request.current);
        return new(request.current.createEmptyOutgoingResponse());
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected static ValueTask<OutgoingResponse> iceD_ice_idsAsync(Object obj, IncomingRequest request)
    {
        InputStream istr = request.inputStream;
        istr.skipEmptyEncapsulation();
        string[] ret = obj.ice_ids(request.current);
        return new(request.current.createOutgoingResponse(ret, static (ostr, ret) => ostr.writeStringSeq(ret)));
    }

    [EditorBrowsable(EditorBrowsableState.Never)]
    protected static ValueTask<OutgoingResponse> iceD_ice_idAsync(Object obj, IncomingRequest request)
    {
        InputStream istr = request.inputStream;
        istr.skipEmptyEncapsulation();
        string ret = obj.ice_id(request.current);
        return new(request.current.createOutgoingResponse(ret, static (ostr, ret) => ostr.writeString(ret)));
    }
}

/// <summary>
/// Base class for all Slice classes.
/// </summary>
public abstract class ObjectImpl : Object
{
    /// <summary>
    /// Gets the Slice type ID of the interface supported by this object.
    /// </summary>
    /// <returns>The return value is always ::Ice::Object.</returns>
    public static string ice_staticId() => "::Ice::Object";

    /// <inheritdoc />
    public virtual string ice_id(Current current) => ice_staticId();

    public virtual ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request) =>
        request.current.operation switch
        {
            "ice_id" => Object.iceD_ice_idAsync(this, request),
            "ice_ids" => Object.iceD_ice_idsAsync(this, request),
            "ice_isA" => Object.iceD_ice_isAAsync(this, request),
            "ice_ping" => Object.iceD_ice_pingAsync(this, request),
            _ => throw new OperationNotExistException()
        };

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
                throw new MarshalException($"unexpected operation mode: expected = {operationModeToString(expected)} received = {operationModeToString(received)}");
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

    public override ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        byte[] inEncaps = request.inputStream.readEncapsulation(out _);
        bool ok = ice_invoke(inEncaps, out byte[] outEncaps, request.current);
        return new(request.current.createOutgoingResponse(ok, outEncaps));
    }
}

public abstract class BlobjectAsync : ObjectImpl
{
    public abstract Task<Object_Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current);

    public override async ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        byte[] inEncaps = request.inputStream.readEncapsulation(out _);
        Object_Ice_invokeResult result = await ice_invokeAsync(inEncaps, request.current).ConfigureAwait(false);
        return request.current.createOutgoingResponse(result.returnValue, result.outEncaps);
    }
}
