struct x
{
    int i;
    x j;	// Error
};

// TODO: The following should cause an error, but doesn't. Need to add a check for that.
#if 0
class y
{
    int i;
    y j;	// Error
};
#endif
