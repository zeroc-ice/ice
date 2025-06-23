% Copyright (c) ZeroC, Inc.

classdef (Sealed) TwowayOnlyException < Ice.LocalException
    %TWOWAYONLYEXCEPTION The operation can only be invoked with a twoway request. This exception is raised if an
    %   attempt is made to invoke an operation with ice_oneway, ice_batchOneway, ice_datagram, or ice_batchDatagram
    %   and the operation has a return value, out-parameters, or an exception specification.

    methods(Hidden)
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
