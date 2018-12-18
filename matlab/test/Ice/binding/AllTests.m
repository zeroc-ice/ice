%{
**********************************************************************

Copyright (c) 2003-present ZeroC, Inc. All rights reserved.

**********************************************************************
%}

classdef AllTests
    methods(Static)
        function r = getAdapterNameWithAMI(test)
            r = test.getAdapterNameAsync().fetchOutputs();
        end

        function r = createTestIntfPrx(adapters)
            endpoints = {};
            test = [];
            for i = 1:length(adapters)
                test = adapters{i}.getTestIntf();
                endpts = test.ice_getEndpoints();
                for j = 1:length(endpts)
                    endpoints{end + 1} = endpts{j};
                end
            end
            r = test.ice_endpoints(endpoints);
        end

        function deactivate(communicator, adapters)
            for i = 1:length(adapters)
                communicator.deactivateObjectAdapter(adapters{i});
            end
        end

        function allTests(helper)
            import Test.*;

            communicator = helper.communicator();

            ref = ['communicator:', helper.getTestEndpoint()];
            rcom = RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref));

            fprintf('testing binding with single endpoint... ');

            adapter = rcom.createObjectAdapter('Adapter', 'default');

            test1 = adapter.getTestIntf();
            test2 = adapter.getTestIntf();
            assert(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();
            test2.ice_ping();

            rcom.deactivateObjectAdapter(adapter);

            test3 = TestIntfPrx.uncheckedCast(test1);
            assert(test3.ice_getConnection() == test1.ice_getConnection());
            assert(test3.ice_getConnection() == test2.ice_getConnection());

            try
                test3.ice_ping();
                assert(false);
            catch ex
                if isa(ex, 'Ice.ConnectFailedException')
                    %
                    % Usually the actual type of this exception is ConnectionRefusedException,
                    % but not always. See bug 3179.
                    %
                elseif isa(ex, 'Ice.ConnectTimeoutException')
                    %
                    % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                    %
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('testing binding with multiple endpoints... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter11', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter12', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter13', 'default');

            %
            % Ensure that when a connection is opened it's reused for new
            % proxies and that all endpoints are eventually tried.
            %
            names = { 'Adapter11', 'Adapter12', 'Adapter13' };
            while length(names) > 0
                adpts = adapters;

                test1 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test2 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test3 = AllTests.createTestIntfPrx(adpts);

                assert(test1.ice_getConnection() == test2.ice_getConnection());
                assert(test2.ice_getConnection() == test3.ice_getConnection());

                pos = find(ismember(names, test1.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Ensure that the proxy correctly caches the connection (we
            % always send the request over the same connection.)
            %
            for i = 1:length(adapters)
                adapters{i}.getTestIntf().ice_ping();
            end

            test = AllTests.createTestIntfPrx(adapters);
            name = test.getAdapterName();
            nRetry = 10;
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), name)
                i = i + 1;
            end
            assert(i == nRetry);

            for i = 1:length(adapters)
                adapters{i}.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Deactivate an adapter and ensure that we can still
            % establish the connection to the remaining adapters.
            %
            rcom.deactivateObjectAdapter(adapters{1});
            names{end + 1} = 'Adapter12';
            names{end + 1} = 'Adapter13';
            while length(names) > 0
                adpts = adapters;

                test1 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test2 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test3 = AllTests.createTestIntfPrx(adpts);

                assert(test1.ice_getConnection() == test2.ice_getConnection());
                assert(test2.ice_getConnection() == test3.ice_getConnection());

                pos = find(ismember(names, test1.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Deactivate an adapter and ensure that we can still
            % establish the connection to the remaining adapter.
            %
            rcom.deactivateObjectAdapter(adapters{3});
            test = AllTests.createTestIntfPrx(adapters);
            assert(strcmp(test.getAdapterName(), 'Adapter12'));

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing binding with multiple random endpoints... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('AdapterRandom11', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterRandom12', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterRandom13', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterRandom14', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterRandom15', 'default');

            shortenTest = ispc();

            count = 20;
            adapterCount = length(adapters);
            while count > 0
                count = count - 1;

                if shortenTest
                    if count == 1
                        rcom.deactivateObjectAdapter(adapters{5});
                        adapterCount = adapterCount - 1;
                    end
                else
                    if count < 20 && mod(count, 4) == 0
                        rcom.deactivateObjectAdapter(adapters{idivide(int32(count), 4)});
                        adapterCount = adapterCount - 1;
                    end
                end

                proxies = cell(1, 10);
                for i = 1:length(proxies)
                    n = randi([1 length(adapters)], 1, 1);
                    adpts = cell(1, n);
                    for j = 1:length(adpts)
                        adpts{j} = adapters{randi([1 length(adapters)], 1, 1)};
                    end
                    proxies{i} = AllTests.createTestIntfPrx(adpts);
                end

                for i = 1:length(proxies)
                    proxies{i}.getAdapterNameAsync();
                end
                for i = 1:length(proxies)
                    try
                        proxies{i}.ice_ping();
                    catch ex
                    end
                end

                connections = {};
                for i = 1:length(proxies)
                    con = proxies{i}.ice_getCachedConnection();
                    if ~isempty(con)
                        if isempty(connections)
                            connections{end + 1} = con;
                        else
                            found = false;
                            for j = 1:length(connections)
                                if connections{j} == con
                                    found = true;
                                    break;
                                end
                            end
                            if ~found
                                connections{end + 1} = con;
                            end
                        end
                    end
                end
                assert(length(connections) <= adapterCount);

                for i = 1:length(adapters)
                    try
                        adapters{i}.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                    catch ex
                        if isa(ex, 'Ice.LocalException')
                            % Expected if adapter is down.
                        else
                            rethrow(ex);
                        end
                    end
                end
            end

            fprintf('ok\n');

            fprintf('testing binding with multiple endpoints and AMI... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI11', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI12', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI13', 'default');

            %
            % Ensure that when a connection is opened it's reused for new
            % proxies and that all endpoints are eventually tried.
            %
            names = { 'AdapterAMI11', 'AdapterAMI12', 'AdapterAMI13' };
            while length(names) > 0
                adpts = adapters;

                test1 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test2 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test3 = AllTests.createTestIntfPrx(adpts);

                assert(test1.ice_getConnection() == test2.ice_getConnection());
                assert(test2.ice_getConnection() == test3.ice_getConnection());

                pos = find(ismember(names, AllTests.getAdapterNameWithAMI(test1)));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Ensure that the proxy correctly caches the connection (we
            % always send the request over the same connection.)
            %
            for i = 1:length(adapters)
                adapters{i}.getTestIntf().ice_ping();
            end

            test = AllTests.createTestIntfPrx(adapters);
            name = AllTests.getAdapterNameWithAMI(test);
            nRetry = 10;
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), name)
                i = i + 1;
            end
            assert(i == nRetry);

            for i = 1:length(adapters)
                adapters{i}.getTestIntf().ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Deactivate an adapter and ensure that we can still
            % establish the connection to the remaining adapters.
            %
            rcom.deactivateObjectAdapter(adapters{1});
            names{end + 1} = 'AdapterAMI12';
            names{end + 1} = 'AdapterAMI13';
            while length(names) > 0
                adpts = adapters;

                test1 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test2 = AllTests.createTestIntfPrx(adpts);
                adpts = adpts(randperm(length(adpts)));
                test3 = AllTests.createTestIntfPrx(adpts);

                assert(test1.ice_getConnection() == test2.ice_getConnection());
                assert(test2.ice_getConnection() == test3.ice_getConnection());

                pos = find(ismember(names, AllTests.getAdapterNameWithAMI(test1)));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test1.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            %
            % Deactivate an adapter and ensure that we can still
            % establish the connection to the remaining adapter.
            %
            rcom.deactivateObjectAdapter(adapters{3});
            test = AllTests.createTestIntfPrx(adapters);
            assert(strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI12'));

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing random endpoint selection... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter21', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter22', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter23', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            assert(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            names = { 'Adapter21', 'Adapter22', 'Adapter23' };
            while length(names) > 0
                pos = find(ismember(names, test.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            test = test.ice_endpointSelection(Ice.EndpointSelectionType.Random);
            assert(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Random);

            names = { 'Adapter21', 'Adapter22', 'Adapter23' };
            while length(names) > 0
                pos = find(ismember(names, test.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
                test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            end

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing ordered endpoint selection... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter31', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter32', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter33', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
            assert(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);

            %
            % Ensure that endpoints are tried in order by deactiving the adapters
            % one after the other.
            %
            nRetry = 5;
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter31')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{1});
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter32')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{2});
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter33')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{3});

            try
                test.getAdapterName();
            catch ex
                if isa(ex, 'Ice.ConnectFailedException')
                    %
                    % Usually the actual type of this exception is ConnectionRefusedException,
                    % but not always. See bug 3179.
                    %
                elseif isa(ex, 'Ice.ConnectTimeoutException')
                    %
                    % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                    %
                else
                    rethrow(ex);
                end
            end

            endpoints = test.ice_getEndpoints();

            adapters = {};

            %
            % Now, re-activate the adapters with the same endpoints in the opposite
            % order.
            %
            adapters{end + 1} = rcom.createObjectAdapter('Adapter36', endpoints{3}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter36')
                i = i + 1;
            end
            assert(i == nRetry);
            test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            adapters{end + 1} = rcom.createObjectAdapter('Adapter35', endpoints{2}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter35')
                i = i + 1;
            end
            assert(i == nRetry);
            test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
            adapters{end + 1} = rcom.createObjectAdapter('Adapter34', endpoints{1}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter34')
                i = i + 1;
            end
            assert(i == nRetry);

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing per request binding with single endpoint... ');

            adapter = rcom.createObjectAdapter('Adapter41', 'default');

            test1 = adapter.getTestIntf().ice_connectionCached(false);
            test2 = adapter.getTestIntf().ice_connectionCached(false);
            assert(~test1.ice_isConnectionCached());
            assert(~test2.ice_isConnectionCached());
            assert(~isempty(test1.ice_getConnection()) && ~isempty(test2.ice_getConnection()));
            assert(test1.ice_getConnection() == test2.ice_getConnection());

            test1.ice_ping();

            rcom.deactivateObjectAdapter(adapter);

            test3 = TestIntfPrx.uncheckedCast(test1);
            try
                assert(test3.ice_getConnection() == test1.ice_getConnection());
                assert(false);
            catch ex
                if isa(ex, 'Ice.ConnectFailedException')
                    %
                    % Usually the actual type of this exception is ConnectionRefusedException,
                    % but not always. See bug 3179.
                    %
                elseif isa(ex, 'Ice.ConnectTimeoutException')
                    %
                    % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                    %
                else
                    rethrow(ex);
                end
            end

            fprintf('ok\n');

            fprintf('testing per request binding with multiple endpoints... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter51', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter52', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter53', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            test = test.ice_connectionCached(false);
            assert(~test.ice_isConnectionCached());

            names = { 'Adapter51', 'Adapter52', 'Adapter53' };
            while length(names) > 0
                pos = find(ismember(names, test.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
            end

            rcom.deactivateObjectAdapter(adapters{1});

            names = { 'Adapter52', 'Adapter53' };
            while length(names) > 0
                pos = find(ismember(names, test.getAdapterName()));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
            end

            rcom.deactivateObjectAdapter(adapters{3});

            assert(strcmp(test.getAdapterName(), 'Adapter52'));

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing per request binding with multiple endpoints and AMI... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI51', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI52', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI53', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            test = test.ice_connectionCached(false);
            assert(~test.ice_isConnectionCached());

            names = { 'AdapterAMI51', 'AdapterAMI52', 'AdapterAMI53' };
            while length(names) > 0
                pos = find(ismember(names, AllTests.getAdapterNameWithAMI(test)));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
            end

            rcom.deactivateObjectAdapter(adapters{1});

            names = { 'AdapterAMI52', 'AdapterAMI53' };
            while length(names) > 0
                pos = find(ismember(names, AllTests.getAdapterNameWithAMI(test)));
                if ~isempty(pos)
                    names(pos) = []; % Using the () syntax removes the element from the cell array.
                end
            end

            rcom.deactivateObjectAdapter(adapters{3});

            assert(strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI52'));

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing per request binding and ordered endpoint selection... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter61', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter62', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter63', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
            assert(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            test = test.ice_connectionCached(false);
            assert(~test.ice_isConnectionCached());

            %
            % Ensure that endpoints are tried in order by deactiving the adapters
            % one after the other.
            %
            nRetry = 5;
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter61')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{1});
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter62')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{2});
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter63')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{3});

            try
                test.getAdapterName();
            catch ex
                if isa(ex, 'Ice.ConnectFailedException')
                    %
                    % Usually the actual type of this exception is ConnectionRefusedException,
                    % but not always. See bug 3179.
                    %
                elseif isa(ex, 'Ice.ConnectTimeoutException')
                    %
                    % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                    %
                else
                    rethrow(ex);
                end
            end

            endpoints = test.ice_getEndpoints();

            adapters = {};

            %
            % Now, re-activate the adapters with the same endpoints in the opposite
            % order.
            %
            adapters{end + 1} = rcom.createObjectAdapter('Adapter66', endpoints{3}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter66')
                i = i + 1;
            end
            assert(i == nRetry);
            adapters{end + 1} = rcom.createObjectAdapter('Adapter65', endpoints{2}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter65')
                i = i + 1;
            end
            assert(i == nRetry);
            adapters{end + 1} = rcom.createObjectAdapter('Adapter64', endpoints{1}.toString());
            i = 0;
            while i < nRetry && strcmp(test.getAdapterName(), 'Adapter64')
                i = i + 1;
            end
            assert(i == nRetry);

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing per request binding and ordered endpoint selection and AMI... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI61', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI62', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI63', 'default');

            test = AllTests.createTestIntfPrx(adapters);
            test = test.ice_endpointSelection(Ice.EndpointSelectionType.Ordered);
            assert(test.ice_getEndpointSelection() == Ice.EndpointSelectionType.Ordered);
            test = test.ice_connectionCached(false);
            assert(~test.ice_isConnectionCached());

            %
            % Ensure that endpoints are tried in order by deactiving the adapters
            % one after the other.
            %
            nRetry = 5;
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI61')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{1});
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI62')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{2});
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI63')
                i = i + 1;
            end
            assert(i == nRetry);
            rcom.deactivateObjectAdapter(adapters{3});

            try
                test.getAdapterName();
            catch ex
                if isa(ex, 'Ice.ConnectFailedException')
                    %
                    % Usually the actual type of this exception is ConnectionRefusedException,
                    % but not always. See bug 3179.
                    %
                elseif isa(ex, 'Ice.ConnectTimeoutException')
                    %
                    % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                    %
                else
                    rethrow(ex);
                end
            end

            endpoints = test.ice_getEndpoints();

            adapters = {};

            %
            % Now, re-activate the adapters with the same endpoints in the opposite
            % order.
            %
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI66', endpoints{3}.toString());
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI66')
                i = i + 1;
            end
            assert(i == nRetry);
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI65', endpoints{2}.toString());
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI65')
                i = i + 1;
            end
            assert(i == nRetry);
            adapters{end + 1} = rcom.createObjectAdapter('AdapterAMI64', endpoints{1}.toString());
            i = 0;
            while i < nRetry && strcmp(AllTests.getAdapterNameWithAMI(test), 'AdapterAMI64')
                i = i + 1;
            end
            assert(i == nRetry);

            AllTests.deactivate(rcom, adapters);

            fprintf('ok\n');

            fprintf('testing endpoint mode filtering... ');

            adapters = {};
            adapters{end + 1} = rcom.createObjectAdapter('Adapter71', 'default');
            adapters{end + 1} = rcom.createObjectAdapter('Adapter72', 'udp');

            test = AllTests.createTestIntfPrx(adapters);
            assert(strcmp(test.getAdapterName(), 'Adapter71'));

            testUDP = test.ice_datagram();
            assert(test.ice_getConnection() ~= testUDP.ice_getConnection());
            try
                testUDP.getAdapterName();
            catch ex
                assert(isa(ex, 'Ice.TwowayOnlyException'));
            end

            fprintf('ok\n');

            if length(communicator.getProperties().getProperty('Ice.Plugin.IceSSL')) > 0
                fprintf('testing unsecure vs. secure endpoints... ');

                adapters = {};
                adapters{end + 1} = rcom.createObjectAdapter('Adapter81', 'ssl');
                adapters{end + 1} = rcom.createObjectAdapter('Adapter82', 'tcp');

                test = AllTests.createTestIntfPrx(adapters);
                for i = 1:5
                    assert(strcmp(test.getAdapterName(), 'Adapter82'));
                    test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                end

                testSecure = test.ice_secure(true);
                assert(testSecure.ice_isSecure());
                testSecure = test.ice_secure(false);
                assert(~testSecure.ice_isSecure());
                testSecure = test.ice_secure(true);
                assert(testSecure.ice_isSecure());
                assert(test.ice_getConnection() ~= testSecure.ice_getConnection());

                rcom.deactivateObjectAdapter(adapters{2});

                for i = 1:5
                    assert(strcmp(test.getAdapterName(), 'Adapter81'));
                    test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                end

                endpts = test.ice_getEndpoints();
                rcom.createObjectAdapter('Adapter83', endpts{2}.toString()); % Reactivate tcp OA.

                for i = 1:5
                    assert(strcmp(test.getAdapterName(), 'Adapter83'));
                    test.ice_getConnection().close(Ice.ConnectionClose.GracefullyWithWait);
                end

                rcom.deactivateObjectAdapter(adapters{1});
                try
                    testSecure.ice_ping();
                    assert(false);
                catch ex
                    if isa(ex, 'Ice.ConnectFailedException')
                        %
                        % Usually the actual type of this exception is ConnectionRefusedException,
                        % but not always. See bug 3179.
                        %
                    elseif isa(ex, 'Ice.ConnectTimeoutException')
                        %
                        % On Windows, we set Ice.Override.ConnectTimeout to speed up testing.
                        %
                    else
                        rethrow(ex);
                    end
                end

                AllTests.deactivate(rcom, adapters);

                fprintf('ok\n');
            end

            rcom.shutdown();
        end
    end
end
