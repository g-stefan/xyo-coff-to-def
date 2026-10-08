// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <stdio.h>
#include <string.h>

#include <stdexcept>
#include <string>
#include <vector>

#include <XYO/CoffToDef/Application.hpp>

using namespace XYO::CoffToDef;

static void check(bool value, const char *expression, int line) {
	if (!value) {
		char message[1024];
		snprintf(message, sizeof(message), "line %d: %s", line, expression);
		throw std::runtime_error(message);
	};
};

#define CHECK(x) check((x), #x, __LINE__)

// --- COFF object builder

static const uint8_t classStatic = 3;
static const uint8_t classExternal = 2;
static const uint16_t typeData = 0x00;
static const uint16_t typeFunction = 0x20;

static void put8(std::string &out, uint8_t value) {
	out += (char)value;
};

static void put16(std::string &out, uint16_t value) {
	put8(out, (uint8_t)(value & 0xFF));
	put8(out, (uint8_t)(value >> 8));
};

static void put32(std::string &out, uint32_t value) {
	put16(out, (uint16_t)(value & 0xFFFF));
	put16(out, (uint16_t)(value >> 16));
};

static void patch32(std::string &out, size_t offset, uint32_t value) {
	std::string x;
	put32(x, value);
	out.replace(offset, 4, x);
};

class CoffBuilder {
	public:
		uint16_t machine;
		bool bigObj;
		uint16_t version;
		std::string symbols;
		std::string strings;
		uint32_t numberOfSymbols;

		CoffBuilder(uint16_t machine_, bool bigObj_ = false) {
			machine = machine_;
			bigObj = bigObj_;
			version = 2;
			numberOfSymbols = 0;
		};

		size_t headerSize() const {
			return bigObj ? 56 : 20;
		};

		size_t symbolSize() const {
			return bigObj ? 20 : 18;
		};

		void name(std::string &out, const char *name_) {
			size_t ln = strlen(name_);
			if (ln <= 8) {
				char shortName[8];
				memset(shortName, 0, 8);
				memcpy(shortName, name_, ln);
				out.append(shortName, 8);
				return;
			};
			put32(out, 0);
			put32(out, (uint32_t)(4 + strings.size()));
			strings.append(name_, ln + 1);
		};

		void symbol(const char *name_, int32_t section, uint16_t type, uint8_t storageClass, uint32_t value = 0, uint8_t numberOfAux = 0) {
			name(symbols, name_);
			put32(symbols, value);
			if (bigObj) {
				put32(symbols, (uint32_t)section);
			} else {
				put16(symbols, (uint16_t)(int16_t)section);
			};
			put16(symbols, type);
			put8(symbols, storageClass);
			put8(symbols, numberOfAux);
			++numberOfSymbols;
			// Aux records that look like an external function, they must be skipped
			for (uint8_t k = 0; k < numberOfAux; ++k) {
				std::string aux;
				aux.append("_auxFake", 8);
				put32(aux, 0);
				if (bigObj) {
					put32(aux, 1);
				} else {
					put16(aux, 1);
				};
				put16(aux, typeFunction);
				put8(aux, classExternal);
				put8(aux, 0);
				symbols += aux;
				++numberOfSymbols;
			};
		};

		void function(const char *name_, int32_t section = 1, uint8_t numberOfAux = 0) {
			symbol(name_, section, typeFunction, classExternal, 0, numberOfAux);
		};

		void data(const char *name_, int32_t section = 2) {
			symbol(name_, section, typeData, classExternal);
		};

		void common(const char *name_, uint32_t size) {
			symbol(name_, 0, typeData, classExternal, size);
		};

		void undefined(const char *name_) {
			symbol(name_, 0, typeFunction, classExternal);
		};

