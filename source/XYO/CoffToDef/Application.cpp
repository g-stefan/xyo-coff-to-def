// Coff To Def
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <XYO/CoffToDef/Application.hpp>
#include <XYO/CoffToDef/Copyright.hpp>
#include <XYO/CoffToDef/License.hpp>
#include <XYO/CoffToDef/Version.hpp>

namespace XYO::CoffToDef {

	void Application::showUsage() {
		printf("xyo-coff-to-def - Extract symbols from COFF object and generate a DEF file for linker\n");
		printf("version %s build %s [%s]\n", CoffToDef::Version::version(), CoffToDef::Version::build(), CoffToDef::Version::datetime());
		printf("%s\n\n", CoffToDef::Copyright::copyright());
		printf("%s\n",
		       "usage:\n"
		       "    xyo-coff-to-def [--out file] [--mode type] [--show] foo1.obj foo2.obj ...\n\n"
		       "options:\n"
		       "    --out file     output file (default out.def)\n"
		       "    --mode type    mode of operation [ WIN32 | WIN64 ]\n"
		       "    --license      show license\n"
		       "    --show         show coff symbols\n");
	};

	void Application::showLicense() {
		printf("%s", CoffToDef::License::license());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", CoffToDef::Version::version(), CoffToDef::Version::build(), CoffToDef::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		Error::initMemory();
		TMemory<TRedBlackTreeOne<String>>::initMemory();
	};

	int Application::main(int cmdN, char *cmdS[]) {
		TRedBlackTreeOne<String> coffList;
		TRedBlackTreeOne<String> defList;
		int i;
		char *opt;
		char *defFile;
		int coffMode;
		int showCoffSymbols;
		TRedBlackTreeOne<String>::TNode *coff;

		showCoffSymbols = 0;
		defFile = "out.def";
		coffMode = 0;

		if (cmdN < 2) {
			showUsage();
			return 0;
		};

		for (i = 1; i < cmdN; ++i) {
			if (strncmp(cmdS[i], "--", 2) == 0) {
				opt = &cmdS[i][2];
				if (strcmp(opt, "out") == 0) {
					if (i + 1 < cmdN) {
						defFile = cmdS[i + 1];
						++i;
					};
					continue;
				} else if (strcmp(opt, "mode") == 0) {
					if (i + 1 < cmdN) {
						if (strcmp(cmdS[i + 1], "WIN32") == 0) {
							coffMode = 0;
						};
						if (strcmp(cmdS[i + 1], "WIN64") == 0) {
							coffMode = 1;
						};
						++i;
					};
					continue;
				};
				if (strcmp(opt, "license") == 0) {
					showLicense();
				};
				if (strcmp(opt, "show") == 0) {
					showCoffSymbols = 1;
				};
			} else {
				coffList.set(String(cmdS[i]));
			};
		};

		for (coff = coffList.begin(); coff; coff = coff->successor()) {
			if (coff->key[0] == '@') {
				FILE *in;
				in = fopen((char *)&(coff->key.value())[1], "rb");
				if (in != nullptr) {
					int k;
					char buffer[1024];
					while (fgets(buffer, 1024, in)) {
						if (buffer[0] == '/') {
							if (buffer[1] == '/') {
								continue;
							};
						};
						for (k = strlen(buffer); k >= 0; --k) {
							if (buffer[k] == '\r') {
								buffer[k] = 0;
							};
							if (buffer[k] == '\n') {
								buffer[k] = 0;
							};
						};
						getCoffSymbolsFromFile((PTSTR)buffer, defList, showCoffSymbols);
					};
					fclose(in);
				};
			} else {
				getCoffSymbolsFromFile((PTSTR)coff->key.value(), defList, showCoffSymbols);
			};
		};

		if (generateDefFile(defList, defFile, coffMode)) {
		} else {
			printf("Error: unable to generate def file %s\n", defFile);
			return 1;
		};

		return 0;
	};

