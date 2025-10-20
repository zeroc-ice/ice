function [properties, remArgs] = createProperties(args, defaults)
    %CREATEPROPERTIES Creates an Ice.Properties object.
    %   This function is provided for backwards compatibility. New code should call the Ice.Properties constructor
    %   directly.
    %
    %   Input Arguments
    %     args - An optional argument vector.
    %       empty string array (default) | string array | cell array of character vectors
    %     defaults - Optional default properties.
    %       Ice.Properties scalar | empty array of Ice.Properties
    %
    %   Output Arguments
    %     properties - The new properties object.
    %       Ice.Properties scalar
    %     remArgs - The remaining command-line arguments.
    %       string array
    %
    %   See also Ice.Properties/Properties.

    % Copyright (c) ZeroC, Inc.

    arguments
        args (1, :) string = string.empty
        defaults Ice.Properties {mustBeScalarOrEmpty} = Ice.Properties.empty
    end
    [properties, remArgs] = Ice.Properties(args, defaults);
end
