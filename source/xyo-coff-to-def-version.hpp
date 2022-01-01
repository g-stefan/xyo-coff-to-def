//
// XYO Coff To Def
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_COFF_TO_DEF_VERSION_HPP
#define XYO_COFF_TO_DEF_VERSION_HPP

#define XYO_COFF_TO_DEF_VERSION_ABCD                1,9,0,19
#define XYO_COFF_TO_DEF_VERSION_STR                 "1.9.0"
#define XYO_COFF_TO_DEF_VERSION_STR_BUILD           "19"
#define XYO_COFF_TO_DEF_VERSION_STR_DATETIME        "2021-11-22 13:35:05"

#ifndef XYO_RC

namespace XYOCoffToDef {
	namespace Version {
		const char *version();
		const char *build();
		const char *versionWithBuild();
		const char *datetime();
	};
};

#endif
#endif

