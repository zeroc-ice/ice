% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    properties = helper.createTestProperties(args);
    properties.setProperty('Ice.Warn.Connections', '0');
    properties.setProperty('Ice.MessageSizeMax', '10'); % 10KB max
    communicator = helper.initialize(Properties = properties);
    cleanup = onCleanup(@() communicator.destroy());

    thrower = AllTests.allTests(helper);
    thrower.shutdown();

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
