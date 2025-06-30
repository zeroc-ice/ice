classdef (Sealed) TwowayOnlyException < Ice.LocalException
    %TWOWAYONLYEXCEPTION The exception that is thrown when attempting to invoke an operation with ice_oneway,
    %   ice_batchOneway, ice_datagram, or ice_batchDatagram, and the operation has a return value, an out parameter,
    %   or an exception specification.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = TwowayOnlyException(operation)
            if nargin == 0
                superArgs = {};
            else
                assert(nargin == 1, 'Invalid number of arguments');
                superArgs = {'Ice:TwowayOnlyException', sprintf('operation ''%s'' can only be invoked with a twoway proxy',...
                    operation)};
            end
            obj@Ice.LocalException(superArgs{:});
        end
    end
end
