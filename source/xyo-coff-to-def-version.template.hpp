//
// XYO Coff To Def
//
// Copyright (c) 2020 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_COFF_TO_DEF_VERSION_HPP
#define XYO_COFF_TO_DEF_VERSION_HPP

#define XYO_COFF_TO_DEF_VERSION_ABCD                $VERSION_ABCD
#define XYO_COFF_TO_DEF_VERSION_STR                 "$VERSION_VERSION"
#define XYO_COFF_TO_DEF_VERSION_STR_BUILD           "$VERSION_BUILD"
#define XYO_COFF_TO_DEF_VERSION_STR_DATETIME        "$VERSION_DATETIME"

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

