classdef Future < IceInternal.WrapperObject
    %FUTURE Represents the future result of an asynchronous invocation.
    %
    %   Future Properties:
    %     ID - A unique identifier for this object.
    %     NumOutputArguments - The number of output arguments that will be returned by fetchOutputs upon successful completion.
    %     Operation - The name of the operation that was invoked.
    %     Read - True if fetchOutputs has already been called.
    %     State - The current state of the future.
    %
    %   Future Methods:
    %     cancel - If the invocation is still pending, calling this method instructs the local Ice runtime to ignore its results.
    %     fetchOutputs - Blocks until the invocation completes and then returns the results or throws an exception.
    %     wait - Blocks until the invocation reaches a certain state, or a timeout expires.

    % Copyright (c) ZeroC, Inc.

    properties (SetAccess = private)
        %ID A unique identifier for this object.
        %   int32 scalar
        ID (1, 1) int32 = 0

        %NUMOUTPUTARGUMENTS The number of output arguments that will be returned by fetchOutputs upon successful
        %   completion.
        %   int32 scalar
        NumOutputArguments (1, 1) int32

        %OPERATION The name of the operation that was invoked.
        %   character vector
        Operation (1, :) char

        %READ True if fetchOutputs has already been called.
        %   logical scalar
        Read (1, 1) logical = false

        %STATE The current state of the future. Its initial value is 'running' and its final value is 'finished'.
        %   'running' | 'sent' | 'finished'
        State (1, :) char = 'running'
    end
    methods (Hidden, Access = {?Ice.Communicator, ?Ice.Connection, ?Ice.ObjectPrx})
        function obj = Future(impl, op, numOutArgs, type, fetchFunc)
            %
            % The nextId variable is persistent, which acts somewhat like a static variable. It retains its
            % current value as long as there is at least one reference to a Future instance. Once the last
            % reference drops, nextId is cleared.
            %
            persistent nextId;

            obj@IceInternal.WrapperObject(impl, type);
            obj.Operation = op;
            obj.NumOutputArguments = numOutArgs;
            obj.fetchFunc = fetchFunc;

            if nextId
                obj.ID = nextId;
            else
                nextId = int32(0);
                obj.ID = 0;
            end
            nextId = nextId + 1;
        end
    end
    methods (Hidden)
        function delete(obj)
            if ~isempty(obj.impl_)
                obj.iceCall('unref');
            end
            obj.impl_ = [];
        end
    end
    methods
        function ok = wait(obj, state, timeout)
            %WAIT Blocks until the invocation reaches a certain state, or a timeout expires.
            %
            %   Input Arguments
            %     state - If provided, wait blocks until the future reaches the given state. If not provided, wait
            %     blocks until the state is 'finished'. Note that the future enters the 'finished' state when completed
            %     successfully or exceptionally.
            %     'running' | 'sent' | 'finished' | empty array (default)
            %   timeout - If provided, wait blocks up to the given number of seconds while waiting for the future to
            %     reach the desired state. If the timeout is negative or not provided, wait blocks indefinitely.
            %     double | empty array (default)
            %
            %    Output Arguments
            %      ok - True if the future reached the desired state, false if the future has not reached the desired
            %        state or an exception occurred.
            %        logical scalar

            if ~isempty(obj.impl_)
                if nargin == 1
                    ok = obj.iceCallWithResult('wait');
                elseif nargin == 2
                    ok = obj.iceCallWithResult('waitState', state, -1);
                else
                    ok = obj.iceCallWithResult('waitState', state, timeout);
                end
            else
                ok = true;
            end
        end

        function varargout = fetchOutputs(obj)
            %FETCHOUTPUTS Blocks until the invocation completes and then returns the results or throws an exception.
            %   Can only be called once.
            %
            %   Output Arguments
            %     varargout - The output arguments of the invocation.

            if obj.Read
                error('Ice:InvalidStateException', 'Outputs already read');
            end
            if ~isempty(obj.fetchFunc)
                %
                % We assume the fetch function implementation also deletes the C++ object so that we
                % can avoid another call into C++.
                %
                try
                    [varargout{1:obj.NumOutputArguments}] = obj.fetchFunc(obj);
                    obj.impl_ = [];
                catch ex
                    obj.impl_ = [];
                    rethrow(ex);
                end
            end
            obj.Read = true;
        end

        function cancel(obj)
            %CANCEL If the invocation is still pending, calling this method instructs the local Ice runtime to ignore
            %   its results.

            if ~isempty(obj.impl_)
                obj.iceCall('cancel');
            end
        end
        function r = get.State(obj)
            if ~isempty(obj.impl_)
                obj.State = obj.iceCallWithResult('state');
                r = obj.State;
            else
                r = 'finished';
            end
        end
    end
    properties (Access = private)
        fetchFunc
    end
end
