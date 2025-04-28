classdef (Sealed) InitializationData
    % InitializationData   Summary of InitializationData
    %
    % A class that encapsulates data to initialize a communicator.
    %
    % InitializationData Properties:
    %   properties_ - The properties for the communicator.
    %   sliceLoader - The Slice loader, used to unmarshal Slice classes and exceptions.

    % Copyright (c) ZeroC, Inc.

    methods
        function r = clone(obj)
            r = Ice.InitializationData();
            r.properties_ = obj.properties_;
            r.sliceLoader = obj.sliceLoader;
        end
    end
    properties
        % properties_ - The properties for the communicator.
        properties_ Ice.Properties

        % sliceLoader - The Slice loader, used to unmarshal Slice classes and exceptions.
        sliceLoader Ice.SliceLoader = IceInternal.DefaultSliceLoader.Instance
    end
end
