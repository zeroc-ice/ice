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

classdef RequestFailedException < Ice.DispatchException
    properties
        % id - The identity of the Ice Object to which the request was sent.
        id Ice.Identity = Ice.Identity.empty
        % facet - The facet to which the request was sent.
        facet char = ''
        % operation - The operation name of the request.
        operation char = ''
    end
    methods
        function obj = RequestFailedException(replyStatus, id, facet, operation, errID, what)
            if nargin == 0
                superArgs = {};
            else
                assert(nargin == 6, 'Invalid number of arguments');
                superArgs = {replyStatus, errID, what};
            end
            obj@Ice.DispatchException(superArgs{:});
            if nargin > 0
                obj.id = id;
                obj.facet = facet;
                obj.operation = operation;
            end
        end
    end
end
