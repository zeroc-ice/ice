- Fixed `Ice::Endpoint` comparison in Ice for Ruby. `<=>` compared an endpoint with itself, making `==`/`<=>`
  asymmetric, and the class defined `eql?` without a matching `hash`; equal endpoints now compare consistently and
  can be used as `Hash`/`Set` keys.
