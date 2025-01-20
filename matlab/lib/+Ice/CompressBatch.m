% CompressBatch   Summary of CompressBatch
%
% The batch compression option when flushing queued batch requests.
%
% CompressBatch Properties:
%   Yes - Compress the batch requests.
%   No - Don't compress the batch requests.
%   BasedOnProxy - Compress the batch requests if at least one request was made on a compressed proxy.

%  Copyright (c) ZeroC, Inc.

classdef CompressBatch < uint8
    enumeration
        % Compress the batch requests.
        Yes (0)
        % Don't compress the batch requests.
        No (1)
        % Compress the batch requests if at least one request was made on a compressed proxy.
        BasedOnProxy (2)
    end
end
