A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

For the bytes sequence operations in this demo we provide some 
optimizations. For in paramaters we use the "cpp:array" metadata
which on the server side means that the operation implementation is
passed pointers into the marshalling buffer which eliminates a copy
of the sequence data. 

For return aramaters we also use the cpp:array" metadata, but this
time in conjuction with AMD. Doing this also reduces the number of 
times the returned sequence is copied during marshalling.

