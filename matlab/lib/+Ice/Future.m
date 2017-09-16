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
            obj = obj@IceInternal.WrapperObject(impl, type);
            obj.Operation = op;
            obj.NumOutputArguments = numOutArgs;
            obj.fetchFunc = fetchFunc;
            idPtr = libpointer('uint64Ptr', 0);
            obj.call_('id', idPtr);
            obj.ID = idPtr.Value;
        end
        function delete(obj)
            if ~isempty(obj.impl_)
                obj.call_('_release');
            end
            obj.impl_ = [];
        end
        function ok = wait(obj)
            okPtr = libpointer('uint8Ptr', 0); % Output param
            obj.call_('wait', okPtr);
            ok = okPtr.Value == 1;
        end
        function varargout = fetchOutputs(obj)
            if obj.Read
                throw(MException('Ice:InvalidStateException', 'outputs already read'));
            end
            if ~isempty(obj.fetchFunc)
                [varargout{1:obj.NumOutputArguments}] = obj.fetchFunc(obj);
            end
            obj.Read = true;
        end
        function cancel(obj)
            obj.call_('cancel');
        end
        function r = get.State(obj)
            obj.State = obj.callWithResult_('state');
            r = obj.State;
        end
    end
    properties(Access=private)
        fetchFunc
    end
end
