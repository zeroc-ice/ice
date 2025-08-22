# Slice API Reference

**Welcome to the Slice API Reference for Ice!**

Ice is complete RPC framework that helps you build networked applications, and Slice is Ice's own
[Interface Definition Language].

Ice and Ice-based applications use the Slice language to describe remote APIs in a purely declarative manner. For
instance, here the Slice definition for a simple Greeter API:

```ice
module VisitorCenter
{
    /// Represents a simple greeter.
    interface Greeter
    {
        /// Creates a personalized greeting.
        /// @param name The name of the person to greet.
        /// @return The greeting.
        string greet(string name);
    }
}
```

The RPC in this example is the `greet` operation. You can learn more about Ice and Slice in the [Ice Manual].

#### Who is this API reference for?

This API reference was generated from the doc-comments in the shared Slice files included with Ice.

If you are using Ice to develop an application in C++, C#, Java, Python, Swift (...), you should consult the API
reference for your programming language, and this Slice API reference is not particularly relevant.

This Slice API reference is primarily for users of Ice services (such as IceGrid and IceStorm) who want to understand
the remote APIs provided by these services. This API reference shows these APIs in their purest form, without language
mapping distractions.

#### Tag file

[Slice tag file]

Use this tag file to include links to this API reference from your own Doxygen Slice API reference.

[Slice tag file]: ../slice.tag
[Ice Manual]: https://docs.zeroc.com
[Interface Definition Language]: https://en.wikipedia.org/wiki/Interface_description_language
