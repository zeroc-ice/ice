// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * This exception is raised if no facet with the given name exists, that is, if the dispatch could
 * not find a servant for the identity + facet carried by the request.
 */
public final class FacetNotExistException extends RequestFailedException {
    /**
     * Creates a new {@code FacetNotExistException} with the default message.
     */
    public FacetNotExistException() {
        super(ReplyStatus.FacetNotExist);
    }

    /**
     * Creates a new {@code FacetNotExistException} with the specified identity, facet, and operation.
     *
     * @param id        the identity of the Ice Object to which the request was sent
     * @param facet     the facet to which the request was sent
     * @param operation the operation name of the request
     */
    public FacetNotExistException(Identity id, String facet, String operation) {
        super(ReplyStatus.FacetNotExist, id, facet, operation);
    }

    @Override
    public String ice_id() {
        return "::Ice::FacetNotExistException";
    }

    private static final long serialVersionUID = -584705670010603188L;
}
