if which swiftformat >/dev/null; then
    swiftformat --version 4.2 $1
else
    echo "warning: SwiftFormat not installed, download from https://github.com/nicklockwood/SwiftFormat"
fi

if which swiftlint >/dev/null; then
    swiftlint --path $1 --config $2
else
    echo "warning: SwiftLint not installed, download from https://github.com/realm/SwiftLint"
fi
