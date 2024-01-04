% ThreadState   Summary of ThreadState
%
% The thread state enumeration keeps track of the different possible states of Ice threads.
%
% ThreadState Properties:
%   ThreadStateIdle - The thread is idle.
%   ThreadStateInUseForIO - The thread is in use performing reads or writes for Ice connections.
%   ThreadStateInUseForUser - The thread is calling user code (servant implementation, AMI callbacks).
%   ThreadStateInUseForOther - The thread is performing other internal activities (DNS lookups, timer callbacks, etc).

% Copyright (c) ZeroC, Inc. All rights reserved.
% Generated from Instrumentation.ice by slice2matlab version 3.7.10

classdef ThreadState < uint8
    enumeration
        % The thread is idle.
        ThreadStateIdle (0)
        % The thread is in use performing reads or writes for Ice connections. This state is only for threads from an Ice
        % thread pool.
        ThreadStateInUseForIO (1)
        % The thread is calling user code (servant implementation, AMI callbacks). This state is only for threads from an
        % Ice thread pool.
        ThreadStateInUseForUser (2)
        % The thread is performing other internal activities (DNS lookups, timer callbacks, etc).
        ThreadStateInUseForOther (3)
    end
    methods(Static)
        function r = ice_getValue(v)
            switch v
                case 0
                    r = Ice.Instrumentation.ThreadState.ThreadStateIdle;
                case 1
                    r = Ice.Instrumentation.ThreadState.ThreadStateInUseForIO;
                case 2
                    r = Ice.Instrumentation.ThreadState.ThreadStateInUseForUser;
                case 3
                    r = Ice.Instrumentation.ThreadState.ThreadStateInUseForOther;
                otherwise
                    throw(Ice.MarshalException('', '', sprintf('enumerator value %d is out of range', v)));
            end
        end
    end
end
