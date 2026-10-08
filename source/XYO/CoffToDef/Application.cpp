// Coff To Def
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <string.h>

#include <XYO/CoffToDef/Application.hpp>
#include <XYO/CoffToDef/Copyright.hpp>
#include <XYO/CoffToDef/License.hpp>
#include <XYO/CoffToDef/Version.hpp>

namespace XYO::CoffToDef {

	void Application::showUsage() {
		printf("xyo-coff-to-def - Extract symbols from COFF object and generate a DEF file for linker\n");
		showVersion();
		printf("%s\n\n", CoffToDef::Copyright::copyright());
		printf("%s\n",
		       "usage:\n"
		       "    xyo-coff-to-def [--out file] [--mode type] [--show] foo1.obj foo2.obj @list.txt ...\n\n"
		       "options:\n"
		       "    --out file     output file (default out.def)\n"
		       "    --mode type    mode of operation [ AUTO | WIN32 | WIN64 ] (default AUTO)\n"
		       "                   AUTO: WIN32 for i386 objects, WIN64 for x64 / ARM64 objects\n"
		       "                   WIN32: \"_name\" is exported as \"name\"\n"
		       "                   WIN64: names are exported as they are\n"
		       "    --show         show coff symbols\n"
		       "    --license      show license\n"
		       "    --version      show version\n"
		       "    @list.txt      read object file names from list.txt, one per line\n");
	};

	void Application::showLicense() {
		printf("%s", CoffToDef::License::license().c_str());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", CoffToDef::Version::version(), CoffToDef::Version::build(), CoffToDef::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		Error::initMemory();
		Buffer::initMemory();
		TMemory<TDynamicArray<String>>::initMemory();
		TMemory<Coff::SymbolList>::initMemory();
	};

	bool Application::readResponseFile(const String &fileName, TDynamicArray<String> &fileList, int level) {
		String content;
		TDynamicArray<String> lines;
		int k;

		if (level >= 16) {
			printf("Error: %s: response files nested too deep\n", fileName.value());
			return false;
		};
		if (!Shell::fileGetContents(fileName.value(), content)) {
			printf("Error: %s: unable to read response file\n", fileName.value());
			return false;
		};
		// UTF-8 BOM
		if (content.beginWith("\xEF\xBB\xBF")) {
			content = content.substring(3);
		};

		content.explode("\n", lines);
		for (k = 0; k < (int)lines.length(); ++k) {
			String line = lines[k].trimASCII();
			if (line.beginWith("//")) {
				continue;
			};
			if (line.length() >= 2) {
				if ((line[0] == '"') && (line[line.length() - 1] == '"')) {
					line = line.substring(1, line.length() - 2);
				};
			};
			if (line.length() == 0) {
				continue;
			};
			if (line[0] == '@') {
				if (!readResponseFile(line.substring(1), fileList, level + 1)) {
					return false;
				};
				continue;
			};
			fileList.push(line);
		};
		return true;
	};

	int Application::main(int cmdN, char *cmdS[]) {
		TDynamicArray<String> fileList;
		Coff::SymbolList symbols;
		String defFile;
		String error;
		int mode;
		bool showCoffSymbols;
		bool showInfo;
		bool isOk;
		int i;
		char *opt;

		defFile = "out.def";
		mode = Coff::ModeAuto;
		showCoffSymbols = false;
		showInfo = false;

		if (cmdN < 2) {
			showUsage();
			return 0;
		};

		for (i = 1; i < cmdN; ++i) {
			if (strncmp(cmdS[i], "--", 2) == 0) {
				opt = &cmdS[i][2];
				if (strcmp(opt, "out") == 0) {
					if (i + 1 >= cmdN) {
						printf("Error: --out requires a file name\n");
						return 1;
					};
					++i;
					defFile = cmdS[i];
					continue;
				};
				if (strcmp(opt, "mode") == 0) {
					if (i + 1 >= cmdN) {
						printf("Error: --mode requires AUTO, WIN32 or WIN64\n");
						return 1;
					};
					++i;
					String value = String(cmdS[i]).toUpperCaseASCII();
					if (value == "WIN32") {
						mode = Coff::ModeWin32;
					} else if (value == "WIN64") {
						mode = Coff::ModeWin64;
					} else if (value == "AUTO") {
						mode = Coff::ModeAuto;
					} else {
						printf("Error: unknown mode %s, use AUTO, WIN32 or WIN64\n", cmdS[i]);
						return 1;
					};
					continue;
				};
				if (strcmp(opt, "license") == 0) {
					showLicense();
					showInfo = true;
					continue;
				};
				if (strcmp(opt, "version") == 0) {
					showVersion();
					showInfo = true;
					continue;
				};
				if (strcmp(opt, "show") == 0) {
					showCoffSymbols = true;
					continue;
				};
				printf("Error: unknown option %s\n", cmdS[i]);
				return 1;
			};
			if (cmdS[i][0] == '@') {
				if (!readResponseFile(&cmdS[i][1], fileList, 0)) {
					return 1;
				};
				continue;
			};
			fileList.push(cmdS[i]);
		};

		if (fileList.length() == 0) {
			if (showInfo) {
				return 0;
			};
			printf("Error: no input files\n");
			return 1;
		};

		isOk = true;
		for (i = 0; i < (int)fileList.length(); ++i) {
			if (!Coff::getSymbolsFromFile(fileList[i].value(), mode, symbols, showCoffSymbols, error)) {
				printf("Error: %s: %s\n", fileList[i].value(), error.value());
				isOk = false;
			};
		};
		if (!isOk) {
			return 1;
		};

		if (!Coff::generateDefFile(defFile.value(), symbols)) {
			printf("Error: unable to generate def file %s\n", defFile.value());
			return 1;
		};

		return 0;
	};

};

#ifndef XYO_COFFTODEF_LIBRARY
XYO_APPLICATION_MAIN(XYO::CoffToDef::Application);
#endif
