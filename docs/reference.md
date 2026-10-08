# API reference

`xyo-coff-to-def` is an executable; it installs no library or headers. The
classes below are what its sources (and `test/test.01.cpp`) use.

## Headers

| Header | Contents |
|--------|----------|
| `<XYO/CoffToDef/Coff.hpp>` | `namespace XYO::CoffToDef::Coff`: COFF reader, DEF writer |
| `<XYO/CoffToDef/Application.hpp>` | `XYO::CoffToDef::Application`, the command line tool class |
| `<XYO/CoffToDef/Dependency.hpp>` | `<XYO/System.hpp>`, `namespace XYO::CoffToDef { using namespace XYO::System; }` |
| `<XYO/CoffToDef/Copyright.hpp>`, `License.hpp`, `Version.hpp` | tool metadata |

## Macros

| Macro | Meaning |
|-------|---------|
| `XYO_COFFTODEF_LIBRARY` | build `Application.cpp` without `main` (`XYO_APPLICATION_MAIN`); set by the `test.01` project |
| `XYO_COFFTODEF_VERSION_*` (`Version.rh`) | version for the Windows resource, generated from `Version.Template.rh` by xyo-version |
| `XYO_COFFTODEF_COPYRIGHT`, `_PUBLISHER`, `_COMPANY`, `_CONTACT` (`Copyright.rh`) | copyright strings |

## namespace XYO::CoffToDef::Coff

```cpp
enum {
	ModeAuto = 0,  // ModeWin32 for i386 objects, ModeWin64 for the others
	ModeWin32 = 1, // "_name" -> "name" when the name has no "@"
	ModeWin64 = 2  // names as they are
};

enum {
	MachineI386 = 0x014C,
	MachineAMD64 = 0x8664,
	MachineARM64 = 0xAA64
};

// DEF export name -> is data (written with DATA)
typedef TRedBlackTree<String, bool> SymbolList;

bool isFiltered(const String &name);
String exportName(const String &name, int mode);

bool getSymbols(const uint8_t *data, size_t size, int mode, SymbolList &symbols, bool show, String &error);
bool getSymbolsFromFile(const char *fileName, int mode, SymbolList &symbols, bool show, String &error);

String generateDef(SymbolList &symbols);
bool generateDefFile(const char *fileName, SymbolList &symbols);
```

| Function | Does |
|----------|------|
| `isFiltered` | `true` for the compiler generated names of [Filtered symbols](def-file.md#filtered-symbols), checked as given and without a first `_` |
| `exportName` | applies the [naming rules](def-file.md#naming-rules) of `ModeWin32` or `ModeWin64` to one name |
| `getSymbols` | parses a COFF object (regular or `/bigobj`) in memory and adds its defined external symbols to `symbols`, filtered and renamed; `ModeAuto` resolves by the object's machine; `show` prints every defined external symbol; returns `false` with `error` set for anything that is not a supported, well formed object; never reads outside `data[0..size)` |
| `getSymbolsFromFile` | reads the file (`Shell::fileGetContents`), prints `Coff Symbols: <file>` when `show`, then `getSymbols`; `error` is `"unable to read file"` when it can not be read |
| `generateDef` | the DEF text: `LIBRARY`, `EXPORTS`, one name per line (`name DATA` for data), `\r\n` |
| `generateDefFile` | writes `generateDef` to `fileName` (`Shell::filePutContents`) |

`symbols` is ordered (red black tree), so the output is sorted and each
name appears once; a later `set` of the same name replaces its data flag.

Example:

```cpp
#include <XYO/CoffToDef/Coff.hpp>

using namespace XYO::CoffToDef;

Coff::SymbolList symbols;
String error;
if (!Coff::getSymbolsFromFile("a.obj", Coff::ModeAuto, symbols, false, error)) {
	printf("Error: a.obj: %s\n", error.value());
	return 1;
};
Coff::generateDefFile("a.def", symbols);
```

## class XYO::CoffToDef::Application

```cpp
class Application : public virtual IApplication {
	public:
		void showUsage();
		void showLicense();
		void showVersion();
		int main(int cmdN, char *cmdS[]);
		static void initMemory();
		bool readResponseFile(const String &fileName, TDynamicArray<String> &fileList, int level);
};
```

| Member | Does |
|--------|------|
| `main` | the `xyo-coff-to-def` command: parses `cmdS[1..cmdN-1]`, reads every object, writes the DEF file only if all succeeded, returns the exit code; prints to `stdout`, never calls `exit` |
| `readResponseFile` | appends the names of a response file to `fileList` (trimmed, unquoted, `//` comments and empty lines skipped, nested `@file`); `level` is the nesting depth, fails at 16; prints an error and returns `false` on failure |
| `showUsage` | prints name, version, copyright and the usage |
| `showLicense` | prints the MIT license |
| `showVersion` | prints `version X build Y [date time]` |
| `initMemory` | initializes the managed memory of `String`, `Error`, `Buffer`, `TDynamicArray<String>` and `Coff::SymbolList` |

Not copyable, not movable. See [Command line](command-line.md).

## namespace XYO::CoffToDef::Version / Copyright / License

```cpp
const char *XYO::CoffToDef::Version::version();
const char *XYO::CoffToDef::Version::build();
const char *XYO::CoffToDef::Version::versionWithBuild();
const char *XYO::CoffToDef::Version::datetime();

const char *XYO::CoffToDef::Copyright::copyright();
const char *XYO::CoffToDef::Copyright::publisher();
const char *XYO::CoffToDef::Copyright::company();
const char *XYO::CoffToDef::Copyright::contact();

std::string XYO::CoffToDef::License::license();
std::string XYO::CoffToDef::License::shortLicense();
```

## fabricare projects

| Project | Kind | Notes |
|---------|------|-------|
| `xyo-coff-to-def` | executable | the `xyo-coff-to-def` command, sources `source/XYO/CoffToDef` |
| `test.01` | test executable (`category: test`) | `test/test.01.cpp` plus the tool sources with `XYO_COFFTODEF_LIBRARY`; run by `fabricare test` in `output/test` |

Both depend on `xyo-system`.
