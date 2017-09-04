%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef BatchOnewaysAMI
    methods(Static)
        function batchOneways(app, p)
            communicator = app.communicator();
            properties = communicator.getProperties();
            bs1 = zeros(1, 10  * 1024);

            batch = p.ice_batchOneway();
            future = batch.ice_flushBatchRequestsAsync(); % Empty flush
            future.wait();
            assert(strcmp(future.State, 'finished'));

            futures = {};
            for i = 1:30
                futures{i} = batch.opByteSOnewayAsync(bs1);
            end
            for i = 1:30
                assert(futures{i}.wait());
            end

            count = 0;
            while count < 27 % 3 * 9 requests auto-flushed.
                count = count + p.opByteSOnewayCallCount();
                pause(0.1);
            end

            if ~isempty(batch.ice_getConnection())
                batch2 = p.ice_batchOneway();

                batch.ice_pingAsync();
                batch2.ice_pingAsync();
                assert(batch.ice_flushBatchRequestsAsync().wait());
                batch.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                batch.ice_pingAsync();
                batch2.ice_pingAsync();

                batch.ice_getConnection();
                batch2.ice_getConnection();

                batch.ice_pingAsync();
                batch.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                assert(batch.ice_pingAsync().wait());
                assert(batch2.ice_pingAsync().wait());
            end

            identity = Ice.Identity();
            identity.name = 'invalid';
            batch3 = batch.ice_identity(identity);
            batch3.ice_pingAsync();
            assert(batch3.ice_flushBatchRequestsAsync().wait());

            % Make sure that a bogus batch request doesn't cause troubles to other ones.
            batch3.ice_pingAsync();
            batch.ice_pingAsync();
            assert(batch.ice_flushBatchRequestsAsync().wait());
            batch.ice_pingAsync();
        end
    end
end