		std::string build(bool withStringTable = true) {
			std::string out;
			// {D1BAA1C7-BAEE-4BA9-AF20-FAF66AA4DCB8}
			static const uint8_t classIdBigObj[16] = {
			    0xC7, 0xA1, 0xBA, 0xD1, 0xEE, 0xBA, 0xA9, 0x4B,
			    0xAF, 0x20, 0xFA, 0xF6, 0x6A, 0xA4, 0xDC, 0xB8};
			if (bigObj) {
				put16(out, 0);
				put16(out, 0xFFFF);
				put16(out, version);
				put16(out, machine);
				put32(out, 0);
				out.append((const char *)classIdBigObj, 16);
				put32(out, 0);
				put32(out, 0);
				put32(out, 0);
				put32(out, 0);
				put32(out, 2);
				put32(out, (uint32_t)headerSize());
				put32(out, numberOfSymbols);
			} else {
				put16(out, machine);
				put16(out, 2);
				put32(out, 0);
				put32(out, (uint32_t)headerSize());
				put32(out, numberOfSymbols);
				put16(out, 0);
				put16(out, 0);
			};
			out += symbols;
			if (withStringTable) {
				put32(out, (uint32_t)(4 + strings.size()));
				out += strings;
			};
			return out;
		};
};

// The constants and compiler generated symbols a real compiler adds, none of them is exported
static void addFiltered(CoffBuilder &coff, bool i386) {
	const char *names[] = {
	    "__real@400921f9f01b866e",
	    "__xmm@00000000000000000000000000000000",
	    "__ymm@0000000000000000000000000000000000000000000000000000000000000000",
	    "__zmm@00",
	    "__mask@@NegDouble@",
	    "??_7S@@6B@",
	    "??_R4S@@6B@",
	    "??_C@_01A@?$AA@",
	    "??_GS@@UAEPAXI@Z",
	    "__@@_PchSym_@00@UfoodUyzi@"};
	for (const char *name : names) {
		coff.data(name);
	};
	// throw info, i386 names have one more "_" in front
	const char *throwInfo[] = {
	    "_CT??_R0H@84",
	    "_CT??_R0?AVexception@std@@@8??0exception@std@@QEAA@AEBV01@@Z24",
	    "_CTA1H",
	    "_CTA2?AVruntime_error@std@@",
	    "_TI1H",
	    "_TI2?AVruntime_error@std@@",
	    "_TIC1?AVexception@std@@",
	    "_TICU2PAD"};
	for (const char *name : throwInfo) {
		coff.data(i386 ? (std::string("_") + name).c_str() : name);
	};
};

static std::string toDef(const std::vector<const char *> &lines) {
	std::string retV = "LIBRARY\r\nEXPORTS\r\n";
	for (const char *line : lines) {
		retV += line;
		retV += "\r\n";
	};
	return retV;
};

static void checkDef(const String &def, const std::vector<const char *> &expected, int line) {
	std::string expectedDef = toDef(expected);
	if (std::string(def.value()) != expectedDef) {
		printf("--- expected\n%s--- found\n%s---\n", expectedDef.c_str(), def.value());
	};
	check(std::string(def.value()) == expectedDef, "def content", line);
};

#define CHECK_DEF(def, ...) checkDef(def, __VA_ARGS__, __LINE__)

static String defFromObject(const std::string &object, int mode, int line) {
	Coff::SymbolList symbols;
	String error;
	bool isOk = Coff::getSymbols((const uint8_t *)object.data(), object.size(), mode, symbols, false, error);
	if (!isOk) {
		printf("* %s\n", error.value());
	};
	check(isOk, "getSymbols", line);
	return Coff::generateDef(symbols);
};

#define DEF_FROM_OBJECT(object, mode) defFromObject(object, mode, __LINE__)

static bool isInvalid(const std::string &object) {
	Coff::SymbolList symbols;
	String error;
	if (Coff::getSymbols((const uint8_t *)object.data(), object.size(), Coff::ModeAuto, symbols, false, error)) {
		return false;
	};
	return error.length() > 0;
};

// --- objects

static std::string objectI386() {
	CoffBuilder coff(Coff::MachineI386);
	coff.function("_cdeclFn");
	coff.function("_stdFn@8");
	coff.function("@fastFn@8");
	coff.function("?cppFn@@YAHH@Z");
	coff.data("_dataVar");
	coff.data("?cppVar@@3HA");
	coff.common("_commonVar", 4);
	coff.undefined("_printf");
	coff.symbol("_hidden", 1, typeFunction, classStatic);
	coff.symbol("_absolute", -1, typeData, classExternal);
	coff.symbol("_debug", -2, typeData, classExternal);
	coff.function("_longFunctionName");
	coff.function("_abcdefg");
	coff.function("_withAux", 1, 1);
	coff.function("_TIMER");
	addFiltered(coff, true);
	return coff.build();
};

