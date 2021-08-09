//
// XYO Coff To Def
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include "xyo-coff-to-def-version.hpp"

namespace XYOCoffToDef {
	namespace Version {

		static const char *version_ = "1.6.0";
		static const char *build_ = "9";
		static const char *versionWithBuild_ = "1.6.0.9";
		static const char *datetime_ = "2021-08-05 20:38:19";

		const char *version() {
			return version_;
		};
		const char *build() {
			return build_;
		};
		const char *versionWithBuild() {
			return versionWithBuild_;
		};
		const char *datetime() {
			return datetime_;
		};

	};
};

