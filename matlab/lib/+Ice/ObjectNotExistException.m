% ObjectNotExistException   Summary of ObjectNotExistException
%
% This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
% exist.

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from LocalException.ice by slice2matlab version 3.7.10

classdef ObjectNotExistException < Ice.RequestFailedException
    methods
        function obj = ObjectNotExistException(errID, msg, id, facet, operation)
            if nargin <= 2
                id = Ice.Identity();
                facet = '';
                operation = '';
            end
            if nargin == 0 || isempty(errID)
                errID = 'Ice:ObjectNotExistException';
            end
            if nargin < 2 || isempty(msg)
                msg = 'Ice.ObjectNotExistException';
            end
            obj = obj@Ice.RequestFailedException(errID, msg, id, facet, operation);
        end
    end
end
