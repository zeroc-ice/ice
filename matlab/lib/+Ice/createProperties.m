function [properties, remArgs] = createProperties(args, defaults)
    % createProperties - Deprecated function to create an Ice.Properties object. Use the Ice.Properties constructor
    % instead.
    arguments
        args (1, :) = {}
        defaults Ice.Properties {mustBeScalarOrEmpty} = Ice.Properties.empty
    end
    [properties, remArgs] = Ice.Properties(args, defaults);
end
