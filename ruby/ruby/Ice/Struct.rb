# Copyright (c) ZeroC, Inc.

module Ice

  # Provide some common functionality for structs
  module Inspect_mixin
      def inspect
          ::Ice::__stringify(self, self.class::ICE_TYPE)
      end
      alias to_s inspect
  end
end
