A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

For the bytes sequence operations in this demo we provide some
optimizations. For in parameters we use the "cpp:array" metadata
which on the server side means that the operation implementation is
passed pointers into the marshaling buffer which eliminates a copy
of the sequence data.

For return parameters we also use the "cpp:array" metadata, but this
time in conjunction with AMD. Doing this also reduces the number of
times the returned sequence is copied during marshaling.

The performance for byte sequences is expected to be greater than
for other types because the cost of marshaling and unmarshaling is
lower than for more complex types. With byte sequences there is no
need to allocate and deallocate objects, which adds overhead. The C++
mapping for byte sequences also uses a zero-copy optimization which is
not possible with other types.
