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
    initData.properties_.setProperty('Ice.ThreadPool.Client.Size', '2');
    initData.properties_.setProperty('Ice.ThreadPool.Client.SizeWarn', '0');
    initData.properties_.setProperty('Ice.BatchAutoFlushSize', '100');
    communicator = Ice.initialize(initData);
    cleanup = onCleanup(@() communicator.destroy());

    app = TestApp(communicator);

    myClass = AllTests.allTests(app);

    fprintf('testing server shutdown... ');
    myClass.shutdown();
    try
        myClass.ice_timeout(100).ice_ping(); % Use timeout to speed up testing on Windows
        throw(MException());
    catch ex
        if isa(ex, 'Ice.LocalException')
            fprintf('ok\n');
        else
            rethrow(ex);
        end
    end

    clear('classes'); % Avoids conflicts with tests that define the same symbols.
end
