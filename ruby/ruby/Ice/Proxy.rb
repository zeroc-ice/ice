# Copyright (c) ZeroC, Inc.

module Ice

    T_ObjectPrx = Ice.__declareProxy('::Ice::Object')
    T_ObjectPrx.defineProxy(ObjectPrx, nil, [])

    # Provide some common functionality for proxy classes
    module Proxy_mixin
        module ClassMethods
            def ice_staticId()
                self::ICE_ID
            end

            def checkedCast(proxy, facet: nil, context: nil)
                if proxy.nil?
                    return nil
                end

                unless facet.nil?
                    proxy = proxy.ice_facet(facet)
                end

                if proxy.ice_isA(self::ICE_ID, context)
                    return self::create(proxy)
                else
                    return nil
                end
            end

            def uncheckedCast(proxy, facet: nil)
                if proxy.nil?
                    return nil
                end

                unless facet.nil?
                    proxy = proxy.ice_facet(facet)
                end

                self::create(proxy)
            end
        end

      def self.included(base)
          base.extend(ClassMethods)
      end
    end

    class ObjectPrx
        include Proxy_mixin
    end

    # Proxy comparison functions.

    def Ice.proxyIdentityCompare(lhs, rhs)
      if (lhs && !lhs.is_a?(ObjectPrx)) || (rhs && !rhs.is_a?(ObjectPrx))
          raise TypeError, 'argument is not a proxy'
      end
      if lhs.nil? && rhs.nil?
          return 0
      elsif lhs.nil? && rhs
          return -1
      elsif lhs && rhs.nil?
          return 1
      else
          return lhs.ice_getIdentity() <=> rhs.ice_getIdentity()
      end
    end

    def Ice.proxyIdentityEqual(lhs, rhs)
        return proxyIdentityCompare(lhs, rhs) == 0
    end

    def Ice.proxyIdentityAndFacetCompare(lhs, rhs)
        n = proxyIdentityCompare(lhs, rhs)
        if n == 0 && lhs && rhs
            n = lhs.ice_getFacet() <=> rhs.ice_getFacet()
        end
        return n
    end

    def Ice.proxyIdentityAndFacetEqual(lhs, rhs)
        return proxyIdentityAndFacetCompare(lhs, rhs) == 0
    end
end
