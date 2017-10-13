function [properties, remArgs] = createProperties(varargin)
    % createProperties  Creates a property set, optionally initialized from an
    %   argument vector and default properties.
    %
    % Examples:
    %   props = Ice.createProperties();
    %   [props, remArgs] = Ice.createProperties(args);
    %   props = Ice.createProperties(defaults);
    %   [props, remArgs] = Ice.createProperties(args, defaults);
    %
    % Parameters:
    %   args (cell array of char) - A command-line argument vector, possibly
    %     containing options to set properties. If the command-line options include
    %     a --Ice.Config option, the corresponding configuration files are parsed.
    %     If the same property is set in a configuration file and in the argument
    %     vector, the argument vector takes precedence.
    %   defaults (Ice.Properties) - A property set used to initialize the default
    %     state of the new property set. Settings in configuration files and the
    %     argument vector override these defaults.
    %
    % Returns:
    %   properties (Ice.Properties) - A new property set initialized with the property
    %     settings that were removed from the argument vector and the default property set.
    %   remArgs (cell array of char) - Contains the remaining command-line arguments
    %     that were not used to set properties.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    if length(varargin) >= 1 && ~isempty(varargin{1})
        args = varargin{1};
    else
        args = {};
    end
    if length(varargin) >= 2 && ~isempty(varargin{2})
        if ~isa(varargin{2}, 'Ice.Properties')
            throw(MException('Ice:ArgumentException', 'expecting Ice.Properties object'));
        end
        defaults = varargin{2}.impl_;
    else
        defaults = libpointer('voidPtr');
    end
    impl = libpointer('voidPtr');
    remArgs = IceInternal.Util.callWithResult('Ice_createProperties', args, defaults, impl);
    properties = Ice.Properties(impl);
end
