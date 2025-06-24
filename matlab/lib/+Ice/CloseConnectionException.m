classdef (Sealed) CloseConnectionException < Ice.ProtocolException
    %CLOSECONNECTIONEXCEPTION The exception that is thrown when the connection has been gracefully shut down by the
    %   server. The request that returned this exception has not been executed by the server. In most cases you will
    %   not get this exception, because the client will automatically retry the invocation. However, if upon retry the
    %   server shuts down the connection again, and the retry limit has been reached, then this exception is
    %   propagated to the application code.

    % Copyright (c) ZeroC, Inc.
end
