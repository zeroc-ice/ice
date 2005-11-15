#
# This ProGuard configuration file illustrates how to process applications.
# Usage:
#     java -jar proguard.jar @applications.pro
#

# Specify the input jars, output jars, and library jars.

# Preserve all public applications.

-libraryjars <java.home>/lib/rt.jar

-keepclasseswithmembers public class * {
    public static void main(java.lang.String[]);
}

# Print out a list of what we're preserving.

-printseeds

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

-dontusemixedcaseclassnames
-dontoptimize

-keepclassmembers class * implements java.io.Serializable {
    static final long serialVersionUID;
    private void writeObject(java.io.ObjectOutputStream);
    private void readObject(java.io.ObjectInputStream);
    java.lang.Object writeReplace();
    java.lang.Object readResolve();
}

# Your application may contain more items that need to be preserved; 
# typically classes that are dynamically created using Class.forName:

-keep class IceGrid.**
-keep interface IceGrid.**
-keep class com.jgoodies.looks.plastic.PlasticXPLookAndFeel
-keep class javax.**
-keep interface javax.**
-keep class * implements javax.**
