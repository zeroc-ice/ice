// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an object does not exist on the server, that is, if the dispatch
 * could not find a servant for the identity carried by the request.
 */
public final class ObjectNotExistException extends RequestFailedException {
    /**
     * Constructs an ObjectNotExistException.
     */
    public ObjectNotExistException() {
        super(ReplyStatus.ObjectNotExist);
    }

    /**
     * Constructs an ObjectNotExistException with object details.
     *
     * @param id        the identity of the Ice Object to which the request was sent
     * @param facet     the facet to which the request was sent
     * @param operation the operation name of the request
     */
    public ObjectNotExistException(Identity id, String facet, String operation) {
        super(ReplyStatus.ObjectNotExist, id, facet, operation);
    }

    @Override
    public String ice_id() {
        return "::Ice::ObjectNotExistException";
    }

    private static final long serialVersionUID = 5680607485723114764L;
}
