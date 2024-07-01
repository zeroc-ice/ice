% NoEndpointException   Summary of NoEndpointException
%
% This exception is raised if no suitable endpoint is available.
%
% NoEndpointException Properties:
%   proxy - The stringified proxy for which no suitable endpoint is available.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef NoEndpointException < Ice.LocalException
    properties
        % proxy - The stringified proxy for which no suitable endpoint is available.
        proxy char
    end
    methods
        function obj = NoEndpointException(errID, msg, proxy)
            if nargin <= 2
                proxy = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:NoEndpointException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.NoEndpointException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.proxy = proxy;
        end
    end
end
