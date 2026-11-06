% Copyright (c) ZeroC, Inc.

function client(args)
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    fprintf("testing Ice.initialize... ");
    args1 = ["--Foo.Bar=1", "--Foo.Baz=2" "--Ice.Trace.Network=3"];
    communicator = Ice.initialize(args1);
    assert(communicator.getProperties().getIceProperty('Ice.Trace.Network') == '3');
    communicator.destroy();
    fprintf('ok\n');

    fprintf("testing Ice.Communicator constructor... ");
    args1 = ["--Foo.Bar=1", "--Foo.Baz=2", "--Ice.Trace.Network=3"];
    args2 = ["--Foo.Baz=1", "--Ice.Trace.Network=2"];
    communicator = Ice.Communicator(args2, Properties = Ice.Properties(args1));
    assert(communicator.getProperties().getIceProperty('Ice.Trace.Network') == '2');
    % display(communicator.getProperties());
    communicator.destroy();
    fprintf('ok\n');

    fprintf("testing ICE_CONFIG... ");
    configDir = fullfile(fileparts(mfilename('fullpath')), 'config');
    previousIceConfig = getenv('ICE_CONFIG');
    restoreIceConfig = onCleanup(@() setenv('ICE_CONFIG', previousIceConfig));
    setenv('ICE_CONFIG', fullfile(configDir, 'iceConfigEnv.cfg'));

    % Without an argument vector, Ice does not load the file named by ICE_CONFIG.
    props = Ice.Properties();
    assert(isempty(props.getProperty('Ice.Trace.Network')));
    props = Ice.createProperties();
    assert(isempty(props.getProperty('Ice.Trace.Network')));
    communicator = Ice.Communicator();
    assert(isempty(communicator.getProperties().getProperty('Ice.Trace.Network')));
    communicator.destroy();
    communicator = Ice.Communicator(Properties = Ice.Properties());
    assert(isempty(communicator.getProperties().getProperty('Ice.Trace.Network')));
    communicator.destroy();
    communicator = Ice.initialize();
    assert(isempty(communicator.getProperties().getProperty('Ice.Trace.Network')));
    communicator.destroy();

    % With an argument vector that has no --Ice.Config option, Ice loads the file named by ICE_CONFIG.
    props = Ice.Properties(string.empty);
    assert(strcmp(props.getProperty('Ice.Trace.Network'), '1'));
    communicator = Ice.Communicator(string.empty);
    assert(strcmp(communicator.getProperties().getProperty('Ice.Trace.Network'), '1'));
    communicator.destroy();
    communicator = Ice.initialize(string.empty);
    assert(strcmp(communicator.getProperties().getProperty('Ice.Trace.Network'), '1'));
    communicator.destroy();
    communicator = Ice.Communicator("--Ice.Trace.Protocol=2");
    assert(strcmp(communicator.getProperties().getProperty('Ice.Trace.Network'), '1'));
    communicator.destroy();

    % --Ice.Config replaces ICE_CONFIG.
    communicator = Ice.Communicator("--Ice.Config=" + fullfile(configDir, 'iceConfigOption.cfg'));
    assert(strcmp(communicator.getProperties().getProperty('Ice.Trace.Protocol'), '1'));
    assert(isempty(communicator.getProperties().getProperty('Ice.Trace.Network')));
    communicator.destroy();
    clear restoreIceConfig;
    fprintf('ok\n');

    fprintf("testing load properties exception... ");
    props = Ice.Properties();
    try
        props.load('./config/xxxx.config')
        assert(false)
    catch ex
        % We don't define Ice.FileException in MATLAB. This allows us to test the conversion of
        % unmapped C++ local exceptions to Ice.LocalException.
        assert(strcmp(ex.identifier, 'Ice:FileException'));
        assert(isa(ex, 'Ice.LocalException'));
    end
    fprintf('ok\n');

    fprintf('testing ice properties with set default values...');
    props = Ice.createProperties();
    toStringMode = props.getIceProperty('Ice.ToStringMode');
    assert(strcmp(toStringMode, 'Unicode'));
    closeTimeout = props.getIcePropertyAsInt('Ice.Connection.Client.CloseTimeout');
    assert(closeTimeout == 10);
    retryIntervals = props.getIcePropertyAsList('Ice.RetryIntervals');
    assert(length(retryIntervals) == 1);
    assert(strcmp(retryIntervals{1}, '0'));
    fprintf('ok\n');

    fprintf('testing ice properties with unset default values...');
    stringValue = props.getIceProperty('Ice.Admin.Router');
    assert(strcmp(stringValue, ''));
    intValue = props.getIcePropertyAsInt('Ice.Admin.Router');
    assert(intValue == 0);
    listValue = props.getIcePropertyAsList('Ice.Admin.Router');
    assert(length(listValue) == 0);
    fprintf('ok\n');

    fprintf('testing that getting an unknown ice property throws an exception...');
    try
        props.getIceProperty('Ice.UnknownProperty');
        assert(false);
    catch ex
        assert(isa(ex, 'Ice.PropertyException'));
    end
    fprintf('ok\n');

    fprintf('testing that trying to read a non-numeric value as an int throws... ');
    try
        props.setProperty('Foo', 'bar');
        props.getPropertyAsInt('Foo');
        assert(false);
    catch ex
        assert(isa(ex, 'Ice.PropertyException'));
    end
    fprintf('ok\n');

    fprintf('testing Ice.ProgramName default... ');
    communicator = Ice.initialize();
    programName = communicator.getProperties().getIceProperty('Ice.ProgramName');
    assert(strcmp(programName, 'MATLAB') || strcmp(programName, 'MATLAB.exe'));
    communicator.destroy();
    fprintf('ok\n');
end
