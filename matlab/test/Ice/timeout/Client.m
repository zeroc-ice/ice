% Copyright (c) ZeroC, Inc.

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    properties = helper.createTestProperties(args);

    %
    % For this test, we want to disable retries.
    %
    properties.setProperty('Ice.RetryIntervals', '-1');

    %
    % Limit the send buffer size, this test relies on the socket
    % send() blocking after sending a given amount of data.
    %
    properties.setProperty('Ice.TCP.SndSize', '50000');

    communicator = helper.initialize(Properties = properties);
    cleanup = onCleanup(@() communicator.destroy());

    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
