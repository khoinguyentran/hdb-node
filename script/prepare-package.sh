PACKAGE_NAME=$1
PACKAGE_VERSION=$2
PACKAGE_OS=$3
PACKAGE_DIR=$4

if [ -z "$PACKAGE_NAME" ]; then
    echo "[EE] Package name missing."
    exit 1
fi

if [ -z "$PACKAGE_VERSION" ]; then
    echo "[EE] Package version missing."
    exit 1
fi

if [ -z "$PACKAGE_OS" ]; then
    echo "[EE] Package OS missing."
    exit 1
fi

if [ -z "$PACKAGE_DIR" ]; then
    echo "[EE] Package directory missing."
    exit 1
fi

echo
echo "Preparing software package:"
echo "----------------------------------------------------------------------"
echo "  Name: $PACKAGE_NAME"
echo "  Version: $PACKAGE_VERSION"
echo "  OS: $PACKAGE_OS"
echo "  Directory: $PACKAGE_DIR"
echo "----------------------------------------------------------------------"
echo "-- Writing pkg-descr"
PKG_DESCR="$PACKAGE_DIR/pkg-descr"
> "$PKG_DESCR"
echo "Name: $PACKAGE_NAME" >> "$PKG_DESCR"
echo "Version: $PACKAGE_VERSION" >> "$PKG_DESCR"
echo "OS: $PACKAGE_OS" >> "$PKG_DESCR"

echo "-- Writing pkg-plist"
PKG_PLIST="$PACKAGE_DIR/pkg-plist"
rm -rf "$PKG_PLIST"
cd "$PACKAGE_DIR"
FILES=`find -s . -type f`

# Calculate checkums
for f in $FILES
do
    CSUM_OUTPUT=`shasum -a 256 "$f"`
    echo "$CSUM_OUTPUT" >> "$PKG_PLIST"
done

# Record name of executable files
for f in $FILES
do
    if [ -x "$f" ]; then
        echo "executable $f" >> "$PKG_PLIST"
    fi
done


echo "----------------------------------------------------------------------"
echo "Finished preparing software package."


