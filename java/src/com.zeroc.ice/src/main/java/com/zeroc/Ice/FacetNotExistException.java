// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if no facet with the given name exists, that is, if the dispatch could
 * not find a servant for the identity + facet carried by the request.
 */
public final class FacetNotExistException extends RequestFailedException {
    public FacetNotExistException() {
        super(ReplyStatus.FacetNotExist);
    }

    public FacetNotExistException(Identity id, String facet, String operation) {
        super(ReplyStatus.FacetNotExist, id, facet, operation);
    }

    @Override
    public String ice_id() {
        return "::Ice::FacetNotExistException";
    }

    private static final long serialVersionUID = -584705670010603188L;
}
