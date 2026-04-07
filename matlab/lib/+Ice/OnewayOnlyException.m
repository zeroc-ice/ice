classdef (Sealed) OnewayOnlyException < Ice.LocalException
    %ONEWAYONLYEXCEPTION The exception that is thrown when attempting to invoke a oneway-only operation
    %   (an operation with the ["oneway"] metadata directive) on a twoway proxy.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden)
        function obj = OnewayOnlyException(operation)
            if nargin == 0
                superArgs = {};
            else
                assert(nargin == 1, 'Invalid number of arguments');
                superArgs = {'Ice:OnewayOnlyException',...
                    sprintf('cannot invoke oneway operation ''%s'' with a twoway proxy', operation)};
            end
            obj@Ice.LocalException(superArgs{:});
        end
    end
end
