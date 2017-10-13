classdef (Sealed) InitializationData
    % InitializationData   Summary of InitializationData
    %
    % A class that encapsulates data to initialize a communicator.
    %
    % InitializationData Properties:
    %   properties_ - The properties for the communicator.
    %   compactIdResolver - Returns the type ID corresponding to a compact ID.
    %   valueFactoryManager - The value factory manager.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    methods
        function r = clone(obj)
            r = Ice.InitializationData();
            r.properties_ = obj.properties_;
            r.compactIdResolver = obj.compactIdResolver;
            r.valueFactoryManager = obj.valueFactoryManager;
        end
    end
    properties
        % properties_ - The properties for the communicator.
        properties_

        % compactIdResolver - Returns the type ID corresponding to a compact ID.
        %   The value must be handle to a function that accepts an int32 value
        %   representing a compact ID and return a char array holding the type ID.
        compactIdResolver

        % valueFactoryManager - The value factory manager.
        valueFactoryManager
    end
end
