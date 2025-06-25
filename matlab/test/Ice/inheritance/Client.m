% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    communicator = helper.initialize(args);
    cleanup = onCleanup(@() communicator.destroy());
    initial = AllTests.allTests(helper);
    initial.shutdown();
end
