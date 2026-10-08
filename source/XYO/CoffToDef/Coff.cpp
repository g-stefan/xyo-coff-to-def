// Coff To Def
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>

#include <XYO/CoffToDef/Coff.hpp>

namespace XYO::CoffToDef::Coff {

	// IMAGE_FILE_HEADER
	static const size_t fileHeaderSize = 20;
	// IMAGE_SYMBOL
	static const size_t symbolSize = 18;
	// ANON_OBJECT_HEADER_BIGOBJ
	static const size_t fileHeaderBigObjSize = 56;
	// IMAGE_SYMBOL_EX
	static const size_t symbolBigObjSize = 20;

	static const uint8_t storageClassExternal = 2;
	// Type, (Type >> 4) & 3 == 2 (IMAGE_SYM_DTYPE_FUNCTION)
	static const uint16_t typeFunction = 0x20;

	// {D1BAA1C7-BAEE-4BA9-AF20-FAF66AA4DCB8}
	static const uint8_t classIdBigObj[16] = {
	    0xC7, 0xA1, 0xBA, 0xD1, 0xEE, 0xBA, 0xA9, 0x4B,
	    0xAF, 0x20, 0xFA, 0xF6, 0x6A, 0xA4, 0xDC, 0xB8};

	static inline uint16_t get16(const uint8_t *p) {
		return (uint16_t)(p[0] | (p[1] << 8));
	};

	static inline uint32_t get32(const uint8_t *p) {
		return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	};

	static bool isDigit(char ch) {
		return (ch >= '0') && (ch <= '9');
	};

	static bool isFilteredX(const String &name) {
		// constants: floating point, SSE / AVX / AVX-512 masks
		if (name.beginWith("__real@")) {
			return true;
		};
		if (name.beginWith("__xmm@")) {
			return true;
		};
		if (name.beginWith("__ymm@")) {
			return true;
		};
		if (name.beginWith("__zmm@")) {
			return true;
		};
		if (name.beginWith("__mask@@")) {
			return true;
		};
		// precompiled header marker
		if (name.beginWith("__@@_PchSym_")) {
			return true;
		};
		// vftables, RTTI, string literals, compiler generated special members
		if (name.beginWith("??_")) {
			return true;
		};
		// exception throw info: _CT??_R0..., _CTA<n>..., _TI<n>..., _TIC<n>..., _TIU<n>..., _TICU<n>...
		if (name.beginWith("_CT??")) {
			return true;
		};
		if (name.beginWith("_CTA")) {
			if (name.length() > 4) {
				if (isDigit(name[4])) {
					return true;
				};
			};
		};
		if (name.beginWith("_TI")) {
			size_t k;
			for (k = 3; k < name.length(); ++k) {
				if ((name[k] == 'C') || (name[k] == 'V') || (name[k] == 'U')) {
					continue;
				};
				break;
			};
			if (k < name.length()) {
				if (isDigit(name[k])) {
					return true;
				};
			};
		};
		return false;
	};

	bool isFiltered(const String &name) {
		if (isFilteredX(name)) {
			return true;
		};
		// i386 names have an extra "_" in front (__CT??..., __TI1...)
		if (name.length() > 1) {
			if (name[0] == '_') {
				return isFilteredX(name.substring(1));
			};
		};
		return false;
	};

	String exportName(const String &name, int mode) {
		if (mode == ModeWin32) {
			// cdecl "_name" -> "name"; stdcall "_name@n", fastcall "@name@n", C++ "?name..." stay as they are
			if (name.length() > 1) {
				if (name[0] == '_') {
					if (!name.itContains("@")) {
						return name.substring(1);
					};
				};
			};
		};
		return name;
	};

