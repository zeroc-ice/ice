% ProtocolException   Summary of ProtocolException
%
% A generic exception base for all kinds of protocol error conditions.
%
% ProtocolException Properties:
%   reason - The reason for the failure.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ProtocolException < Ice.LocalException
    properties
        % reason - The reason for the failure.
        reason char
    end
    methods
        function obj = ProtocolException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ProtocolException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ProtocolException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.reason = reason;
        end
    end
end
