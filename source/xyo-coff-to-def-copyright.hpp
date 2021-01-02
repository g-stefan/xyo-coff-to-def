//
// XYO Coff To Def
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef XYO_COFF_TO_DEF_COPYRIGHT_HPP
#define XYO_COFF_TO_DEF_COPYRIGHT_HPP

#define XYO_COFF_TO_DEF_COPYRIGHT            "Copyright (c) Grigore Stefan"
#define XYO_COFF_TO_DEF_PUBLISHER            "Grigore Stefan"
#define XYO_COFF_TO_DEF_COMPANY              XYO_COFF_TO_DEF_PUBLISHER
#define XYO_COFF_TO_DEF_CONTACT              "g_stefan@yahoo.com"
#define XYO_COFF_TO_DEF_FULL_COPYRIGHT       XYO_COFF_TO_DEF_COPYRIGHT " <" XYO_COFF_TO_DEF_CONTACT ">"

#ifndef XYO_RC

namespace XYOCoffToDef {
	namespace Copyright {
		const char *copyright();
		const char *publisher();
		const char *company();
		const char *contact();
		const char *fullCopyright();
	};
};

#endif
#endif