static const std::vector<const char *> objectI386Win32 = {
    "?cppFn@@YAHH@Z",
    "?cppVar@@3HA DATA",
    "@fastFn@8",
    "TIMER",
    "_stdFn@8",
    "abcdefg",
    "cdeclFn",
    "commonVar DATA",
    "dataVar DATA",
    "longFunctionName",
    "withAux"};

static std::string objectAMD64() {
	CoffBuilder coff(Coff::MachineAMD64);
	coff.function("cdeclFn");
	coff.function("?cppFn@@YAHH@Z");
	coff.data("dataVar");
	coff.common("commonVar", 8);
	coff.function("_userName");
	coff.function("longFunctionNameForAmd64", 3, 2);
	coff.undefined("__imp_GetLastError");
	addFiltered(coff, false);
	return coff.build();
};

static const std::vector<const char *> objectAMD64Win64 = {
    "?cppFn@@YAHH@Z",
    "_userName",
    "cdeclFn",
    "commonVar DATA",
    "dataVar DATA",
    "longFunctionNameForAmd64"};

// --- tests

static void testExportName() {
	CHECK(Coff::exportName("_cdeclFn", Coff::ModeWin32) == "cdeclFn");
	CHECK(Coff::exportName("_stdFn@8", Coff::ModeWin32) == "_stdFn@8");
	CHECK(Coff::exportName("@fastFn@8", Coff::ModeWin32) == "@fastFn@8");
	CHECK(Coff::exportName("?fn@@YAXXZ", Coff::ModeWin32) == "?fn@@YAXXZ");
	CHECK(Coff::exportName("__name", Coff::ModeWin32) == "_name");
	CHECK(Coff::exportName("_", Coff::ModeWin32) == "_");
	CHECK(Coff::exportName("_cdeclFn", Coff::ModeWin64) == "_cdeclFn");
	CHECK(Coff::exportName("cdeclFn", Coff::ModeWin64) == "cdeclFn");
};

static void testFilter() {
	CHECK(Coff::isFiltered("__real@400921f9f01b866e"));
	CHECK(Coff::isFiltered("??_7S@@6B@"));
	CHECK(Coff::isFiltered("_CTA2?AVruntime_error@std@@"));
	CHECK(Coff::isFiltered("__CTA2?AVruntime_error@std@@"));
	CHECK(Coff::isFiltered("_TI1H"));
	CHECK(Coff::isFiltered("__TI1H"));
	CHECK(Coff::isFiltered("_TICU2PAD"));
	CHECK(!Coff::isFiltered("_TIMER"));
	CHECK(!Coff::isFiltered("_CTAB"));
	CHECK(!Coff::isFiltered("?fn@@YAXXZ"));
	CHECK(!Coff::isFiltered("cdeclFn"));
	CHECK(!Coff::isFiltered("_"));
};

static void testI386() {
	std::string object = objectI386();
	CHECK_DEF(DEF_FROM_OBJECT(object, Coff::ModeAuto), objectI386Win32);
	CHECK_DEF(DEF_FROM_OBJECT(object, Coff::ModeWin32), objectI386Win32);
	CHECK_DEF(DEF_FROM_OBJECT(object, Coff::ModeWin64),
	          {"?cppFn@@YAHH@Z",
	           "?cppVar@@3HA DATA",
	           "@fastFn@8",
	           "_TIMER",
	           "_abcdefg",
	           "_cdeclFn",
	           "_commonVar DATA",
	           "_dataVar DATA",
	           "_longFunctionName",
	           "_stdFn@8",
	           "_withAux"});
};

static void testAMD64() {
	std::string object = objectAMD64();
	CHECK_DEF(DEF_FROM_OBJECT(object, Coff::ModeAuto), objectAMD64Win64);
	CHECK_DEF(DEF_FROM_OBJECT(object, Coff::ModeWin64), objectAMD64Win64);
};

static void testARM64() {
	CoffBuilder coff(Coff::MachineARM64);
	coff.function("arm64Function");
	coff.data("_arm64Data");
	CHECK_DEF(DEF_FROM_OBJECT(coff.build(), Coff::ModeAuto), {"_arm64Data DATA", "arm64Function"});
};

