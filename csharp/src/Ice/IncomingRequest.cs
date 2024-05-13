// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents a request received by a connection. It's the argument to the dispatchAsync method on Object.
/// </summary>
public sealed class IncomingRequest
{
    /// <summary>
    /// Gets the current object for the request.
    /// </summary>
    public Current current { get; }

    /// <summary>
    /// Gets the incoming stream buffer of the request.
    /// </summary>
    public InputStream inputStream { get; }

    /// <summary>
    /// Gets the number of bytes in the request.
    /// </summary>
    /// <value>The number of bytes in the request. These are all the bytes starting with the identity of the target.
    /// </value>
    public int requestSize { get; }

    /// <summary>
    /// Constructs an incoming request.
    /// </summary>
    /// <param name="requestId">The request ID. It's 0 for oneway requests.</param>
    /// <param name="connection">The connection that received the request. It's null for collocated invocations.</param>
    /// <param name="adapter">The object adapter to set in current.</param>
    /// <param name="inputStream">The input stream buffer over the incoming Ice protocol request message. The stream is
    /// positioned at the beginning of the request header - the next data to read is the identity of the target </param>
    public IncomingRequest(int requestId, Connection? connection, ObjectAdapter adapter, InputStream inputStream)
    {
        this.inputStream = inputStream;

        current = new Current();
        current.adapter = adapter;
        current.con = connection;
        current.requestId = requestId;

        // Read everything else from the input stream
        int start = inputStream.pos();
        current.id = new Identity(inputStream);

        string[] facetPath = inputStream.readStringSeq();
        if (facetPath.Length > 0)
        {
            if (facetPath.Length > 1)
            {
                throw new Ice.MarshalException();
            }
            current.facet = facetPath[0];
        }
        current.operation = inputStream.readString();
        current.mode = (OperationMode)inputStream.readByte();
        current.ctx = new Dictionary<string, string>();
        int sz = inputStream.readSize();
        while (sz-- > 0)
        {
            string first = inputStream.readString();
            string second = inputStream.readString();
            current.ctx[first] = second;
        }

        int encapsulationSize = inputStream.readInt();
        current.encoding = new EncodingVersion(inputStream);

        // Rewind to the start of the encapsulation
        inputStream.pos(inputStream.pos() - 6);

        requestSize = inputStream.pos() - start + encapsulationSize;
    }
}
