%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef OnewaysAMI
    methods(Static)
        function onewaysAMI(p)
            import Test.*;

            p = p.ice_oneway();

            call(p, 'ice_ping');

            try
                p.ice_isAAsync(MyClassPrx.ice_staticId());
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end

            try
                p.ice_idAsync();
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end

            try
                p.ice_idsAsync();
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end

            call(p, 'opVoid');

            call(p, 'opIdempotent');

            call(p, 'opNonmutating');

            try
                p.opByteAsync(hex2dec('ff'), hex2dec('0f'));
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end
        end
    end
end

function varargout = call(p, op, varargin)
    try
        name = [op, 'Async'];
        future = p.(name)(varargin{:});
        assert(strcmp(future.Operation, op));
        assert(future.wait());
        assert(strcmp(future.State, 'finished'));
    catch ex
        disp(getReport(ex, 'extended'));
        assert(false);
    end
end
