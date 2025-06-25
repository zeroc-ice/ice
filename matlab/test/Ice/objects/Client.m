% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();

    % We need to use the ClassSliceLoader for the classes with compact IDs. Naturally, it also works for classes
    % without a compact ID.
    properties = Ice.Properties(args);
    properties.setProperty('Ice.Warn.Connections', '0');

    sliceLoader = Ice.CompositeSliceLoader(CustomSliceLoader(), ...
        Ice.ClassSliceLoader(?DI, ?Test.Compact, ?Test.CompactExt));

    communicator = helper.initialize(Properties = properties, SliceLoader = sliceLoader);
    cleanup = onCleanup(@() communicator.destroy());

    initial = AllTests.allTests(helper);
    initial.shutdown();

    %
    % Local tests
    %
    LocalTests.localTests(helper);
end
