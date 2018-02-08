%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function client(args)
    addpath('generated');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    initData = TestApp.createInitData('client', args);

    %
    % For this test, we want to disable retries.
    %
    initData.properties_.setProperty('Ice.RetryIntervals', '-1');

    %
    % This test kills connections, so we don't want warnings.
    %
    initData.properties_.setProperty('Ice.Warn.Connections', '0');

    %
    % Limit the send buffer size, this test relies on the socket
    % send() blocking after sending a given amount of data.
    %
    initData.properties_.setProperty('Ice.TCP.SndSize', '50000');

    communicator = Ice.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());

    app = TestApp(communicator);
    AllTests.allTests(app);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
