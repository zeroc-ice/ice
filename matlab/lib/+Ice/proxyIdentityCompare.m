function r = proxyIdentityCompare(lhs, rhs)
    % proxyIdentityCompare  Compares the object identities of two proxies.
    %
    % Parameters:
    %   lhs (Ice.ObjectPrx) - A proxy.
    %   rhs (Ice.ObjectPrx) - A proxy.
    %
    % Returns (int32) - -1 if the identity in lhs compares less than the
    %   identity in rhs; 0 if the identities compare equal; 1, otherwise.

    % Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

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
        r = IceInternal.Util.strcmp(lhsIdentity.category, rhsIdentity.category);
    end
end
