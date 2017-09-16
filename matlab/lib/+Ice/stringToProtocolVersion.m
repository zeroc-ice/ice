%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function r = stringToProtocolVersion(s)
    tokens = regexp(s, '^([0-9]+)\.([0-9]+)$', 'tokens');
    if isempty(tokens)
        throw(MException('Ice:ArgumentException', 'expecting a version in X.Y format'));
    end
    r = Ice.ProtocolVersion(str2num(tokens{1}{1}), str2num(tokens{1}{2}));
end
