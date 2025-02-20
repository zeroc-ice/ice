// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if an object does not exist on the server, that is, if the dispatch
 * could not find a servant for the identity carried by the request.
 */
public final class ObjectNotExistException extends RequestFailedException {
    public ObjectNotExistException() {
        super(ReplyStatus.ObjectNotExist);
    }

    public ObjectNotExistException(Identity id, String facet, String operation) {
        super(ReplyStatus.ObjectNotExist, id, facet, operation);
    }

    @Override
    public String ice_id() {
        return "::Ice::ObjectNotExistException";
    }

    private static final long serialVersionUID = 5680607485723114764L;
}
