% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();

    % We need to use the ClassSliceLoader for the classes with compact IDs. Naturally, it also works for classes
    % without a compact ID.
    initData = Ice.InitializationData(Properties = Ice.createProperties(args), ...
        SliceLoader = Ice.CompositeSliceLoader(CustomSliceLoader(), ...
            Ice.ClassSliceLoader(?DI, ?Test.Compact, ?Test.CompactExt)));

    initData.Properties.setProperty('Ice.Warn.Connections', '0');

    communicator = helper.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());

    initial = AllTests.allTests(helper);
    initial.shutdown();

    %
    % Local tests
    %
    LocalTests.localTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
