%{
**********************************************************************

Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.

This copy of Ice is licensed to you under the terms described in the
ICE_LICENSE file included in this distribution.

**********************************************************************
%}

classdef Future < handle
    properties(SetAccess=private)
        ID = 0
        NumOutputArguments
        Operation
        Read = false
        State = 'running'
    end
    methods
        function obj = Future(impl, op, numOutArgs, type, fetchFunc)
            obj.impl = impl;
            obj.Operation = op;
            obj.NumOutputArguments = numOutArgs;
            obj.type_ = type;
            obj.fetchFunc_ = fetchFunc;
            idPtr = libpointer('uint64Ptr', 0);
            Ice.Util.callMethodOnType(obj, obj.type_, 'id', idPtr);
            obj.ID = idPtr.Value;
        end
        function delete(obj)
            if ~isempty(obj.impl)
                Ice.Util.callMethodOnType(obj, obj.type_, '_release');
            end
        end
        function ok = wait(obj)
            okPtr = libpointer('uint8Ptr', 0); % Output param
            Ice.Util.callMethodOnType(obj, obj.type_, 'wait', okPtr);
            ok = okPtr.Value == 1;
        end
        function varargout = fetchOutputs(obj)
            if obj.Read
                throw(MException('Ice:InvalidStateException', 'outputs already read'));
            end
            if obj.NumOutputArguments == 0
                Ice.Util.callMethodOnType(obj, obj.type_, 'check');
            else
                [varargout{1:nargout}] = obj.fetchFunc_(obj);
            end
            obj.Read = true;
        end
        function cancel(obj)
            Ice.Util.callMethodOnType(obj, obj.type_, 'cancel');
        end
        function r = get.State(obj)
            obj.State = Ice.Util.callMethodOnTypeWithResult(obj, obj.type_, 'state');
            r = obj.State;
        end
    end
    properties(Access=private)
        type_
        fetchFunc_
    end
    properties(Hidden,SetAccess=protected)
        impl
    end
end
