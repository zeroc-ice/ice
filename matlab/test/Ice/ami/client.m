%
% Copyright (c) ZeroC, Inc. All rights reserved.
%

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    helper = TestHelper();
    properties = helper.createTestProperties(args);
    properties.setProperty('Ice.Warn.AMICallback', '0');
    properties.setProperty('Ice.Warn.Connections', '0');
    %
    % Limit the send buffer size, this test relies on the socket
    % send() blocking after sending a given amount of data.
    %
    properties.setProperty('Ice.TCP.SndSize', '50000');
    communicator = helper.initialize(properties);
    cleanup = onCleanup(@() communicator.destroy());
    AllTests.allTests(helper);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
