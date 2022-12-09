// Coff To Def
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_COFFTODEF_LICENSE_HPP
#define XYO_COFFTODEF_LICENSE_HPP

#ifndef XYO_COFFTODEF_DEPENDENCY_HPP
#	include <XYO/CoffToDef/Dependency.hpp>
#endif

namespace XYO::CoffToDef::License {

	const char *licenseHeader();
	const char *licenseBody();
	const char *shortLicense();

};

#endif
