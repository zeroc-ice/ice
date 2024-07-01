% MarshalException   Summary of MarshalException
%
% This exception is raised for errors during marshaling or unmarshaling data.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef MarshalException < Ice.ProtocolException
    methods
        function obj = MarshalException(errID, msg, reason)
            if nargin <= 2
                reason = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:MarshalException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.MarshalException';
            end
            obj = obj@Ice.ProtocolException(errID, msg, reason);
        end
    end
end
