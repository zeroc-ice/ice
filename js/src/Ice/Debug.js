
export class Debug
{
    static assert(condition, message)
    {
        if (!condition)
        {
            console.trace("Assertion failed: " + message);
            if (typeof process !== "undefined")
            {
                process.exit(1);
            }
        }
    }
}
