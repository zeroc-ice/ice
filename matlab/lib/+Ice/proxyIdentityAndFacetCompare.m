function r = proxyIdentityAndFacetCompare(lhs, rhs)
    % proxyIdentityAndFacetCompare  Compares the object identities and facets
    %   of two proxies.
    %
    % Parameters:
    %   lhs (Ice.ObjectPrx) - A proxy.
    %   rhs (Ice.ObjectPrx) - A proxy.
    %
    % Returns (int32) - -1 if the identity and facet in lhs compares less than
    %   the identity and facet in rhs; 0 if the identities and facets compare
    %   equal; 1, otherwise.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    if isempty(lhs) && isempty(rhs)
        r = 0;
    elseif isempty(lhs) && ~isempty(rhs)
        r = -1;
    elseif ~isempty(lhs) && isempty(rhs)
        r = 1;
    else
        lhsIdentity = lhs.ice_getIdentity();
        rhsIdentity = rhs.ice_getIdentity();
        n = IceInternal.Util.strcmp(lhsIdentity.name, rhsIdentity.name);
        if n ~= 0
            r = n;
            return;
        end
        n = IceInternal.Util.strcmp(lhsIdentity.category, rhsIdentity.category);
        if n ~= 0
            r = n;
            return;
        end

        lhsFacet = lhs.ice_getFacet();
        rhsFacet = rhs.ice_getFacet();
        if isempty(lhsFacet) && isempty(rhsFacet)
            r = 0;
        elseif isempty(lhsFacet)
            r = -1;
        elseif isempty(rhsFacet)
            r = 1;
        else
            r = IceInternal.Util.strcmp(lhsFacet, rhsFacet);
        end
    end
end
