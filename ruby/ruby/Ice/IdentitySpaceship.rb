# Copyright (c) ZeroC, Inc.

require_relative 'Identity.rb'

module Ice
    class Identity
        def <=>(other)
            return nil unless other.is_a?(Identity)
            n = self.name <=> other.name
            if n == 0
                return self.category <=> other.category
            else
                return n
            end
        end
    end
end
