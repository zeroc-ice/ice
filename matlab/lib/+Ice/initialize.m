function communicator = initialize(varargin)
    %INITIALIZE Creates a communicator.
    %   This function is provided for backwards compatibility. New code should call the Ice.Communicator
    %   constructor directly.
    %
    %   Examples
    %     communicator = Ice.initialize()
    %     communicator = Ice.initialize(args)
    %     communicator = Ice.initialize(initData)
    %
    %   Input Arguments
    %     args - An optional argument vector. Any Ice-related options in this vector are used to initialize the
    %       communicator. Not compatible with initData.
    %       string array | cell array of character vectors
    %     initData - An optional initialization data. Not compatible with args.
    %       Ice.InitializationData scalar | empty array of Ice.InitializationData
    %
    %   Output Arguments
    %     communicator - The new communicator.
    %       Ice.Communicator scalar

    % Copyright (c) ZeroC, Inc.

    if nargin > 1
        error('Ice:ArgumentException', 'Too many arguments to Ice.initialize');
    end

    if nargin == 0
        communicator = Ice.Communicator();
    elseif isa(varargin{1}, 'cell') || isa(varargin{1}, 'string')
        communicator = Ice.Communicator(varargin{1});
    elseif isa(varargin{1}, 'Ice.InitializationData')
        initData = varargin{1};
        if isempty(initData)
            communicator = Ice.Communicator();
        else
            communicator = Ice.Communicator(Properties = initData.Properties, SliceLoader = initData.SliceLoader);
        end
    else
        error('Ice:ArgumentException', 'Unexpected argument to Ice.initialize');
    end
end
