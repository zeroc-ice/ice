#include "a.ice"
#include "b.ice"

module Test
{
    interface C
    {
        void op(A a, B b);
    }
}
