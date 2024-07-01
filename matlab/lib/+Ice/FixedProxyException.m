% FixedProxyException   Summary of FixedProxyException
%
% This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef FixedProxyException < Ice.LocalException
    methods
        function obj = FixedProxyException(errID, msg)
            if nargin == 0 || isempty(errID)
                errID = 'Ice:FixedProxyException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.FixedProxyException';
            end
            obj = obj@Ice.LocalException(errID, msg);
        end
    end
end
