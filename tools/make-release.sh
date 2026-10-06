#!/bin/bash
# Builds the release packages on Haiku, from a clean copy of this tree:
#   tools/make-release.sh <version> [<revision>]
# x86_64:           tools/make-release.sh 0.3.0
# x86_gcc2 hybrid:  setarch x86 tools/make-release.sh 0.3.0
# Result: release/projectconceptor[_x86]{,_debuginfo}-<version>-<revision>-<arch>.hpkg
set -euo pipefail

VERSION="${1:?usage: make-release.sh <version> [<revision>]}"
REVISION="${2:-1}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"

ARCH="$(getarch)"
PRIMARY="$(getarch -p)"
if [ "$ARCH" != "$PRIMARY" ]; then
	# secondary architecture: the package is for the primary one, add-ons
	# and dependencies carry the secondary suffix (HaikuPorts convention)
	SUFFIX="_$ARCH"
	ADDONS="add-ons/$ARCH"
else
	SUFFIX=""
	ADDONS="add-ons"
fi
NAME="projectconceptor$SUFFIX"
FULL="$VERSION-$REVISION"

WORK="$HOME/release-work/$NAME-$FULL"
BUILD="$WORK/tree"
MAIN="$WORK/main"
DEBUG="$WORK/debug"
OUT="$ROOT/release"

fail() { echo "RELEASE FAILED: $1" >&2; exit 1; }

rm -rf "$WORK"
mkdir -p "$BUILD" "$MAIN" "$DEBUG/develop/debug" "$OUT"

# --- build: clean copy, catalogs bound, bound binaries copied into bin/ ---
tar -C "$ROOT" --exclude='./.git' --exclude='./bin' --exclude='*/objects*' --exclude='./release' \
	-cf - . | tar -C "$BUILD" -xf -
cd "$BUILD/src"
make > "$WORK/build.log" 2>&1 || true
make bindcatalogs >> "$WORK/build.log" 2>&1 || true
# the plugins copy their binary into bin/ on every make, the app too
make >> "$WORK/build.log" 2>&1 || true
# makefile-engine doesn't pass exit codes through reliably
if grep -qE '^\s*(make(\[[0-9]+\])?: )?\*\*\* |: error:|: fatal error:|undefined reference to|couldn.t write' "$WORK/build.log"; then
	fail "build, see $WORK/build.log"
fi

# --- stage ---
cp -a "$BUILD/bin/apps" "$MAIN/"
cp -a "$BUILD/bin/data" "$MAIN/"
mkdir -p "$MAIN/$ADDONS/Translators"
cp -a "$BUILD/bin/add-ons/Translators/." "$MAIN/$ADDONS/Translators/"

# --- split debug info ---
# order matters: objcopy --add-gnu-debuglink drops Haiku resources, so the
# resources (icons, bound catalogs) are copied back last
# (~/repos/haiku-reference-md/notes/packaging.md)
PROVIDES_MAIN=""
PROVIDES_DEBUG=""
cd "$MAIN"
while IFS= read -r -d '' file; do
	head -c 4 "$file" | grep -q 'ELF' || continue
	base="$(basename "$file")"
	debugName="$base($NAME-$FULL).debuginfo"
	orig="$WORK/orig"
	cp -a "$file" "$orig"
	objcopy --only-keep-debug "$orig" "$DEBUG/develop/debug/$debugName"
	objcopy --strip-debug --strip-unneeded "$file"
	(cd "$DEBUG/develop/debug" && objcopy --add-gnu-debuglink="$debugName" "$MAIN/$file")
	xres -o "$file" "$orig"
	resattr -O -o "$file" "$file"
	mimeset -f "$file"
	rm -f "$orig"
	PROVIDES_DEBUG="$PROVIDES_DEBUG	\"debuginfo:$base($NAME)\" = $VERSION
"
done < <(find apps "$ADDONS" -type f -print0)

