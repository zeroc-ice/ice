% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    communicator = helper.initialize(args);
    cleanup = onCleanup(@() communicator.destroy());
    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
