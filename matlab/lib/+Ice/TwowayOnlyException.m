% TwowayOnlyException   Summary of TwowayOnlyException
%
% The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
% an operation with ice_oneway, ice_batchOneway, ice_datagram, or
% ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
%
% TwowayOnlyException Properties:
%   operation - The name of the operation that was invoked.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef TwowayOnlyException < Ice.LocalException
    properties
        % operation - The name of the operation that was invoked.
        operation char
    end
    methods
        function obj = TwowayOnlyException(errID, msg, operation)
            if nargin <= 2
                operation = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:TwowayOnlyException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.TwowayOnlyException';
            end
            obj = obj@Ice.LocalException(errID, msg);
            obj.operation = operation;
        end
    end
end
