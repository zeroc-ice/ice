% RequestFailedException   Summary of RequestFailedException
%
% This exception is raised if a request failed. This exception, and all exceptions derived from
% RequestFailedException, are transmitted by the Ice protocol, even though they are declared
% local.
%
% RequestFailedException Properties:
%   id - The identity of the Ice Object to which the request was sent.
%   facet - The facet to which the request was sent.
%   operation - The operation name of the request.

% Copyright (c) ZeroC, Inc.

classdef RequestFailedException < Ice.LocalException
    properties
        % id - The identity of the Ice Object to which the request was sent.
        id Ice.Identity
        % facet - The facet to which the request was sent.
        facet char
        % operation - The operation name of the request.
        operation char
    end
    methods
        function obj = RequestFailedException(id, facet, operation, errID, what)
            if nargin == 0 % default constructor
                id = Ice.Identity();
                facet = '';
                operation = '';
                superArgs = {};
            else
                assert(nargin == 5, 'Invalid number of arguments');
                superArgs = {errID, what};
            end

            obj@Ice.LocalException(superArgs{:});
            obj.id = id;
            obj.facet = facet;
            obj.operation = operation;
        end
    end
end