PROVIDES_MAIN="	app:ProjectConceptor = $VERSION
"
for t in "$ADDONS"/Translators/*; do
	PROVIDES_MAIN="$PROVIDES_MAIN	addon:$(basename "$t") = $VERSION
"
done

# --- package info ---
cat > "$MAIN/.PackageInfo" <<EOF
name		$NAME
version		$FULL
architecture	$PRIMARY
summary		"A modular graph editor"
description	"ProjectConceptor is a modular application to view, edit and process
information that can be represented as a graph. It is still in beta, so
be aware of bugs.

Nodes carry attributes of any BMessage type and can be connected,
grouped, laid out automatically (Graphviz) and edited in a graph view
or in a navigator that shows a node's raw data. Every step can be
undone. Macros record what you do, can be edited as a tree and played
back with loops, conditions, variables and text placeholders.
Translated into German, Spanish, French, Friulian, Croatian, Romanian
and Swedish."
vendor		"Paradoxon"
packager	"Matthias Lindner <two4god@gmail.com>"
copyrights {
	"2005-2026 Paradoxon"
}
licenses {
	"MIT"
}
provides {
	$NAME = $VERSION
$PROVIDES_MAIN}
requires {
	haiku$SUFFIX
	lib:libtinyxml$SUFFIX
	cmd:dot
}
urls {
	"https://github.com/Paradoxianer/ProjectConceptor"
}
source-urls {
	"https://github.com/Paradoxianer/ProjectConceptor/archive/refs/tags/v$VERSION.tar.gz"
}
EOF

cat > "$DEBUG/.PackageInfo" <<EOF
name		${NAME}_debuginfo
version		$FULL
architecture	$PRIMARY
summary		"A modular graph editor (debug info)"
description	"Debug symbols for $NAME, split out of the shipped binaries.
Install alongside $NAME so Debugger and crash reports can resolve
function names and line numbers."
vendor		"Paradoxon"
packager	"Matthias Lindner <two4god@gmail.com>"
copyrights {
	"2005-2026 Paradoxon"
}
licenses {
	"MIT"
}
provides {
	${NAME}_debuginfo = $VERSION
$PROVIDES_DEBUG}
requires {
	$NAME == $VERSION base
}
urls {
	"https://github.com/Paradoxianer/ProjectConceptor"
}
EOF

MAIN_PKG="$OUT/$NAME-$FULL-$PRIMARY.hpkg"
DEBUG_PKG="$OUT/${NAME}_debuginfo-$FULL-$PRIMARY.hpkg"
rm -f "$MAIN_PKG" "$DEBUG_PKG"
package create -C "$MAIN" "$MAIN_PKG"
package create -C "$DEBUG" "$DEBUG_PKG"

# --- check what actually went in ---
CHECK="$WORK/check"
mkdir -p "$CHECK"
package extract -C "$CHECK" "$MAIN_PKG"
app="$CHECK/apps/ProjectConceptor/ProjectConceptor"
[ -n "$(xres -l "$app" | grep STD_ICON)" ] || fail "app icon missing"
catalogs="$(xres -l "$app" | grep -c CADA || true)"
[ "$catalogs" -gt 1 ] || fail "app catalogs missing"
for plugin in Plugins/GraphEditor Plugins/MacroEditor Plugins/NavigatorEditor \
		Plugins/LayoutEditor Plugins/Commands/Ask; do
	[ "$(xres -l "$CHECK/apps/ProjectConceptor/$plugin" | grep -c CADA || true)" -gt 1 ] \
		|| fail "$plugin catalogs missing"
done
for t in FreeMindTranslator ProjectConceptorTranslator; do
	[ -f "$CHECK/$ADDONS/Translators/$t" ] || fail "$t missing"
done

echo "app catalogs: $catalogs, plugins: $(find "$CHECK/apps/ProjectConceptor/Plugins" -type f | wc -l)"
echo "$MAIN_PKG"
echo "$DEBUG_PKG"
