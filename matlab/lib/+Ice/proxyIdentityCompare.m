function r = proxyIdentityCompare(lhs, rhs)
    %PROXYIDENTITYCOMPARE Compares the object identities of two proxies.
    %
    %   Input Arguments
    %     lhs - A proxy.
    %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array
    %     rhs - Another proxy.
    %       Ice.ObjectPrx scalar | Ice.ObjectPrx empty array
    %
    %   Output Arguments
    %     r - -1 if the identity in lhs compares less than the identity in rhs; 0 if the identities compare equal;
    %       1, otherwise.
    %       int32 scalar

    % Copyright (c) ZeroC, Inc.

    arguments
        lhs Ice.ObjectPrx {mustBeScalarOrEmpty}
        rhs Ice.ObjectPrx {mustBeScalarOrEmpty}
    end

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
