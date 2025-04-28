classdef (Sealed) InitializationData
    % InitializationData   Represents a set of options that you can specify when initializing a communicator.
    %
    % InitializationData Properties:
    %   properties_ - The properties of the communicator.
    %   sliceLoader - The Slice loader, used to unmarshal Slice classes and exceptions.
    properties
        % properties_ (Ice.Properties) - The properties for the communicator.
        properties_ Ice.Properties

        % sliceLoader (Ice.SliceLoader) - The Slice loader, used to unmarshal Slice classes and exceptions.
        sliceLoader Ice.SliceLoader = IceInternal.DefaultSliceLoader.Instance
    end
end