	void Application::getCoffSymbolsFromFile(PTSTR pszFileName, TRedBlackTreeOne<String> &retV, int showCoffSymbols) {
		HANDLE hFile;
		HANDLE hFileMapping;
		PVOID pMemoryMappedFileBase;
		PIMAGE_FILE_HEADER pFileHdr;
		PIMAGE_SYMBOL pSymbol;
		PIMAGE_SYMBOL pSymbolEnd;
		PSTR pStringTable;
		PSTR pszName;
		char shortName[9];

		hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE) {
			hFileMapping = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
			if (hFileMapping != nullptr) {
				pMemoryMappedFileBase = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
				if (pMemoryMappedFileBase != nullptr) {
					pFileHdr = (PIMAGE_FILE_HEADER)pMemoryMappedFileBase;
					if ((pFileHdr->Machine == IMAGE_FILE_MACHINE_I386) ||
					    (pFileHdr->Machine == IMAGE_FILE_MACHINE_AMD64)) {

						if (showCoffSymbols) {
							printf("Coff Symbols: %s\n\n", pszFileName);
						};

						pSymbol = (PIMAGE_SYMBOL)((unsigned char *)pFileHdr + (pFileHdr->PointerToSymbolTable));
						pSymbolEnd = pSymbol + pFileHdr->NumberOfSymbols;
						pStringTable = (PSTR)pSymbolEnd;

						for (; pSymbol < pSymbolEnd; pSymbol += (pSymbol->NumberOfAuxSymbols + 1)) {
							if (pSymbol->StorageClass == IMAGE_SYM_CLASS_EXTERNAL) {
								if (pSymbol->SectionNumber == IMAGE_SYM_UNDEFINED) {
									continue;
								};
								if (pSymbol->N.Name.Short) {
									memset(shortName, 0, 9);
									memcpy(shortName, pSymbol->N.ShortName, 8);
									pszName = shortName;
								} else {
									pszName = pStringTable + pSymbol->N.Name.Long;
								};

								if (showCoffSymbols) {
									printf("%s\n", pszName);
								};

								retV.set(String(pszName));
							};
						};
					};

					UnmapViewOfFile(pMemoryMappedFileBase);
				};
				CloseHandle(hFileMapping);
			};
			CloseHandle(hFile);
		};
	};

	int Application::generateDefFile(TRedBlackTreeOne<String> &inList, PTSTR pszFileName, int mode) {
		FILE *out;
		int retV;
		retV = 0;
		out = fopen(pszFileName, "wb");
		if (out != nullptr) {
			fprintf(out, "%s", "LIBRARY\r\nEXPORTS\r\n");
			TRedBlackTreeOne<String>::TNode *i;
			for (i = inList.begin(); i; i = i->successor()) {
				if (StringCore::beginWith(i->key, "__real@")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "??_")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__CT??")) {
					continue;
				};
				if (i->key == "__CTA1_N") {
					continue;
				};
				if (StringCore::beginWith(i->key, "__CTA2?")) {
					continue;
				};
				if (i->key == "__TI1_N") {
					continue;
				};
				if (StringCore::beginWith(i->key, "__TI2?")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__CTA1?AV")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__CTA2PAV")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__TI1?AV")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__TI2PAV")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__mask@@")) {
					continue;
				};
				if (StringCore::beginWith(i->key, "__xmm@")) {
					continue;
				};

				if (mode == 0) {
					if (StringCore::beginWith(i->key, "_")) {
						size_t index;
						if (StringCore::indexOf(i->key, "@", index)) {
							fwrite(i->key.value(), 1, i->key.length(), out);
						} else {
							fwrite(i->key.index(1), 1, i->key.length() - 1, out);
						};
						fwrite("\r\n", 1, 2, out);
					};
					if (StringCore::beginWith(i->key, "?")) {
						fwrite(i->key.value(), 1, i->key.length(), out);
						fwrite("\r\n", 1, 2, out);
					};
				} else if (mode == 1) {
					fwrite(i->key.value(), 1, i->key.length(), out);
					fwrite("\r\n", 1, 2, out);
				};
			};

			fclose(out);
			retV = 1;
		};
		return retV;
	};

};

#ifndef XYO_COFFTODEF_LIBRARY
XYO_APPLICATION_MAIN(XYO::CoffToDef::Application);
#endif
