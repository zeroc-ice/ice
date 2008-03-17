This demo shows how to transform a Freeze map when its definition
changes.

In this demo, we start with a simple "contacts" map:

  key = string
  value = struct ContactData
          {
             string phoneNumber;
          };

Later on, we want to add a new data member to ContactData:

//
// New version of ContactData defined in NewContactData.ice
//
struct ContactData
{
    string phoneNumber;
    string emailAddress;
};

If the contacts map was not sorted on its key, the transformation
would be a straightforward 'FreezeScript' transformation:

$ transformdb --old ContactData.ice --new NewContactData.ice db dbnew

(with -f <xml-file> if you want a customized transformation)

With a sorted map, the situation is more complicated. transformdb
is unaware of the sort order, and as a result transforms the map into
an unsorted map -- or more precisely, a new map sorted using Berkeley
DB's default sort order (comparison of binary strings that correspond
to data encoded using the Ice encoding).

The solution, illustrated in this demo, is to recreate the map after
the successful transformdb transformation.

This demo provides four programs:

 - create

   Creates or updates the contacts map in the 'db' directory (uses the
   old version of ContactData)

 - read

   Reads and displays the contents of the contacts map in the 'db'
   directory (uses the old version of ContactData)

 - readnew

   Reads and displays the contents of the contacts map in the 'dbnew'
   directory (uses the new version of ContactData)

 - recreate

   Recreate the contacts map in the 'dbnew' directory (uses the new
   version of ContactData)


Running the demo
----------------

 - first create the original map, by running create:

   $ create

 - display the entries created by create:

   $ read

   Please notice the alphabetical sort order.

 - transform the contacts database to the new format:

   $  transformdb --old ContactData.ice --new NewContactData.ice \
       -f transform.xml db dbnew

 - read the transformed contacts map:

   $ readnew

   You'll notice the sort order (looks random), and the corrupt index
   The index is actually created by readnew when it opens contacts;
   the mismatch between the expected sort order (alphabetical) and
   actual sort order leads to this corruption. If you prefer, you can
   update readnew with

      bool createDb = false

   With this update, readnew does not create the index and will fail
   to open the contacts map when the associated phoneNumber index does
   not exist.

 - recreate the new contacts map with:

   $ recreate

   recreate reads and rewrites the contacts map with the proper sort
   order; it also removes and recreates the phoneNumber index. You can
   run 'recreate --Freeze.Trace.Map=2' to get more information on the
   actions performed by recreate.

 - read again the new contacts map:

   $ readnew

   This time, the sort order should be alphabetical again, and readnew
   is expected to complete successfully.
