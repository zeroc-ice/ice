A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

The Ice extension for Ruby currently supports only client-side
functionality, therefore you must use a server from any other language
mapping.

After starting the server, run the Ruby client:

$ ruby Client.rb

The performance for byte sequences is expected to be greater than 
for other types because the cost of marshaling and unmarshaling is
lower than for more complex types. With byte sequences there is no
need to allocate and deallocate objects, which adds overhead.
