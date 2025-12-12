// Copyright (c) ZeroC, Inc.

#nullable enable

using System.ComponentModel;

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
    /// <remarks>If an exception is thrown, Ice will marshal it as the response payload.</remarks>
    public ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request) =>
        request.current.operation switch
        {
            "ice_id" => iceD_ice_idAsync(this, request),
            "ice_ids" => iceD_ice_idsAsync(this, request),
            "ice_isA" => iceD_ice_isAAsync(this, request),
            "ice_ping" => iceD_ice_pingAsync(this, request),
            _ => throw new OperationNotExistException()
        };

    /// <summary>
    /// Tests whether this object supports a specific Slice interface.
    /// </summary>
    ///
    /// <param name="s">The type ID of the Slice interface to test against.</param>
    /// <param name="current">The Current object of the incoming request.</param>
    /// <returns><see langword="true"/> if this object implements the Slice interface specified by <paramref name="s"/>
    /// or implements a derived interface, <see langword="false"/> otherwise.</returns>
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
    /// <param name="current">The Current object of the incoming request.</param>
    public void ice_ping(Current current)
    {
        // does nothing
    }

    /// <summary>
    /// Returns the Slice interfaces supported by this object as a list of Slice type IDs.
    /// </summary>
    /// <param name="current">The Current object of the incoming request.</param>
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
    /// Returns the type ID of the most-derived Slice interface supported by this object.
    /// </summary>
    /// <param name="current">The Current object of the incoming request.</param>
    /// <returns>The Slice type ID of the most-derived interface.</returns>
    public string ice_id(Current current) => ice_staticId();

    /// <summary>
    /// Returns the type ID of the associated Slice interface.
    /// </summary>
    /// <returns>The return value is always <c>"::Ice::Object"</c>.</returns>
    public static string ice_staticId() => "::Ice::Object";

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
/// Base class for dynamic dispatch servants.
/// </summary>
/// <remarks>This class is provided for backward compatibility. You should consider deriving directly from
/// <see cref="Object"/> and overriding the <see cref="Object.dispatchAsync"/> method.</remarks>
public abstract class Blobject : Object
{
    /// <summary>
    /// Dispatches an incoming request.
    /// </summary>
    /// <param name="inParams">An encapsulation containing the encoded in-parameters for the operation.</param>
    /// <param name="outParams">An encapsulation containing the encoded result for the operation.</param>
    /// <param name="current">The Current object of the incoming request.</param>
    /// <returns><see langword="true"/> if the dispatch completes successfully, <see langword="false"/> if the dispatch
    /// completes with a user exception encoded in <paramref name="outParams"/>.</returns>
    public abstract bool ice_invoke(byte[] inParams, out byte[] outParams, Current current);

    public ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        byte[] inEncaps = request.inputStream.readEncapsulation(out _);
        bool ok = ice_invoke(inEncaps, out byte[] outEncaps, request.current);
        return new(request.current.createOutgoingResponse(ok, outEncaps));
    }
}

/// <summary>
/// Base class for asynchronous dynamic dispatch servants.
/// </summary>
/// <remarks>This class is provided for backward compatibility. You should consider deriving directly from
/// <see cref="Object"/> and overriding the <see cref="Object.dispatchAsync"/> method.</remarks>
public abstract class BlobjectAsync : Object
{
    /// <summary>
    /// Dispatches an incoming request.
    /// </summary>
    /// <param name="inParams">An encapsulation containing the encoded in-parameters for the operation.</param>
    /// <param name="outParams">An encapsulation containing the encoded result for the operation.</param>
    /// <param name="current">The Current object of the incoming request.</param>
    /// <returns>A task that will complete with <see langword="true"/> if the dispatch completes successfully and
    /// <see langword="false"/> if the dispatch completes with a user exception encoded in
    /// <paramref name="outParams"/>.</returns>
    public abstract Task<Object_Ice_invokeResult> ice_invokeAsync(byte[] inEncaps, Current current);

    public async ValueTask<OutgoingResponse> dispatchAsync(IncomingRequest request)
    {
        byte[] inEncaps = request.inputStream.readEncapsulation(out _);
        Object_Ice_invokeResult result = await ice_invokeAsync(inEncaps, request.current).ConfigureAwait(false);
        return request.current.createOutgoingResponse(result.returnValue, result.outEncaps);
    }
}
