function [properties, remArgs] = createProperties(args, defaults)
    % createProperties  Creates a property set, optionally initialized from an argument vector and default properties.
    %
    % Examples:
    %   props = Ice.createProperties();
    %   [props, remArgs] = Ice.createProperties(args);
    %   [props, remArgs] = Ice.createProperties(args, defaults);
    %
    % Parameters:
    %   args (cell array of char or string array) - A command-line argument vector, possibly containing options to set
    %     properties. If the command-line options include a --Ice.Config option, the corresponding configuration files
    %     are parsed. If the same property is set in a configuration file and in the argument vector, the argument
    %     vector takes precedence.
    %   defaults (Ice.Properties) - A property set used to initialize the default state of the new property set.
    %     Settings in configuration files and the argument vector override these defaults.
    %
    % Returns:
    %   properties (Ice.Properties) - A new property set initialized with the property settings that were removed from
    %      the argument vector and the default property set.
    %   remArgs (string array) - Contains the remaining command-line arguments that were not used to set properties.

    % Copyright (c) ZeroC, Inc.

    arguments
        args (1, :) = {}
        defaults Ice.Properties {mustBeScalarOrEmpty} = Ice.Properties.empty
    end
    if isempty(defaults)
        defaultsImpl = libpointer('voidPtr');
    else
        defaultsImpl = defaults.impl_;
    end
    impl = libpointer('voidPtr');
    remArgs = IceInternal.Util.callWithResult('Ice_createProperties', args, defaultsImpl, impl);
    properties = Ice.Properties(impl);
end
