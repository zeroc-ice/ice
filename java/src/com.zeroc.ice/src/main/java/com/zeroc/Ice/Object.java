// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

/** The base interface for servants. */
public interface Object {
    /**
     * @hidden
     */
    static final String[] _iceIds = {"::Ice::Object"};

    /** Holds the results of a call to <code>ice_invoke</code>. */
    public class Ice_invokeResult {
        /** Default initializes the members. */
        public Ice_invokeResult() {}

        /**
         * One-shot constructor to initialize the members.
         *
         * @param returnValue True for a successful invocation with any results encoded in <code>
         *     outParams</code>. False if a user exception occurred with the exception encoded in
         *     <code>
         *     outParams</code>.
         * @param outParams The encoded results.
         */
        public Ice_invokeResult(boolean returnValue, byte[] outParams) {
            this.returnValue = returnValue;
            this.outParams = outParams;
        }

        /**
         * If the operation completed successfully, the return value is <code>true</code>. If the
         * operation raises a user exception, the return value is <code>false</code>; in this case,
         * <code>outParams</code> contains the encoded user exception. If the operation raises a
         * run-time exception, it throws it directly.
         */
        public boolean returnValue;

        /**
         * The encoded out-parameters and return value for the operation. The return value follows
         * any out-parameters.
         */
        public byte[] outParams;
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param s The type ID of the Slice interface to test against.
     * @param current The {@link Current} object for the invocation.
     * @return <code>true</code> if this object has the interface specified by <code>s</code> or
     *     derives from the interface specified by <code>s</code>.
     */
    default boolean ice_isA(String s, Current current) {
        return java.util.Arrays.binarySearch(ice_ids(current), s) >= 0;
    }

    /**
     * Tests whether this object can be reached.
     *
     * @param current The {@link Current} object for the invocation.
     */
    default void ice_ping(Current current) {
        // Nothing to do.
    }

    /**
     * Returns the Slice type IDs of the interfaces supported by this object.
     *
     * @param current The {@link Current} object for the invocation.
     * @return The Slice type IDs of the interfaces supported by this object, in alphabetical order.
     */
    default String[] ice_ids(Current current) {
        return _iceIds;
    }

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @param current The {@link Current} object for the invocation.
     * @return The Slice type ID of the most-derived interface.
     */
    default String ice_id(Current current) {
        return ice_staticId();
    }

    /**
     * Returns the Slice type ID of the interface supported by this object.
     *
     * @return The return value is always ::Ice::Object.
     */
    public static String ice_staticId() {
        return _iceIds[0];
    }

    /**
     * Dispatches an incoming request and returns the corresponding outgoing response.
     *
     * @param request The incoming request.
     * @return The outgoing response.
     */
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request)
            throws UserException {
        return switch (request.current.operation) {
            case "ice_id" -> _iceD_ice_id(this, request);
            case "ice_ids" -> _iceD_ice_ids(this, request);
            case "ice_isA" -> _iceD_ice_isA(this, request);
            case "ice_ping" -> _iceD_ice_ping(this, request);
            default -> throw new OperationNotExistException();
        };
    }

    /**
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_isA(Object obj, IncomingRequest request) {
        InputStream istr = request.inputStream;
        istr.startEncapsulation();
        String iceP_id = istr.readString();
        istr.endEncapsulation();
        boolean ret = obj.ice_isA(iceP_id, request.current);
        return CompletableFuture.completedFuture(
                request.current.createOutgoingResponse(
                        ret, (ostr, value) -> ostr.writeBool(value), FormatType.CompactFormat));
    }

    /**
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_ping(Object obj, IncomingRequest request) {
        request.inputStream.skipEmptyEncapsulation();
        obj.ice_ping(request.current);
        return CompletableFuture.completedFuture(request.current.createEmptyOutgoingResponse());
    }

    /**
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_ids(Object obj, IncomingRequest request) {
        request.inputStream.skipEmptyEncapsulation();
        String[] ret = obj.ice_ids(request.current);
        return CompletableFuture.completedFuture(
                request.current.createOutgoingResponse(
                        ret,
                        (ostr, value) -> ostr.writeStringSeq(value),
                        FormatType.CompactFormat));
    }

    /**
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_id(Object obj, IncomingRequest request) {
        request.inputStream.skipEmptyEncapsulation();
        String ret = obj.ice_id(request.current);
        return CompletableFuture.completedFuture(
                request.current.createOutgoingResponse(
                        ret, (ostr, value) -> ostr.writeString(value), FormatType.CompactFormat));
    }
}
