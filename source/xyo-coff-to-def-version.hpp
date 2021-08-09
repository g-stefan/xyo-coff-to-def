//
// XYO Coff To Def
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_COFF_TO_DEF_VERSION_HPP
#define XYO_COFF_TO_DEF_VERSION_HPP

#define XYO_COFF_TO_DEF_VERSION_ABCD                1,7,0,18
#define XYO_COFF_TO_DEF_VERSION_STR                 "1.7.0"
#define XYO_COFF_TO_DEF_VERSION_STR_BUILD           "18"
#define XYO_COFF_TO_DEF_VERSION_STR_DATETIME        "2021-08-09 17:38:07"

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

