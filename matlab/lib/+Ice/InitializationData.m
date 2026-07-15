classdef (Sealed) InitializationData
    %INITIALIZATIONDATA Represents a set of options that you can specify when initializing a communicator.
    %
    %   InitializationData Properties:
    %     Properties - The properties of the communicator.
    %     SliceLoader - The Slice loader, used to unmarshal Slice classes and exceptions.
    %
    %   See also Ice.initialize, Ice.Communicator.

    % Copyright (c) ZeroC, Inc.

    properties
        %PROPERTIES The properties of the communicator.
        %   Ice.Properties scalar
        Properties (1, 1) Ice.Properties = Ice.Properties()

        %SLICELOADER The Slice loader, used to unmarshal Slice classes and exceptions.
        %   Ice.SliceLoader scalar
        SliceLoader (1, 1) Ice.SliceLoader = IceInternal.DefaultSliceLoader.Instance
    end
    properties (Dependent, Hidden)
        properties_ (1, 1) Ice.Properties % Deprecated: Use Properties instead.
    end
    methods
        function obj = InitializationData(options)
            %INITIALIZATIONDATA Constructs an InitializationData from name-value arguments.
            %
            %   Examples
            %     initData = Ice.InitializationData(Properties = props);
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
