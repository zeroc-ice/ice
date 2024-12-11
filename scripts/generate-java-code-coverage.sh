#!/bin/bash

JACOCO_VERSION=0.8.12

if [ "$(basename "$(pwd)")" != "java" ]; then
    echo "This script must be run from the 'java' directory."
    exit 1
fi

make -C ../cpp srcs
make

if [ ! -f org.jacoco.cli-nodeps.jar ]; then
    wget https://repo1.maven.org/maven2/org/jacoco/org.jacoco.cli/$JACOCO_VERSION/org.jacoco.cli-$JACOCO_VERSION-nodeps.jar -O org.jacoco.cli-nodeps.jar
fi

if [ ! -f org.jacoco.agent-runtime.jar ]; then
    wget https://repo1.maven.org/maven2/org/jacoco/org.jacoco.agent/$JACOCO_VERSION/org.jacoco.agent-$JACOCO_VERSION-runtime.jar -O org.jacoco.agent-runtime.jar
fi

python3 allTests.py --all --workers=4 --debug --jacoco $(pwd)/org.jacoco.agent-runtime.jar

# We don't want any coverage for generated code. We remove their class files and any inner classes.
jacocoArgs=()
for dir in src/*; do
    # Skip non-directories.
    [ -d "$dir" ] || continue

    # Skip IceGridGUI
    [ "$(basename "$dir")" == "IceGridGUI" ] && continue

    jacocoArgs+=(--classfiles "$dir/build/classes/java/main")
    jacocoArgs+=(--sourcefiles "$dir/src/main/java")

    # Iterate over the generated Java files and remove the associated class and inner class files.
    IFS=$'\n'; set -f
    for javaFile in $(find "$dir" -name '*.java' | grep generated-src); do
        # Remove everything up to generated-src/ from the path.
        relativeJavaRoot=${javaFile#*generated-src/}
        classDir="$dir/build/classes/java/main"
        classFile=${classDir}/${relativeJavaRoot%.java}.class
        # Remove the class file (a/b/Foo.class)
        rm "$classFile"
        # Remove any inner classes (a/b/Foo$*.class)
        find "$(dirname "$classFile")" -name "$(basename -- "$classFile" ".class")\$*.class" -exec rm {} \;

    done
    unset IFS; set +f
done

IFS=$'\n'; set -f
# shellcheck disable=SC2207
coverageFiles=($(find . -name \*.jacoco.exec))
unset IFS; set +f

java -jar org.jacoco.cli-nodeps.jar report "${coverageFiles[@]}" "${jacocoArgs[@]}" --html coverage
