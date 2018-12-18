%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Oneways
    methods(Static)
        function oneways(p)
            p = p.ice_oneway();

            p.ice_ping();

            p.opVoid();

            p.opIdempotent();

            p.opNonmutating();

            try
                p.opByte(hex2dec('ff'), hex2dec('0f'));
                assert(false);
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end
        end
    end
end
