// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents a request received by a connection. It's the argument to <see cref="Object.dispatchAsync" />.
/// </summary>
public sealed class IncomingRequest
{
    /// <summary>
    /// Gets the current object of the request.
    /// </summary>
    public Current current { get; }

    /// <summary>
    /// Gets the input stream buffer of the request.
    /// </summary>
    public InputStream inputStream { get; }

    /// <summary>
    /// Gets the number of bytes in the request.
    /// </summary>
    /// <value>The number of bytes in the request. These are all the bytes starting with the identity of the target
    /// object.</value>
    public int size { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="IncomingRequest" /> class.
    /// </summary>
    /// <param name="requestId">The request ID. It's 0 for oneway requests.</param>
    /// <param name="connection">The connection that received the request. It's null for collocated invocations.</param>
    /// <param name="adapter">The object adapter to set in current.</param>
    /// <param name="inputStream">The input stream buffer over the incoming Ice protocol request message. The stream is
    /// positioned at the beginning of the request header - the next data to read is the identity of the target object.
    /// </param>
    public IncomingRequest(int requestId, Connection? connection, ObjectAdapter adapter, InputStream inputStream)
    {
        this.inputStream = inputStream;

        // Read everything else from the input stream
        int start = inputStream.pos();
        var identity = new Identity(inputStream);

        string facet = "";
        string[] facetPath = inputStream.readStringSeq();
        if (facetPath.Length > 0)
        {
            if (facetPath.Length > 1)
            {
                throw new MarshalException($"Received invalid facet path with {facetPath.Length} elements.");
            }
            facet = facetPath[0];
        }
        string operation = inputStream.readString();
        var mode = (OperationMode)inputStream.readByte();
        var ctx = new Dictionary<string, string>();
        int sz = inputStream.readSize();
        while (sz-- > 0)
        {
            string first = inputStream.readString();
            string second = inputStream.readString();
            ctx[first] = second;
        }

        int encapsulationSize = inputStream.readInt();
        var encoding = new EncodingVersion(inputStream);

        current = new Current(adapter, connection, identity, facet, operation, mode, ctx, requestId, encoding);

        // Rewind to the start of the encapsulation
        inputStream.pos(inputStream.pos() - 6);

        size = inputStream.pos() - start + encapsulationSize;
    }
}
