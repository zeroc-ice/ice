classdef Future < IceInternal.WrapperObject
    % Future   Summary of Future
    %
    % Represents an asynchronous invocation.
    %
    % Future Methods:
    %   wait - Block until the invocation reaches a certain state, or a
    %     timeout expires.
    %   fetchOutputs - Block until the invocation completes and then return
    %     the results or raise an exception.
    %   cancel - If the invocation is still pending, calling this method
    %     instructs the local Ice run time to ignore its results.
    %
    % Future Properties:
    %   ID - A unique identifier for this object.
    %   NumOutputArguments - The number of output arguments that will be
    %     returned by fetchOutputs upon successful completion.
    %   Operation - The name of the operation that was invoked.
    %   Read - True if fetchOutputs has already been called.
    %   State - The current state of the future.

    % Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

    properties(SetAccess=private)
        % ID - A unique identifier for this object.
        ID int32 = 0

        % NumOutputArguments - The number of output arguments that will be
        %   returned by fetchOutputs upon successful completion.
        NumOutputArguments int32

        % Operation - The name of the operation that was invoked.
        Operation char

        % Read - True if fetchOutputs has already been called.
        Read logical = false

        % State - The current state of the future. Its initial value is
        %   'running' and its final value is 'finished'. A remote invocation
        %   transitions from 'running' to 'sent' to 'finished'.
        State char = 'running'
    end
    methods
        function obj = Future(impl, op, numOutArgs, type, fetchFunc)
            %
            % The nextId variable is persistent, which acts somewhat like a static variable. It retains its
            % current value as long as there is at least one reference to a Future instance. Once the last
            % reference drops, nextId is cleared.
            %
            persistent nextId;

            obj = obj@IceInternal.WrapperObject(impl, type);
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
        function delete(obj)
            if ~isempty(obj.impl_)
                obj.iceCall('unref');
            end
            obj.impl_ = [];
        end
        function ok = wait(obj, state, timeout)
            % wait - Block until the invocation reaches a certain state, or a
            %   timeout expires.
            %
            % Parameters:
            %   state (char) - If provided, wait blocks until the future reaches
            %     the given state. Must be one of 'running', 'sent', 'finished'.
            %     If not provided, wait blocks until the state is 'finished'.
            %     Note that the future enters the 'finished' state when
            %     completed successfully or exceptionally.
            %   timeout (double) - If provided, wait blocks up to the given
            %     number of seconds while waiting for the future to reach the
            %     desired state. If the timeout is negative or not provided,
            %     wait blocks indefinitely.
            %
            % Returns (logical) - True if the future reached the desired state,
            %   false if the future has not reached the desired state or an
            %   exception occurred.

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
            % fetchOutputs   Block until the invocation completes and then
            %   return the results or raise an exception. Can only be called
            %   once.

            if obj.Read
                throw(MException('Ice:InvalidStateException', 'outputs already read'));
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
            % cancel   If the invocation is still pending, calling this method
            %   instructs the local Ice run time to ignore its results.

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
    properties(Access=private)
        fetchFunc
    end
end
