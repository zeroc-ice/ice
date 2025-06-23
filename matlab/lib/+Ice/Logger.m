classdef Logger < IceInternal.WrapperObject
    %LOGGER Represents Ice's abstraction for logging and tracing.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = ?Ice.Communicator)
        function obj = Logger(impl)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        function print(obj, message)
            %PRINT Prints a message. The message is printed literally, without any decorations such as executable name
            %   or time stamp.
            %
            %   Input Arguments
            %     message - The message to log.
            %       character vector

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('print', message);
        end

        function trace(obj, category, message)
            %TRACE Logs a trace message.
            %
            %   Input Arguments
            %     category - The trace category.
            %       character vector
            %     message - The trace message to log.
            %       character vector

            arguments
                obj (1, 1) Ice.Logger
                category (1, :) char
                message (1, :) char
            end
            obj.iceCall('trace', category, message);
        end

        function warning(obj, message)
            %WARNING Logs a warning message.
            %
            %   Input Arguments
            %     message - The warning message to log.
            %       character vector

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('warning', message);
        end

        function error(obj, message)
            %ERROR Logs an error message.
            %
            %   Input Arguments
            %     message - The error message to log.
            %       character vector

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('error', message);
        end

        function r = getPrefix(obj)
            %GETPREFIX Returns this logger's prefix.
            %
            %   Output Arguments
            %     r - The prefix.
            %       character vector

            arguments
                obj (1, 1) Ice.Logger
            end
            r = obj.iceCallWithResult('getPrefix');
        end

        function r = cloneWithPrefix(obj, prefix)
            %CLONEWITHPREFIX Returns a clone of the logger with a new prefix.
            %
            %   Input Arguments
            %     prefix - The new prefix for the logger.
            %       character vector
            %
            %   Output Arguments
            %     r - A new logger instance.
            %       Ice.Logger scalar

            arguments
                obj (1, 1) Ice.Logger
                prefix (1, :) char
            end
            impl = libpointer('voidPtr');
            obj.iceCall('cloneWithPrefix', prefix, impl);
            if isNull(impl)
                r = obj;
            else
                r = Ice.Logger(impl);
            end
        end
    end
end
