%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

function Client(args)
    addpath('generated');
    addpath('../../lib');
    if ~libisloaded('ice')
        loadlibrary('ice', @iceproto)
    end

    initData = TestApp.createInitData('Client', args);
    initData.properties_.setProperty('Ice.Warn.Connections', '0');
    communicator = Ice.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());

    app = TestApp(communicator);
    AllTests.allTests(app);

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
