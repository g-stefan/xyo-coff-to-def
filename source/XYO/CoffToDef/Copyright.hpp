// Coff To Def
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_COFFTODEF_COPYRIGHT_HPP
#define XYO_COFFTODEF_COPYRIGHT_HPP

#ifndef XYO_COFFTODEF_DEPENDENCY_HPP
#	include <XYO/CoffToDef/Dependency.hpp>
#endif

namespace XYO::CoffToDef::Copyright {
	const char *copyright();
	const char *publisher();
	const char *company();
	const char *contact();
};

#endif
