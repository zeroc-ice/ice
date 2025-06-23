% Copyright (c) ZeroC, Inc.

classdef ConnectionAbortedException < Ice.LocalException
    %CONNECTIONABORTEDEXCEPTION This exception indicates that the connection was closed forcefully.
    %
    %   ConnectionAbortedException Properties:
    %     closedByApplication - True if the connection was closed by the application, false if it was closed by the Ice runtime.

    properties
        %CLOSEDBYAPPLICATION True if the connection was closed by the application, false if it was closed by the Ice
        %   runtime.
        %   logical scalar
        closedByApplication (1, 1) logical = false
    end
    methods(Hidden)
        function obj = ConnectionAbortedException(closedByApplication, errID, what)
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
