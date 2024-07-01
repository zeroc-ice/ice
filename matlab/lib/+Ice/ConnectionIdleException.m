% ConnectionIdleException   Summary of ConnectionIdleException
%
% This exception indicates that a connection was aborted by the idle check.

% Copyright (c) ZeroC, Inc. All rights reserved.

classdef ConnectionIdleException < Ice.LocalException
    methods
        function obj = ConnectionIdleException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ConnectionIdleException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ConnectionIdleException';
            end
            obj = obj@Ice.LocalException(errID, msg);
        end
    end
end
