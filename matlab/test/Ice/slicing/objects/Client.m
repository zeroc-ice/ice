% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    customSliceLoader = CustomSliceLoader();

    initData = Ice.InitializationData(SliceLoader = customSliceLoader, Properties = Ice.createProperties(args));

    % Use deprecated properties_ field to check it still works.
    initData.properties_.setProperty('Ice.SliceLoader.NotFoundCacheSize', '5');
    initData.properties_.setProperty('Ice.Warn.SliceLoader', '0'); % comment out to see the warning

    communicator = helper.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());
    test = AllTests.allTests(helper, customSliceLoader);
    test.shutdown();

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
