% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    customSliceLoader = CustomSliceLoader();
    properties = Ice.Properties(args);
    properties.setProperty('Ice.SliceLoader.NotFoundCacheSize', '5');
    properties.setProperty('Ice.Warn.SliceLoader', '0'); % comment out to see the warning

    communicator = helper.initialize(Properties = properties, SliceLoader = customSliceLoader);
    cleanup = onCleanup(@() communicator.destroy());
    test = AllTests.allTests(helper, customSliceLoader);
    test.shutdown();

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
