%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function r = intVersion()
    v = libpointer('int32Ptr', 0);
    IceInternal.Util.call('Ice_intVersion', v);
    r = v.Value;
end
