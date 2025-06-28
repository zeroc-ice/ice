classdef RequestFailedException < Ice.DispatchException
    %REQUESTFAILEDEXCEPTION The base class for the 3 NotExist exceptions.
    %
    %   RequestFailedException Properties:
    %     id - The identity of the Ice Object to which the request was sent.
    %     facet - The facet to which the request was sent.
    %     operation - The operation name of the request.

    % Copyright (c) ZeroC, Inc.

    properties
        %ID The identity of the Ice Object to which the request was sent.
        %   Ice.Identity scalar
        id (1, :) Ice.Identity = Ice.Identity.empty

        %FACET The facet to which the request was sent.
        %   character vector
        facet (1, :) char = ''

        %OPERATION The operation name of the request.
        %   character vector
        operation (1, :) char = ''
    end
    methods (Hidden)
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