static void testBigObj() {
	CoffBuilder coff64(Coff::MachineAMD64, true);
	coff64.function("bigFunction", 40000, 1);
	coff64.data("bigDataWithLongName", 70000);
	coff64.undefined("undefinedFunction");
	coff64.symbol("absoluteSymbol", -1, typeData, classExternal);
	addFiltered(coff64, false);
	CHECK_DEF(DEF_FROM_OBJECT(coff64.build(), Coff::ModeAuto), {"bigDataWithLongName DATA", "bigFunction"});

	CoffBuilder coff32(Coff::MachineI386, true);
	coff32.function("_bigFunction", 40000);
	coff32.function("_bigStd@4", 2);
	coff32.common("_bigCommon", 16);
	addFiltered(coff32, true);
	CHECK_DEF(DEF_FROM_OBJECT(coff32.build(), Coff::ModeAuto), {"_bigStd@4", "bigCommon DATA", "bigFunction"});
};

static void testEmpty() {
	CoffBuilder coff(Coff::MachineAMD64);
	CHECK_DEF(DEF_FROM_OBJECT(coff.build(), Coff::ModeAuto), {});
	CHECK_DEF(DEF_FROM_OBJECT(coff.build(false), Coff::ModeAuto), {});

	// Short names only, the file ends after the symbol table
	CoffBuilder coffShort(Coff::MachineAMD64);
	coffShort.function("fn");
	CHECK_DEF(DEF_FROM_OBJECT(coffShort.build(false), Coff::ModeAuto), {"fn"});
};

static void testInvalid() {
	CHECK(isInvalid(""));
	CHECK(isInvalid(std::string("\x4C\x01\x00", 3)));
	CHECK(isInvalid(std::string("\x4C\x01\x02\x00\x00\x00\x00\x00\x14\x00", 10)));
	CHECK(isInvalid("!<arch>\n/               0           0     0     0       4         `\n"));
	CHECK(isInvalid(std::string("MZ\x90\x00\x03\x00\x00\x00\x04\x00\x00\x00\xFF\xFF\x00\x00\xB8\x00\x00\x00", 20)));

	// Unknown machine
	CoffBuilder unknown(0x1234);
	unknown.function("fn");
	CHECK(isInvalid(unknown.build()));

	// LTCG (/GL) and import objects are anonymous objects that are not /bigobj
	CoffBuilder ltcg(Coff::MachineAMD64, true);
	ltcg.version = 1;
	ltcg.function("fn");
	CHECK(isInvalid(ltcg.build()));
	ltcg.version = 2;
	CHECK(!isInvalid(ltcg.build()));
	std::string ltcgObject = ltcg.build();
	ltcgObject[12] = 0;
	CHECK(isInvalid(ltcgObject));
	CHECK(isInvalid(std::string("\x00\x00\xFF\xFF\x00\x00\x4C\x01\x00\x00\x00\x00\x08\x00\x00\x00", 16)));

	// Bad symbol table position / count
	CoffBuilder coff(Coff::MachineI386);
	coff.function("_longFunctionName");
	std::string object = coff.build();
	std::string x = object;
	patch32(x, 8, 0xFFFFFF00);
	CHECK(isInvalid(x));
	x = object;
	patch32(x, 12, 0x7FFFFFFF);
	CHECK(isInvalid(x));
	x = object;
	patch32(x, 12, 0xFFFFFFFF);
	CHECK(isInvalid(x));

	// Long name outside of the string table
	x = object;
	patch32(x, 20 + 4, 0x10000);
	CHECK(isInvalid(x));
	x = object;
	patch32(x, 20 + 4, 2);
	CHECK(isInvalid(x));

	// String table size bigger than the file
	x = object;
	patch32(x, 20 + 18, 0x10000);
	CHECK(isInvalid(x));

	// Long name not terminated
	x = object;
	x.resize(x.size() - 1);
	patch32(x, 20 + 18, (uint32_t)(x.size() - (20 + 18)));
	CHECK(isInvalid(x));

	// Every truncation of a valid object fails cleanly or reads a valid prefix, never crashes
	std::string big = objectI386();
	for (size_t k = 0; k < big.size(); ++k) {
		Coff::SymbolList symbols;
		String error;
		Coff::getSymbols((const uint8_t *)big.data(), k, Coff::ModeAuto, symbols, false, error);
	};
};

// --- command line

static int run(const std::vector<const char *> &arguments) {
	std::vector<char *> cmdS;
	cmdS.push_back((char *)"xyo-coff-to-def");
	for (const char *argument : arguments) {
		cmdS.push_back((char *)argument);
	};
	cmdS.push_back(nullptr);
	Application application;
	return application.main((int)cmdS.size() - 1, cmdS.data());
};

