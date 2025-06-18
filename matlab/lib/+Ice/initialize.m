function [communicator, remArgs] = initialize(varargin)
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
    %   remArgs (string array) - Contains the remaining command-line arguments that were not used to set properties.

    % Copyright (c) ZeroC, Inc.

    if length(varargin) > 2
        error('Ice:ArgumentException', 'Too many arguments to Ice.initialize');
    end

    args = [];
    initData = [];
    configFile = '';

    for i = 1:length(varargin)
        if isa(varargin{i}, 'cell') && isempty(args)
            args = varargin{i}; % keep cell array
        elseif isa(varargin{i}, 'string') && isempty(args)
            args = varargin{i};
        elseif isa(varargin{i}, 'Ice.InitializationData') && isempty(initData)
            initData = varargin{i};
        elseif isa(varargin{i}, 'char') && isempty(configFile)
            configFile = varargin{i};
        else
            error('Ice:ArgumentException', 'Unexpected argument to Ice.initialize');
        end
    end

    if ~isempty(initData) && ~isempty(configFile)
        error('Ice:ArgumentException', 'initialize accepts either Ice.InitializationData or a configuration filename');
    end

    if isempty(initData)
        initData = Ice.InitializationData();

        if ~isempty(configFile)
            initData.Properties.load(configFile);
        end
    end

    [communicator, remArgs] = Ice.Communicator(args, Properties = initData.Properties, ...
        SliceLoader = initData.SliceLoader);
end
