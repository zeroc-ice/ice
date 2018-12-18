classdef (Sealed) InitializationData
    % InitializationData   Summary of InitializationData
    %
    % A class that encapsulates data to initialize a communicator.
    %
    % InitializationData Properties:
    %   properties_ - The properties for the communicator.

    % Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

    methods
        function r = clone(obj)
            r = Ice.InitializationData();
            r.properties_ = obj.properties_;
        end
    end
    properties
        % properties_ - The properties for the communicator.
        properties_
    end
end
