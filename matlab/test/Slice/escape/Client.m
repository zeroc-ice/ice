% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    sliceLoader = Ice.ClassSliceLoader(?classdef_.logical_, ?classdef_.escaped_xor, ?classdef_.Base, ...
        ?classdef_.Derived, ?classdef_.bitand_, ?classdef_.escaped_bitor);

    communicator = helper.initialize(args, SliceLoader = sliceLoader);
    cleanup = onCleanup(@() communicator.destroy());
    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
