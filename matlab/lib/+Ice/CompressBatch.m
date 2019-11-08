% CompressBatch   Summary of CompressBatch
%
% The batch compression option when flushing queued batch requests.
%
% CompressBatch Properties:
%   Yes - Compress the batch requests.
%   No - Don't compress the batch requests.
%   BasedOnProxy - Compress the batch requests if at least one request was made on a compressed proxy.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Connection.ice by slice2matlab version 3.7.3

classdef CompressBatch < uint8
    enumeration
        % Compress the batch requests.
        Yes (0)
        % Don't compress the batch requests.
        No (1)
        % Compress the batch requests if at least one request was
        % made on a compressed proxy.
        BasedOnProxy (2)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.CompressBatch.Yes;
                case 1
                    r = Ice.CompressBatch.No;
                case 2
                    r = Ice.CompressBatch.BasedOnProxy;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
