classdef Logger < IceInternal.WrapperObject
    % Logger   Summary of Logger
    %
    % The Ice message logger.
    %
    % Logger Methods:
    %   print - Print a message.
    %   trace - Log a trace message.
    %   warning - Log a warning message.
    %   error - Log an error message.
    %   getPrefix - Return this logger's prefix.
    %   cloneWithPrefix - Returns a clone of the logger with a new prefix.

    % Copyright (c) ZeroC, Inc.

    methods (Hidden, Access = ?Ice.Communicator)
        function obj = Logger(impl)
            assert(isa(impl, 'lib.pointer'));
            obj@IceInternal.WrapperObject(impl);
        end
    end
    methods
        function print(obj, message)
            % print - Print a message. The message is printed literally, without
            % any decorations such as executable name or time stamp.
            %
            % Parameters:
            %   message (char) - The message to log.

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('print', message);
        end
        function trace(obj, category, message)
            % trace - Log a trace message.
            %
            % Parameters:
            %   category (char) - The trace category.
            %   message (char) - The trace message to log.

            arguments
                obj (1, 1) Ice.Logger
                category (1, :) char
                message (1, :) char
            end
            obj.iceCall('trace', category, message);
        end
        function warning(obj, message)
            % warning - Log a warning message.
            %
            % Parameters:
            %   message - The warning message to log.

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('warning', message);
        end
        function error(obj, message)
            % error - Log an error message.
            %
            % Parameters:
            %   message - The error message to log.

            arguments
                obj (1, 1) Ice.Logger
                message (1, :) char
            end
            obj.iceCall('error', message);
        end
        function r = getPrefix(obj)
            % getPrefix - Returns this logger's prefix.
            %
            % Returns (char) - The prefix.

            arguments
                obj (1, 1) Ice.Logger
            end
            r = obj.iceCallWithResult('getPrefix');
        end
        function r = cloneWithPrefix(obj, prefix)
            % cloneWithPrefix - Returns a clone of the logger with a new prefix.
            %
            % Parameters:
            %   prefix - The new prefix for the logger.
            %
            % Returns (Ice.Logger) - A logger instance.

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
