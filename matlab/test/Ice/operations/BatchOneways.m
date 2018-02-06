%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef BatchOneways
    methods(Static)
        function batchOneways(app, p)
            import Test.*;

            communicator = app.communicator();
            properties = communicator.getProperties();
            bs1 = zeros(1, 10  * 1024);

            batch = p.ice_batchOneway();
            batch.ice_flushBatchRequests(); % Empty flush

            if ~isempty(batch.ice_getConnection())
                batch.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);
            end
            communicator.flushBatchRequests(Ice.CompressBatch.BasedOnProxy);

            p.opByteSOnewayCallCount(); % Reset the call count

            for i = 1:30
                try
                    batch.opByteSOneway(bs1);
                catch ex
                    if isa(ex, 'Ice.MemoryLimitException')
                        assert(false);
                    else
                        rethrow(ex);
                    end
                end
            end

            count = 0;
            while count < 27 % 3 * 9 requests auto-flushed.
                count = count + p.opByteSOnewayCallCount();
                pause(0.1);
            end

            if ~isempty(batch.ice_getConnection())
                batch1 = p.ice_batchOneway();
                batch2 = p.ice_batchOneway();

                batch1.ice_ping();
                batch2.ice_ping();
                batch1.ice_flushBatchRequests();
                batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                batch1.ice_ping();
                batch2.ice_ping();

                batch1.ice_getConnection();
                batch2.ice_getConnection();

                batch1.ice_ping();
                batch1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                batch1.ice_ping();
                batch2.ice_ping();
            end

            identity = Ice.Identity();
            identity.name = 'invalid';
            batch3 = batch.ice_identity(identity);
            batch3.ice_ping();
            batch3.ice_flushBatchRequests();

            % Make sure that a bogus batch request doesn't cause troubles to other ones.
            batch3.ice_ping();
            batch.ice_ping();
            batch.ice_flushBatchRequests();
            batch.ice_ping();

            p.ice_ping();
            if ~isempty(p.ice_getConnection()) && isempty(properties.getProperty('Ice.Override.Compress'))
                prx = p.ice_getConnection().createProxy(p.ice_getIdentity()).ice_batchOneway();

                batchC1 = MyClassPrx.uncheckedCast(prx.ice_compress(false));
                batchC2 = MyClassPrx.uncheckedCast(prx.ice_compress(true));
                batchC3 = MyClassPrx.uncheckedCast(prx.ice_identity(identity));

                batchC1.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.Yes);

                batchC2.opByteSOneway(bs1);
                batchC2.opByteSOneway(bs1);
                batchC2.opByteSOneway(bs1);
                batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.No);

                batchC1.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);

                batchC1.opByteSOneway(bs1);
                batchC2.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);

                batchC1.opByteSOneway(bs1);
                batchC3.opByteSOneway(bs1);
                batchC1.opByteSOneway(bs1);
                batchC1.ice_getConnection().flushBatchRequests(Ice.CompressBatch.BasedOnProxy);
            end
        end
    end
end
