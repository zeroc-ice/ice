%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function r = identityToString(id, varargin)
    if length(varargin) == 1
        mode = varargin{1};
    elseif length(varargin) == 0
        mode = Ice.ToStringMode.Unicode;
    elseif length(varargin) > 2
        throw(MException('Ice:ArgumentException', 'too many arguments'));
    end
    r = IceInternal.Util.callWithResult('Ice_identityToString', id, int32(mode));
end
