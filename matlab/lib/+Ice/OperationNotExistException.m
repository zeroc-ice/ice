% OperationNotExistException   Summary of OperationNotExistException
%
% This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
% by either the client or the server using an outdated Slice specification.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef OperationNotExistException < Ice.RequestFailedException
    methods
        function obj = OperationNotExistException(errID, msg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:OperationNotExistException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.OperationNotExistException';
            end
            obj = obj@Ice.RequestFailedException(errID, msg, id, facet, operation);
        end
    end
end
