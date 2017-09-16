%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

%
% This function returns a singleton instance that we use as a sentinel value to indicate an unset optional value.
%
function r = Unset()
    r = IceInternal.UnsetI.getInstance();
end
