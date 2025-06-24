classdef CompressBatch < uint8
    %COMPRESSBATCH The batch compression option when flushing queued batch requests.
    %
    %   CompressBatch Properties:
    %     Yes - Compress the batch requests.
    %     No - Don't compress the batch requests.
    %     BasedOnProxy - Compress the batch requests if at least one request was made on a compressed proxy.

    % Copyright (c) ZeroC, Inc.

    enumeration
        %YES Compress the batch requests.
        Yes (0)

        %NO Don't compress the batch requests.
        No (1)

        %BASEDONPROXY Compress the batch requests if at least one request was made on a compressed proxy.
        BasedOnProxy (2)
    end
end
