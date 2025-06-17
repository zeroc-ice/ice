% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();

    % Can even use deprecated properties_ name in constructor (not recommended obviously).
    initData = Ice.InitializationData(properties_ = helper.createTestProperties(args));
    initData.SliceLoader = Ice.ClassSliceLoader([?classdef_.logical_, ?classdef_.escaped_xor, ?classdef_.Base, ...
        ?classdef_.Derived, ?classdef_.bitand_, ?classdef_.escaped_bitor]);

    communicator = helper.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());
    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
