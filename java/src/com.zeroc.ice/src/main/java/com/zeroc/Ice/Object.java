// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.util.SortedSet;
import java.util.TreeSet;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

/** The base interface for servants. */
@SliceTypeId(value = "::Ice::Object")
public interface Object {
    /**
     * Holds the results of a call to {@code ice_invoke}.
     *
     * @see Blobject#ice_invoke
     * @see ObjectPrx#ice_invoke
     */
    public class Ice_invokeResult {
        /** Default initializes the fields. */
        public Ice_invokeResult() {}

        /**
         * Primary constructor to initialize the fields.
         *
         * @param returnValue {@code true} for a successful invocation (any results are encoded in {@code outParams});
         *     {@code false} if a user exception occurred (the exception is encoded in {@code outParams}).
         * @param outParams the encoded results of the operation
         */
        public Ice_invokeResult(boolean returnValue, byte[] outParams) {
            this.returnValue = returnValue;
            this.outParams = outParams;
        }

        /**
         * {@code true} if the operation completed successfully, {@code false} if the operation threw a user exception.
         * If {@code false}, {@code outParams} contains the encoded user exception.
         * If the operation raises a run-time exception, it throws it directly.
         */
        public boolean returnValue;

        /**
         * The encoded out-parameters and return value for the operation (in that order).
         */
        public byte[] outParams;
    }

    /**
     * Tests whether this object supports a specific Slice interface.
     *
     * @param typeId the type ID of the Slice interface to test against
     * @param current the {@link Current} object of the incoming request
     * @return {@code true} if this object implements the Slice interface specified by {@code typeId} or
     *     implements a derived interface, {@code false} otherwise
     */
    default boolean ice_isA(String typeId, Current current) {

        return isA(this.getClass(), typeId);
    }

    /**
     * Tests whether this object can be reached.
     *
     * @param current the {@link Current} object of the incoming request
     */
    default void ice_ping(Current current) {
        // Nothing to do.
    }

    /**
     * Returns the Slice interfaces supported by this object as a list of Slice type IDs.
     *
     * @param current the {@link Current} object of the incoming request
     * @return the Slice type IDs of the interfaces supported by this object, in alphabetical order
     */
    default String[] ice_ids(Current current) {
        var typeIds = new TreeSet<String>();
        addTypeIds(this.getClass(), typeIds);
        return typeIds.toArray(new String[0]);
    }

    /**
     * Returns the type ID of the most-derived Slice interface supported by this object.
     *
     * @param current the {@link Current} object of the incoming request
     * @return the Slice type ID of the most-derived interface
     */
    default String ice_id(Current current) {
        // The first direct interface with a type ID is necessarily the most derived interface.
        for (var directInterface : this.getClass().getInterfaces()) {
            var sliceTypeIdAnnotation = directInterface.getAnnotation(SliceTypeId.class);
            if (sliceTypeIdAnnotation != null) {
                return sliceTypeIdAnnotation.value();
            }
        }
        return ice_staticId();
    }

    /**
     * Returns the type ID of the associated Slice interface.
     *
     * @return The return value is always {@code "::Ice::Object"}.
     */
    public static String ice_staticId() {
        return "::Ice::Object";
    }

    /**
     * Dispatches an incoming request and returns the corresponding outgoing response.
     *
     * @param request the incoming request
     * @return a {@link CompletionStage} that will complete with the outgoing response
     * @throws UserException If a {@code UserException} is thrown, Ice will marshal it as the response payload.
     */
    default CompletionStage<OutgoingResponse> dispatch(IncomingRequest request) throws UserException {
        return switch (request.current.operation) {
            case "ice_id" -> _iceD_ice_id(this, request);
            case "ice_ids" -> _iceD_ice_ids(this, request);
            case "ice_isA" -> _iceD_ice_isA(this, request);
            case "ice_ping" -> _iceD_ice_ping(this, request);
            default -> throw new OperationNotExistException();
        };
    }

    /**
     * Dispatches an incoming request and returns a corresponding outgoing response for the {@link ice_isA} operation.
     *
     * @param obj the object to dispatch the request to
     * @param request the incoming request
     * @return a {@link CompletionStage} that will complete with the outgoing response
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
     * Dispatches an incoming request and returns a corresponding outgoing response for the {@link ice_ping} operation.
     *
     * @param obj the object to dispatch the request to
     * @param request the incoming request
     * @return a {@link CompletionStage} that will complete with the outgoing response
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_ping(Object obj, IncomingRequest request) {
        request.inputStream.skipEmptyEncapsulation();
        obj.ice_ping(request.current);
        return CompletableFuture.completedFuture(request.current.createEmptyOutgoingResponse());
    }

    /**
     * Dispatches an incoming request and returns a corresponding outgoing response for the {@link ice_ids} operation.
     *
     * @param obj the object to dispatch the request to
     * @param request the incoming request
     * @return a {@link CompletionStage} that will complete with the outgoing response
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
     * Dispatches an incoming request and returns a corresponding outgoing response for the {@link ice_id} operation.
     *
     * @param obj the object to dispatch the request to
     * @param request the incoming request
     * @return a {@link CompletionStage} that will complete with the outgoing response
     * @hidden
     */
    static CompletionStage<OutgoingResponse> _iceD_ice_id(Object obj, IncomingRequest request) {
        request.inputStream.skipEmptyEncapsulation();
        String ret = obj.ice_id(request.current);
        return CompletableFuture.completedFuture(
            request.current.createOutgoingResponse(
                ret, (ostr, value) -> ostr.writeString(value), FormatType.CompactFormat));
    }

    /** Implements {@link ice_isA} by checking all interfaces recursively. */
    private static boolean isA(Class<?> type, String typeId) {
        for (var directInterface : type.getInterfaces()) {
            var sliceTypeIdAnnotation = directInterface.getAnnotation(SliceTypeId.class);
            if (sliceTypeIdAnnotation != null) {
                if (sliceTypeIdAnnotation.value().equals(typeId)) {
                    return true;
                }
            }
            // Check the interfaces of this interface, if any.
            if (isA(directInterface, typeId)) {
                return true;
            }
        }
        return false;
    }

    /** Helper for ice_ids. */
    private static void addTypeIds(Class<?> type, SortedSet<String> typeIds) {
        for (var directInterface : type.getInterfaces()) {
            var sliceTypeIdAnnotation = directInterface.getAnnotation(SliceTypeId.class);
            if (sliceTypeIdAnnotation != null) {
                typeIds.add(sliceTypeIdAnnotation.value());
            }
            // Recursively add the type IDs of the interfaces of this interface, if any.
            addTypeIds(directInterface, typeIds);
        }
    }
}
