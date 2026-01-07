%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    properties = helper.createTestProperties(args);
    properties.setProperty('Ice.Warn.Connections', '0');
    communicator = helper.initialize(properties);
    cleanup = onCleanup(@() communicator.destroy());

    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
