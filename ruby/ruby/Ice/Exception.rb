# Copyright (c) ZeroC, Inc.

module Ice

    class Exception < ::StandardError
    end

    class LocalException < Exception
    end

    class UserException < Exception
        def ice_id
            to_s
        end

        def inspect
            return ::Ice::__stringifyException(self)
        end
    end
end