	bool getSymbols(const uint8_t *data, size_t size, int mode, SymbolList &symbols, bool show, String &error) {
		uint16_t machine;
		size_t symbolTable;
		size_t numberOfSymbols;
		size_t symbolEntrySize;
		bool isBigObj;

		if (size < 4) {
			error = "not a COFF object, file too small";
			return false;
		};
		if (size >= 8) {
			if (memcmp(data, "!<arch>\n", 8) == 0) {
				error = "is a library (.lib), extract the objects first (lib /extract)";
				return false;
			};
		};
		if ((data[0] == 'M') && (data[1] == 'Z')) {
			error = "is an executable image (.exe / .dll), not a COFF object";
			return false;
		};

		isBigObj = false;
		if ((get16(data) == 0) && (get16(data + 2) == 0xFFFF)) {
			// Anonymous object: /bigobj, /GL (LTCG), import object, ...
			if (size >= fileHeaderBigObjSize) {
				if ((get16(data + 4) >= 2) && (memcmp(data + 12, classIdBigObj, 16) == 0)) {
					isBigObj = true;
				};
			};
			if (!isBigObj) {
				error = "anonymous object (compiled with /GL, an import object or a .NET object) is not supported";
				return false;
			};
			machine = get16(data + 6);
			symbolTable = get32(data + 48);
			numberOfSymbols = get32(data + 52);
			symbolEntrySize = symbolBigObjSize;
		} else {
			if (size < fileHeaderSize) {
				error = "not a COFF object, file too small";
				return false;
			};
			machine = get16(data);
			symbolTable = get32(data + 8);
			numberOfSymbols = get32(data + 12);
			symbolEntrySize = symbolSize;
		};

		if ((machine != MachineI386) && (machine != MachineAMD64) && (machine != MachineARM64)) {
			char buffer[64];
			snprintf(buffer, sizeof(buffer), "not a COFF object or unsupported machine 0x%04X", (unsigned int)machine);
			error = buffer;
			return false;
		};

		if (mode == ModeAuto) {
			mode = (machine == MachineI386) ? ModeWin32 : ModeWin64;
		};

		if (numberOfSymbols == 0) {
			return true;
		};

		if (symbolTable > size) {
			error = "invalid COFF object, symbol table outside of file";
			return false;
		};
		if (numberOfSymbols > (size - symbolTable) / symbolEntrySize) {
			error = "invalid COFF object, symbol table outside of file";
			return false;
		};

		// The string table follows the symbol table, its first 4 bytes are its size (including them)
		const uint8_t *stringTable = data + symbolTable + numberOfSymbols * symbolEntrySize;
		size_t stringTableSize = 0;
		size_t available = size - (symbolTable + numberOfSymbols * symbolEntrySize);
		if (available >= 4) {
			stringTableSize = get32(stringTable);
			if (stringTableSize > available) {
				error = "invalid COFF object, string table outside of file";
				return false;
			};
		};

		size_t index;
		for (index = 0; index < numberOfSymbols;) {
			const uint8_t *symbol = data + symbolTable + index * symbolEntrySize;
			uint32_t value;
			int32_t sectionNumber;
			uint16_t type;
			uint8_t storageClass;
			uint8_t numberOfAuxSymbols;

			value = get32(symbol + 8);
			if (isBigObj) {
				sectionNumber = (int32_t)get32(symbol + 12);
				type = get16(symbol + 16);
				storageClass = symbol[18];
				numberOfAuxSymbols = symbol[19];
			} else {
				sectionNumber = (int16_t)get16(symbol + 12);
				type = get16(symbol + 14);
				storageClass = symbol[16];
				numberOfAuxSymbols = symbol[17];
			};
			index += 1 + (size_t)numberOfAuxSymbols;

			if (storageClass != storageClassExternal) {
				continue;
			};

			bool isData;
			if (sectionNumber == 0) {
				// undefined, value is the size of a common symbol (C tentative definition: int x;)
				if (value == 0) {
					continue;
				};
				isData = true;
			} else if (sectionNumber < 0) {
				// absolute or debug
				continue;
			} else {
				isData = ((type & 0x30) != typeFunction);
			};

			String name;
			if (get32(symbol) == 0) {
				size_t offset = get32(symbol + 4);
				if ((offset < 4) || (offset >= stringTableSize)) {
					error = "invalid COFF object, symbol name outside of string table";
					return false;
				};
				const uint8_t *nameEnd = (const uint8_t *)memchr(stringTable + offset, 0, stringTableSize - offset);
				if (nameEnd == nullptr) {
					error = "invalid COFF object, symbol name not terminated";
					return false;
				};
				name = String((const char *)(stringTable + offset), nameEnd - (stringTable + offset));
			} else {
				size_t length;
				for (length = 0; length < 8; ++length) {
					if (symbol[length] == 0) {
						break;
					};
				};
				name = String((const char *)symbol, length);
			};

			if (show) {
				printf("%s\n", name.value());
			};

			if (isFiltered(name)) {
				continue;
			};

			String export_ = exportName(name, mode);
			if (export_.length() == 0) {
				continue;
			};
			symbols.set(export_, isData);
		};

		return true;
	};

	bool getSymbolsFromFile(const char *fileName, int mode, SymbolList &symbols, bool show, String &error) {
		Buffer buffer;
		if (!Shell::fileGetContents(fileName, buffer)) {
			error = "unable to read file";
			return false;
		};
		if (show) {
			printf("Coff Symbols: %s\n\n", fileName);
		};
		return getSymbols(buffer.buffer, buffer.length, mode, symbols, show, error);
	};

	String generateDef(SymbolList &symbols) {
		String retV;
		retV << "LIBRARY\r\nEXPORTS\r\n";
		SymbolList::TNode *node;
		for (node = symbols.begin(); node; node = node->successor()) {
			retV << node->key;
			if (node->value) {
				retV << " DATA";
			};
			retV << "\r\n";
		};
		return retV;
	};

	bool generateDefFile(const char *fileName, SymbolList &symbols) {
		return Shell::filePutContents(fileName, generateDef(symbols));
	};

};
