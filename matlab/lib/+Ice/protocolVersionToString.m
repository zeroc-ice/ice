%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function r = protocolVersionToString(v)
    if ~isa(v, 'Ice.ProtocolVersion')
        throw(MException('Ice:ArgumentException', 'expecting an Ice.ProtocolVersion'));
    end
    r = sprintf('%d.%d', v.major, v.minor);
end
