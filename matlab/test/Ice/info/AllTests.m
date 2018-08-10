%{
**********************************************************************

Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            fprintf('testing proxy endpoint information... ');

            p1 = communicator.stringToProxy(...
                    ['test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:', ...
                    'udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:', ...
                    'opaque -e 1.8 -t 100 -v ABCD']);

            endps = p1.ice_getEndpoints();
            info = endps{1}.getInfo();
            tcpEndpoint = getTCPEndpointInfo(info);
            assert(strcmp(tcpEndpoint.host, 'tcphost'));
            assert(tcpEndpoint.port == 10000);
            assert(tcpEndpoint.timeout == 1200);
            assert(strcmp(tcpEndpoint.sourceAddress, '10.10.10.10'));
            assert(tcpEndpoint.compress);
            assert(~tcpEndpoint.datagram());
            assert(tcpEndpoint.type() == Ice.TCPEndpointType.value && ~tcpEndpoint.secure() || ...
                   tcpEndpoint.type() == Ice.SSLEndpointType.value && tcpEndpoint.secure() || ...
                   tcpEndpoint.type() == Ice.WSEndpointType.value && ~tcpEndpoint.secure() || ...
                   tcpEndpoint.type() == Ice.WSSEndpointType.value && tcpEndpoint.secure());

            % TODO: With SSL types
                 %tcpEndpoint.type() == Ice.SSLEndpointType.value && isa(info, 'IceSSL.EndpointInfo') ||
            assert(tcpEndpoint.type() == Ice.TCPEndpointType.value && isa(info, 'Ice.TCPEndpointInfo') || ...
                   tcpEndpoint.type() == Ice.SSLEndpointType.value && isa(info, 'IceSSL.EndpointInfo') || ...
                   tcpEndpoint.type() == Ice.WSEndpointType.value && isa(info, 'Ice.WSEndpointInfo') || ...
                   tcpEndpoint.type() == Ice.WSSEndpointType.value && isa(info, 'Ice.WSEndpointInfo'));

            udpEndpoint = endps{2}.getInfo();
            assert(strcmp(udpEndpoint.host, 'udphost'));
            assert(udpEndpoint.port == 10001);
            assert(strcmp(udpEndpoint.mcastInterface, 'eth0'));
            assert(udpEndpoint.mcastTtl == 5);
            assert(strcmp(udpEndpoint.sourceAddress, '10.10.10.10'));
            assert(udpEndpoint.timeout == -1);
            assert(~udpEndpoint.compress);
            assert(~udpEndpoint.secure());
            assert(udpEndpoint.datagram());
            assert(udpEndpoint.type() == Ice.UDPEndpointType.value);

            opaqueEndpoint = endps{3}.getInfo();
            assert(opaqueEndpoint.rawEncoding == Ice.EncodingVersion(1, 8));

            fprintf('ok\n');

            base = communicator.stringToProxy(['test:', helper.getTestEndpoint(), ':', helper.getTestEndpoint('udp')]);
            testIntf = TestIntfPrx.checkedCast(base);

            endpointPort = helper.getTestPort();

            defaultHost = communicator.getProperties().getProperty('Ice.Default.Host');
            fprintf('test connection endpoint information... ');

            info = base.ice_getConnection().getEndpoint().getInfo();
            tcpinfo = getTCPEndpointInfo(info);
            assert(tcpinfo.port == endpointPort);
            assert(~tcpinfo.compress);
            assert(strcmp(tcpinfo.host, defaultHost));

            ctx = testIntf.getEndpointInfoAsContext();
            assert(strcmp(ctx('host'), tcpinfo.host));
            assert(strcmp(ctx('compress'), 'false'));
            port = str2num(ctx('port'));
            assert(port > 0);

            info = base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            udp = info;
            assert(udp.port == endpointPort);
            assert(strcmp(udp.host, defaultHost));

            fprintf('ok\n');

            fprintf('testing connection information... ');

            connection = base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            info = getTCPConnectionInfo(connection.getInfo());
            assert(~info.incoming);
            assert(length(info.adapterName) == 0);
            assert(info.localPort > 0);
            assert(info.remotePort == endpointPort);
            if strcmp(defaultHost, '127.0.0.1')
                assert(strcmp(info.remoteAddress, defaultHost));
                assert(strcmp(info.localAddress, defaultHost));
            end
            assert(info.rcvSize >= 1024);
            assert(info.sndSize >= 2048);

            ctx = testIntf.getConnectionInfoAsContext();
            assert(strcmp(ctx('incoming'), 'true'));
            assert(strcmp(ctx('adapterName'), 'TestAdapter'));
            assert(strcmp(ctx('remoteAddress'), info.localAddress));
            assert(strcmp(ctx('localAddress'), info.remoteAddress));
            assert(strcmp(ctx('remotePort'), num2str(info.localPort)));
            assert(strcmp(ctx('localPort'), num2str(info.remotePort)));

            type = base.ice_getConnection().type();
            if strcmp(base, 'ws') || strcmp(base, 'wss')
                headers = connection.getInfo().headers;
                assert(strcmp(headers('Upgrade'), 'websocket'));
                assert(strcmp(headers('Connection'), 'Upgrade'));
                assert(strcmp(headers('Sec-WebSocket-Protocol'), 'ice.zeroc.com'));
                assert(headers.isKey('Sec-WebSocket-Accept'));

                assert(strcmp(ctx('ws.Upgrade'), 'websocket'));
                assert(strcmp(ctx('ws.Connection'), 'Upgrade'));
                assert(strcmp(ctx('ws.Sec-WebSocket-Protocol'), 'ice.zeroc.com'));
                assert(strcmp(ctx('ws.Sec-WebSocket-Version'), '13'));
                assert(ctx.isKey('ws.Sec-WebSocket-Key'));
            end

            connection = base.ice_datagram().ice_getConnection();
            connection.setBufferSize(2048, 1024);

            udpinfo = connection.getInfo();
            assert(~udpinfo.incoming);
            assert(length(udpinfo.adapterName) == 0);
            assert(udpinfo.localPort > 0);
            assert(udpinfo.remotePort == endpointPort);
            if strcmp(defaultHost, '127.0.0.1')
                assert(strcmp(udpinfo.remoteAddress, defaultHost));
                assert(strcmp(udpinfo.localAddress, defaultHost));
            end
            assert(udpinfo.rcvSize >= 2048);
            assert(udpinfo.sndSize >= 1024);

            fprintf('ok\n');

            testIntf.shutdown();
        end
    end
end

function r = getTCPEndpointInfo(info)
    p = info;
    while ~isempty(p)
        if isa(p, 'Ice.TCPEndpointInfo')
            r = p;
            return;
        end
        p = p.underlying;
    end
    r = [];
end

function r = getTCPConnectionInfo(info)
    p = info;
    while ~isempty(p)
        if isa(p, 'Ice.TCPConnectionInfo')
            r = p;
            return;
        end
        p = p.underlying;
    end
    r = [];
end
