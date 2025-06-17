function [communicator, args] = initialize(varargin)
    % initialize  Creates a communicator.
    %
    % Examples:
    %   communicator = Ice.initialize();
    %   [communicator, remArgs] = Ice.initialize(args);
    %   communicator = Ice.initialize(initData);
    %   [communicator, remArgs] = Ice.initialize(args, initData);
    %   communicator = Ice.initialize(configFile);
    %   [communicator, remArgs] = Ice.initialize(args, configFile);
    %
    % Parameters:
    %   args (cell array of char or string array) - An optional argument vector. Any Ice-related options in this vector
    %     are used to initialize the communicator.
    %   initData (Ice.InitializationData) - Optional additional initialization data.
    %   configFile (char) - Optional configuration file name.
    %
    % Returns:
    %   communicator (Ice.Communicator) - The new communicator.
    %   args (string array) - Contains the remaining command-line arguments that were not used to set properties.

    % Copyright (c) ZeroC, Inc.

    if length(varargin) > 2
        throw(Ice.LocalException('Ice:ArgumentException', 'too many arguments to Ice.initialize'));
    end

    args = [];
    initData = [];
    configFile = '';

    for i = 1:length(varargin)
        if isa(varargin{i}, 'cell') && isempty(args)
            args = string(varargin{i}); % convert cell array to string array
        elseif isa(varargin{i}, 'string') && isempty(args)
            args = varargin{i};
        elseif isa(varargin{i}, 'Ice.InitializationData') && isempty(initData)
            initData = varargin{i};
        elseif isa(varargin{i}, 'char') && isempty(configFile)
            configFile = varargin{i};
        else
            throw(Ice.LocalException('Ice:ArgumentException', 'unexpected argument to Ice.initialize'));
        end
    end

    if ~isempty(initData) && ~isempty(configFile)
        throw(Ice.LocalException('Ice:ArgumentException', ...
                         'initialize accepts either Ice.InitializationData or a configuration filename'));
    end

    if isempty(initData)
        initData = Ice.InitializationData();

        if ~isempty(configFile)
            initData.Properties.load(configFile);
        end
    end

    %
    % Implementation notes:
    %
    % We need to extract and pass the libpointer object for properties to the C function. Passing the wrapper
    % (Ice.Properties) object won't work because the C code has no way to obtain the inner pointer.
    %
    props = initData.Properties.impl_;

    impl = libpointer('voidPtr');
    args = IceInternal.Util.callWithResult('Ice_initialize', args, props, impl);
    communicator = Ice.Communicator(impl, initData);
end
