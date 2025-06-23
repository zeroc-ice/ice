function [properties, remArgs] = createProperties(args, defaults)
    %CREATEPROPERTIES Deprecated function to create an Ice.Properties object. Use the Ice.Properties constructor
    %   instead.
    %
    %   Input Arguments
    %     args - An optional argument vector.
    %       cell array of char | string array
    %     defaults - Optional default properties.
    %       Ice.Properties scalar | Ice.Properties empty array
    %
    %   Output Arguments
    %     properties - The new properties object.
    %       Ice.Properties scalar
    %     remArgs - The remaining command-line arguments.
    %       string array
    %
    %   See also Ice.Properties/Properties.
    arguments
        args (1, :) = {}
        defaults Ice.Properties {mustBeScalarOrEmpty} = Ice.Properties.empty
    end
    [properties, remArgs] = Ice.Properties(args, defaults);
end
