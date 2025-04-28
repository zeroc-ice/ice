% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    initData = Ice.InitializationData();
    initData.properties_ = helper.createTestProperties(args);
    customSliceLoader = CustomSliceLoader();
    initData.sliceLoader = customSliceLoader;

    communicator = helper.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());
    test = AllTests.allTests(helper, customSliceLoader);
    test.shutdown();

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