static void writeFile(const char *fileName, const std::string &content) {
	CHECK(Shell::filePutContents(fileName, (const uint8_t *)content.data(), content.size()));
};

static void checkDefFile(const char *fileName, const std::vector<const char *> &expected, int line) {
	String content;
	check(Shell::fileGetContents(fileName, content), "read def file", line);
	checkDef(content, expected, line);
};

#define CHECK_DEF_FILE(fileName, ...) checkDefFile(fileName, __VA_ARGS__, __LINE__)

static void testApplication() {
	writeFile("test.01.i386.obj", objectI386());
	writeFile("test.01 amd64.obj", objectAMD64());
	writeFile("test.01.bad.obj", "not an object");
	writeFile("test.01.list.txt", "\xEF\xBB\xBF// objects\r\n"
	                              "\r\n"
	                              "  \"test.01 amd64.obj\"  \r\n"
	                              "@test.01.list2.txt\r\n");
	writeFile("test.01.list2.txt", "test.01.i386.obj\n");
	writeFile("test.01.loop.txt", "@test.01.loop.txt\n");

	Shell::removeFile("test.01.def");
	CHECK(run({"--out", "test.01.def", "test.01.i386.obj"}) == 0);
	CHECK_DEF_FILE("test.01.def", objectI386Win32);

	CHECK(run({"test.01.i386.obj", "--mode", "win64", "--out", "test.01.def"}) == 0);
	CHECK_DEF_FILE("test.01.def", {"?cppFn@@YAHH@Z",
	                               "?cppVar@@3HA DATA",
	                               "@fastFn@8",
	                               "_TIMER",
	                               "_abcdefg",
	                               "_cdeclFn",
	                               "_commonVar DATA",
	                               "_dataVar DATA",
	                               "_longFunctionName",
	                               "_stdFn@8",
	                               "_withAux"});

	// Response files, mixed machines in AUTO mode
	CHECK(run({"--out", "test.01.def", "@test.01.list.txt"}) == 0);
	CHECK_DEF_FILE("test.01.def", {"?cppFn@@YAHH@Z",
	                               "?cppVar@@3HA DATA",
	                               "@fastFn@8",
	                               "TIMER",
	                               "_stdFn@8",
	                               "_userName",
	                               "abcdefg",
	                               "cdeclFn",
	                               "commonVar DATA",
	                               "dataVar DATA",
	                               "longFunctionName",
	                               "longFunctionNameForAmd64",
	                               "withAux"});

	CHECK(run({"--version"}) == 0);

	printf("--- expected errors\n");

	// A bad input: error, the def file is not written
	Shell::removeFile("test.01.def");
	CHECK(run({"--out", "test.01.def", "test.01.i386.obj", "test.01.missing.obj"}) == 1);
	CHECK(!Shell::fileExists("test.01.def"));
	CHECK(run({"--out", "test.01.def", "test.01.bad.obj"}) == 1);
	CHECK(!Shell::fileExists("test.01.def"));
	CHECK(run({"--out", "test.01.def", "@test.01.missing.txt"}) == 1);
	CHECK(run({"--out", "test.01.def", "@test.01.loop.txt"}) == 1);
	CHECK(!Shell::fileExists("test.01.def"));

	// Bad options
	CHECK(run({"--out"}) == 1);
	CHECK(run({"--mode"}) == 1);
	CHECK(run({"--mode", "WIN16", "test.01.i386.obj"}) == 1);
	CHECK(run({"--unknown", "test.01.i386.obj"}) == 1);
	CHECK(run({"--show"}) == 1);
	CHECK(!Shell::fileExists("test.01.def"));

	printf("--- end of expected errors\n");
};

static void test() {
	Application::initMemory();

	testExportName();
	testFilter();
	testI386();
	testAMD64();
	testARM64();
	testBigObj();
	testEmpty();
	testInvalid();
	testApplication();
	printf("Done.\r\n");
};

int main(int cmdN, char *cmdS[]) {

	try {
		test();
		return 0;
	} catch (const std::exception &e) {
		printf("* Error: %s\n", e.what());
	} catch (...) {
		printf("* Error: Unknown\n");
	};

	return 1;
};
