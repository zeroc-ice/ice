classdef (Sealed) InitializationData
    % InitializationData   Represents a set of options that you can specify when initializing a communicator.
    %
    % InitializationData Properties:
    %   Properties - The properties of the communicator.
    %   SliceLoader - The Slice loader, used to unmarshal Slice classes and exceptions.

    properties
        Properties (1, 1) Ice.Properties = Ice.createProperties() % The properties of the communicator.

        SliceLoader (1, 1) Ice.SliceLoader = IceInternal.DefaultSliceLoader.Instance % The Slice loader.
    end
    properties (Dependent, Hidden)
        properties_ (1, 1) Ice.Properties % Deprecated: Use Properties instead.
    end
    methods
        function obj = InitializationData(options)
            arguments
                options.?Ice.InitializationData
            end
            for prop = string(fieldnames(options))'
                obj.(prop) = options.(prop);
            end
        end

        function value = get.properties_(obj)
            value = obj.Properties;
        end
        function obj = set.properties_(obj, value)
            obj.Properties = value;
        end
    end
end
