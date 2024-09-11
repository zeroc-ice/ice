// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if a request failed. This exception, and all exceptions derived from
 * {@link RequestFailedException}, are transmitted by the Ice protocol, even though they are
 * declared <code>local</code>.
 */
public class RequestFailedException extends LocalException {
    public RequestFailedException(String typename) {
        super("Dispatch failed with " + typename + ".");
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
    }

    public RequestFailedException(String typename, Identity id, String facet, String operation) {
        super(createMessage(typename, id, facet, operation));
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    public String ice_id() {
        return "::Ice::RequestFailedException";
    }

    static String createMessage(String typeName, Identity id, String facet, String operation) {
        return "Dispatch failed with "
                + typeName
                + " { id = '"
                + Util.identityToString(id)
                + "', facet = '"
                + facet
                + "', operation = '"
                + operation
                + "' }";
    }

    /** The identity of the Ice Object to which the request was sent. */
    public final Identity id;

    /** The facet to which the request was sent. */
    public final String facet;

    /** The operation name of the request. */
    public final String operation;

    private static final long serialVersionUID = 4181164754424262091L;
}
