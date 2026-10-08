# Getting started

## 1. Build and install

The tool is built with [fabricare](https://github.com/g-stefan/fabricare).
`xyo-platform`, `xyo-managed-memory`, `xyo-data-structures`,
`xyo-multithreading`, `xyo-encoding` and `xyo-system` must be installed to
the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # build and run test/test.01.cpp
fabricare install    # copy output/bin to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

After `install`, `xyo-coff-to-def` is in `~/.fabricare/<platform>/bin`,
which is on the `PATH` of a fabricare build, so the scripts of other
projects (and `lib-to-dll`) can call it.

The tool is Windows oriented (DEF files and COFF objects are Microsoft
formats) but the reader is portable C++: it builds and its tests pass on
Linux too, which is useful for cross builds.

## 2. Compile the objects

Any objects from `cl.exe` or `clang-cl` work, for x86, x64 or ARM64, with
or without `/bigobj`:

```
cl /c /O2 /MD /EHsc adler32.c compress.c deflate.c
```

Do **not** compile with `/GL` (whole program optimization): those objects
hold compiler intermediate code, not COFF symbols, and are rejected.

## 3. First DEF file

```
xyo-coff-to-def --out zlib.def adler32.obj compress.obj deflate.obj
```

Exit code `0` and a `zlib.def`:

```
LIBRARY
EXPORTS
adler32
compress
deflate
...
```

Every external symbol defined by the objects, sorted, one per line;
variables carry `DATA`. Compiler internal symbols are left out. The rules
are in [DEF file](def-file.md).

To see what the objects define before writing the file:

```
xyo-coff-to-def --show --out zlib.def adler32.obj
```

## 4. Link the DLL

```
link /DLL /DEF:zlib.def /OUT:zlib.dll /IMPLIB:zlib.lib adler32.obj compress.obj deflate.obj
```

`zlib.dll` exports the listed symbols; `zlib.lib` is the import library to
link the programs that use it. Code that reads an exported variable must
declare it `__declspec(dllimport)` (that is what `DATA` means: the import
library has only `__imp_name` for it).

## 5. From a static library

`lib-to-dll` does the whole job for a `.lib`:

```
lib-to-dll --mode WIN64 zlib.lib
```

It extracts the objects (`lib /extract`), runs
`xyo-coff-to-def --out zlib.dll.def --mode WIN64 ...` on them and links
`zlib.dll`. It does that when there is no hand written `zlib.def` next to
the library, or always with `--use-coff-def`. By hand:

```
lib /nologo /list zlib.lib > objects.txt
for /f %i in (objects.txt) do lib /nologo /extract:%i /out:%~nxi zlib.lib
xyo-coff-to-def --out zlib.def adler32.obj compress.obj ...
```

`xyo-coff-to-def` reads object files only; passing the `.lib` itself is an
error ("is a library (.lib), extract the objects first").

## 6. From a fabricare script

```js
runInPath("temp", function() {
	exitIf(Shell.system("xyo-coff-to-def --out my-library.def @my-library.objects.txt"));
});
```

`exitIf` stops the build on a non zero exit code, which is how every error
of the tool is reported.

## 7. Conventions

- **Exit code `1` on any error** (unreadable file, not a COFF object,
  unsupported object, bad option), with an `Error: ...` line on standard
  output; **no `.def` is written** then, an old one is left as it was.
- **Paths are used as given**, relative to the current directory, also the
  ones inside a response file.
- **The output is overwritten**; its folder must exist.
- **Two dashes for every option**: `--out`. Anything else is a file name.
