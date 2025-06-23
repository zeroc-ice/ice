% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    properties = helper.createTestProperties(args);
    properties.setProperty('Ice.ThreadPool.Client.Size', '2');
    properties.setProperty('Ice.ThreadPool.Client.SizeWarn', '0');
    properties.setProperty('Ice.BatchAutoFlushSize', '100');
    communicator = helper.initialize(Properties = properties);
    cleanup = onCleanup(@() communicator.destroy());

    myClass = AllTests.allTests(helper);
    fprintf('testing server shutdown... ');
    myClass.shutdown();
    try
        myClass.ice_invocationTimeout(100).ice_ping(); % Use timeout to speed up testing on Windows
        throw(MException('Ice:TestException', 'expected exception not thrown'));
    catch ex
        if isa(ex, 'Ice.LocalException')
            fprintf('ok\n');
        else
            rethrow(ex);
        end
    end

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
