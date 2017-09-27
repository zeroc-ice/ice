%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Future < IceInternal.WrapperObject
    properties(SetAccess=private)
        ID = 0
        NumOutputArguments
        Operation
        Read = false
        State = 'running'
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
                nextId = 0;
                obj.ID = 0;
            end
            nextId = nextId + 1;
        end
        function delete(obj)
            if ~isempty(obj.impl_)
                obj.call_('_release');
            end
            obj.impl_ = [];
        end
        function ok = wait(obj)
            if ~isempty(obj.impl_)
                okPtr = libpointer('uint8Ptr', 0); % Output param
                obj.call_('wait', okPtr);
                ok = okPtr.Value == 1;
            else
                ok = true;
            end
        end
        function varargout = fetchOutputs(obj)
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
            if ~isempty(obj.impl_)
                obj.call_('cancel');
            end
        end
        function r = get.State(obj)
            if ~isempty(obj.impl_)
                obj.State = obj.callWithResult_('state');
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
