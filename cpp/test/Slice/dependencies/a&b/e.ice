#include "../slices/a.ice"
#include "f.ice"

module Test
{
    interface E
    {
        void op(A a, F f);
    }
}
