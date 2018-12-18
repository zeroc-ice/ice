%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

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
    % This test kills connections, so we don't want warnings.
    %
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
