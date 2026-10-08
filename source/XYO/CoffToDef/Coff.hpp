// Coff To Def
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_COFFTODEF_COFF_HPP
#define XYO_COFFTODEF_COFF_HPP

#ifndef XYO_COFFTODEF_DEPENDENCY_HPP
#	include <XYO/CoffToDef/Dependency.hpp>
#endif

namespace XYO::CoffToDef::Coff {

	// How symbol names are turned into DEF export names
	enum {
		// Win32 rules for i386 objects, Win64 rules for the others
		ModeAuto = 0,
		// i386 rules: "_name" (cdecl) is exported as "name", other names as they are
		ModeWin32 = 1,
		// x64 / ARM64 rules: names are exported as they are
		ModeWin64 = 2
	};

	enum {
		MachineI386 = 0x014C,
		MachineAMD64 = 0x8664,
		MachineARM64 = 0xAA64
	};

	// DEF export name -> is data (exported with the DATA keyword)
	typedef TRedBlackTree<String, bool> SymbolList;

	// Compiler generated symbols that must not be exported (constants, RTTI, vftables, throw info, ...)
	bool isFiltered(const String &name);

	// DEF export name of a symbol, mode is ModeWin32 or ModeWin64
	String exportName(const String &name, int mode);

	// Add the external symbols defined in a COFF object in memory to symbols
	// show: print every external symbol defined
	// On failure returns false and sets error
	bool getSymbols(const uint8_t *data, size_t size, int mode, SymbolList &symbols, bool show, String &error);

	bool getSymbolsFromFile(const char *fileName, int mode, SymbolList &symbols, bool show, String &error);

	String generateDef(SymbolList &symbols);
	bool generateDefFile(const char *fileName, SymbolList &symbols);

};

#endif
