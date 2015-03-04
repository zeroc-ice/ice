# ProGuard configuration options

-dontnote

-keep class HelloApplet

# Preserve all annotations.
-keepattributes *Annotation*

# Preserve all native method names and the names of their classes.
-keepclasseswithmembernames class * {
    native <methods>;
}

# Preserve a method that is required in all enumeration classes.
-keepclassmembers class * extends java.lang.Enum {
    public **[] values();
}

-dontskipnonpubliclibraryclasses
-dontusemixedcaseclassnames

# We could reduce the size of the JAR file significantly if we
# enable obfuscation but it would make things like stack traces
# much harder to read.
-dontobfuscate
-dontoptimize

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

# This class is loaded dynamically.
-keep public class IceSSL.PluginFactory {
  public *;
}

# More keeps to suppress Notes

-keep interface Ice.Communicator

-keep public class Ice.DispatchStatus {
    int __value;
    Ice.DispatchStatus[] __values;
    int value();
}

-keep public class java.net.DatagramSocketImpl {
    java.io.FileDescriptor fd;
}
