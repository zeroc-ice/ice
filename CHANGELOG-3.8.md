The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

We recommend that you use the release notes as a guide for migrating your applications to this release, and the manual
for complete details on a particular aspect of Ice.

# Changes in Ice 3.8.0

These are the changes since the Ice 3.7.10 release in [CHANGELOG-3.7.md](./CHANGELOG-3.7.md).

## Slice Language Changes

- Removed local Slice. `local` is no longer a Slice keyword.

- Slice classes can no longer define operations or implement interfaces, and `implements` is no longer a Slice keyword.
This feature has been deprecated since Ice 3.7.

- Slice classes no longer represent remote Ice objects; the syntax `MyClass*` (a proxy to a class) is now invalid.

- An interface can no longer be used as a type This feature, known as "interface by value", has been deprecated since
Ice 3.7. You can still define proxies with the usual syntax, `Greeter*`, where `Greeter` represents an interface.

## C++ Changes

- The C++98 mapping is now called the Original mapping.

- The C++11 mapping is now called the New mapping.

- (Original mapping) The base class for mapped class instances is now Ice::Value, like in the new mapping. Previously,
Ice::Object was the base class for both mapped class instances and servants.

- (Original mapping) Ice::Value does not derive from IceUtil::Shared and the generated Ptr for mapped classed is now an
Ice::SharedPtr that behaves mostly like the previous IceUtil (and IceInternal) Handle by wrapping a std::shared_ptr.
The important differences are:
 - the comparison operators of Ice::SharedPtr compare pointers like std::shared_ptr but unlike IceUtil::Handle.
 - the pointed-to object no longer holds the reference count, and as result you must be careful and avoid creating
 multiple SharedPtr managing the same object. For example:
 ```
 MyClassPtr c1 = new MyClass(); // SharedPtr to class instance
 MyClassPtr c2 = c1; // c1 and c2 point to the same instance
 MyClassPtr c3 = c1.get(); // c3 points to the same instance. With Ice 3.7 and before, it's ok as it simply adds a
                           // a reference count to the shared instance. As of Ice 3.8, it's incorrect since c3 is a new
                           // independent SharedPtr with its own reference count.
 ```

- (Original mapping) Removed all support for garbage collection (GC) of class instances. If you create or receive a
graph of class instances with a cycle, you must break this cycle to avoid a leak.

- (New mapping) Ice::optional is now an alias for std::optional.

- (Original mapping) IceUtil::Optional is now an alias for std::optional. When upgrading from Ice 3.7 or earlier, you
need to replace calls to `get()` on IceUtil::Optional by calls to `value()`.

## Objective-C Changes

- The base class for class instances is now Ice::Value. Previously, Ice::Object was the base class for both mapped class
instances and servants.

- The slice compiler no longer generates an Objective-C protocol for Slice classes. It generates only an Objective-C
class (interface).

## PHP Changes

- Removed the flattened mapping deprecated in 3.7.

## Ice Service Changes

- The implementations of Glacier2, IceGrid, IceStorm and IcePatch2 were updated to use the new C++ mapping.
