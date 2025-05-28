// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if a request failed. This exception, and all exceptions derived from
 * {@link DispatchException}, are transmitted by the Ice protocol.
 */
public abstract class RequestFailedException extends DispatchException {
    @Override
    public String ice_id() {
        return "::Ice::RequestFailedException";
    }

    /** The identity of the Ice Object to which the request was sent. */
    public final Identity id;

    /** The facet to which the request was sent. */
    public final String facet;

    /** The operation name of the request. */
    public final String operation;

    // Logically protected too.
    RequestFailedException(ReplyStatus replyStatus) {
        super(replyStatus.value());
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
    }

    // Logically protected too.
    RequestFailedException(ReplyStatus replyStatus, Identity id, String facet, String operation) {
        super(replyStatus.value(), createMessage(replyStatus, id, facet, operation));
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    private static String createMessage(
            ReplyStatus replyStatus, Identity id, String facet, String operation) {
        return "Dispatch failed with "
            + replyStatus
            + " { id = '"
            + Util.identityToString(id)
            + "', facet = '"
            + facet
            + "', operation = '"
            + operation
            + "' }";
    }

    private static final long serialVersionUID = 4181164754424262091L;
}
