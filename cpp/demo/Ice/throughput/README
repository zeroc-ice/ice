A simple throughput demo that allows you to send sequences of various
types between client and server and to measure the maximum bandwidth
that can be achieved using serialized synchronous requests.

To run the demo, first start the server:

$ server

In a separate window, start the client:

$ client

We provide a number of optimizations through metadata in this demo.

Bytes sequences
--------------

For in parameters we use the "cpp:array" metadata which on the server 
side means that the operation implementation is passed pointers into 
the marshaling buffer which eliminates a copy of the sequence data.

For return parameters we also use the "cpp:array" metadata, but this
time in conjunction with AMD. Doing this also reduces the number of
times the returned sequence is copied during marshaling.

Strings
-------

We use a custom mapping for strings: instead of the default std::string,
we use our own lightweight Util::string_view type, which does not 
allocate memory.
 
string_view is similar to boost::string_ref and to the proposed
experimental C++14 string_view.

The resulting behavior is similar to the "cpp:array" behavior for bytes 
sequences, with the string_view "pointing" to the Ice marshaling buffer. 

Like the cpp:array metadata, the cpp:view-type metadata changes the
mapped type only when it's safe to reference memory. In particular,
cpp:array and cpp:view-type have no effect for regular returned 
parameters and out parameters, such as:  

// Slice
["cpp:view-type:std::vector<Util::string_view>"] StringSeq recvStringSeq();

//
// C++ 
// Mapped to the default std::vector<string>, and not std::vector<Util::string_view>
//
stringVect = prx->recvStringSeq();

If you use cpp:type above instead of cpp:view-type, you would get a
std::vector<Util::string_view>, with the string_view objects pointing to 
deallocated memory:

// Slice - you should _not_ use cpp:type here!
["cpp:type:std::vector<Util::string_view>"] StringSeq recvStringSeq();

//
// C++ 
// Mapped to std::vector<Util::string_view>
//
stringViewVect = prx->recvStringSeq(); // very dangerous, as the string_views
                                       // point to deallocated memory 
