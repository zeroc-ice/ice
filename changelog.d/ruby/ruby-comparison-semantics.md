- The comparison methods (`<=>`, `==`, `eql?`) of Ice types in Ruby (Slice-generated enums, `Ice::Identity`,
  `Ice::ObjectPrx`, and `Ice::Endpoint`) now follow standard Ruby semantics for an operand of a different type: `==`
  and `eql?` return `false` and `<=>` returns `nil`, instead of raising an exception. On enums, which are
  `Comparable`, ordered comparisons such as `<` still raise `ArgumentError`.
