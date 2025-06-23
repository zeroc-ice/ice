classdef (Sealed) CloseConnectionException < Ice.ProtocolException
    %CLOSECONNECTIONEXCEPTION This exception indicates that the connection has been gracefully shut down by the
    %   server. The operation call that caused this exception has not been executed by the server. In most cases you
    %   will not get this exception, because the client will automatically retry the operation call in case the server
    %   shut down the connection. However, if upon retry the server shuts down the connection again, and the retry
    %   limit has been reached, then this exception is propagated to the application code.

    % Copyright (c) ZeroC, Inc.
end
