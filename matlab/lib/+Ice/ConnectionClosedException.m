% Copyright (c) ZeroC, Inc.

classdef ConnectionClosedException < Ice.LocalException
    %CONNECTIONCLOSEDEXCEPTION This exception indicates that the connection was closed gracefully.
    %
    %   ConnectionClosedException Properties:
    %     closedByApplication - True if the connection was closed by the application, false if it was closed by the Ice runtime.

    properties
        %CLOSEDBYAPPLICATION True if the connection was closed by the application, false if it was closed by the Ice
        %   runtime.
        %   logical scalar
        closedByApplication (1, 1) logical = false
    end
    methods(Hidden)
        function obj = ConnectionClosedException(closedByApplication, errID, what)
            if nargin == 0 % default constructor
                superArgs = {};
            else
                assert(nargin == 3, 'Invalid number of arguments');
                superArgs = {errID, what};
            end
            obj@Ice.LocalException(superArgs{:});
            if nargin > 0
                obj.closedByApplication = closedByApplication;
            end
        end
    end
end
