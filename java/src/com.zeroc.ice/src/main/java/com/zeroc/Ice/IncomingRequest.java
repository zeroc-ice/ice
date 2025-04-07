// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.HashMap;
import java.util.Map;

/**
 * Represents a request received by a connection. It's the argument of {@link
 * Object#dispatch(IncomingRequest)}.
 */
public final class IncomingRequest {
    /** Gets the current object for the request. */
    public final Current current;

    /** Gets the incoming stream buffer of the request. */
    public final InputStream inputStream;

    /**
     * Gets the number of bytes in the request. These are all the bytes starting with the identity
     * of the target.
     */
    public final int size;

    /**
     * Constructs an incoming request.
     *
     * @param requestId The request ID. It's 0 for oneway requests.
     * @param connection The connection that received the request. It's null for collocated
     *     invocations.
     * @param adapter The object adapter to set in current.
     * @param inputStream The input stream buffer over the incoming Ice protocol request message.
     *     The stream is positioned at the beginning of the request header - the next data to read
     *     is the identity of the target.
     */
    public IncomingRequest(
            int requestId, Connection connection, ObjectAdapter adapter, InputStream inputStream) {
        this.inputStream = inputStream;

        // Read everything else from the input stream
        int start = inputStream.pos();
        var identity = Identity.ice_read(inputStream);

        String facet = "";
        String[] facetPath = inputStream.readStringSeq();
        if (facetPath.length > 0) {
            if (facetPath.length > 1) {
                throw new MarshalException(
                        "Received invalid facet path with " + facetPath.length + " elements.");
            }
            facet = facetPath[0];
        }
        String operation = inputStream.readString();
        OperationMode mode = OperationMode.valueOf(inputStream.readByte());
        Map<String, String> ctx = new HashMap<>();
        int sz = inputStream.readSize();
        while (sz-- > 0) {
            String first = inputStream.readString();
            String second = inputStream.readString();
            ctx.put(first, second);
        }

        int encapsulationSize = inputStream.readInt();
        var encoding = EncodingVersion.ice_read(inputStream);

        current =
                new Current(
                        adapter,
                        connection,
                        identity,
                        facet,
                        operation,
                        mode,
                        ctx,
                        requestId,
                        encoding);

        // Rewind to the start of the encapsulation
        inputStream.pos(inputStream.pos() - 6);

        size = inputStream.pos() - start + encapsulationSize;
    }
}
