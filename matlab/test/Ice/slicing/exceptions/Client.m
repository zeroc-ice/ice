%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    communicator = helper.initialize(args);
    cleanup = onCleanup(@() communicator.destroy());
    test = AllTests.allTests(helper);
    test.shutdown();

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
