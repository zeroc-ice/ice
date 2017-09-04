%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function r = proxyIdentityCompare(lhs, rhs)
    if isempty(lhs) && isempty(rhs)
        r = 0;
    elseif isempty(lhs) && ~isempty(rhs)
        r = -1;
    elseif ~isempty(lhs) && isempty(rhs)
        r = 1;
    else
        lhsIdentity = lhs.ice_getIdentity();
        rhsIdentity = rhs.ice_getIdentity();
        n = Ice.Util.strcmp(lhsIdentity.name, rhsIdentity.name);
        if n ~= 0
            r = n;
            return;
        end
        r = Ice.Util.strcmp(lhsIdentity.category, rhsIdentity.category);
    end
end
