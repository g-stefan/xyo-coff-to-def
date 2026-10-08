// Coff To Def
// Copyright (c) 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_COFFTODEF_APPLICATION_HPP
#define XYO_COFFTODEF_APPLICATION_HPP

#ifndef XYO_COFFTODEF_COFF_HPP
#	include <XYO/CoffToDef/Coff.hpp>
#endif

namespace XYO::CoffToDef {

	class Application : public virtual IApplication {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

		public:
			inline Application(){};

			void showUsage();
			void showLicense();
			void showVersion();

			int main(int cmdN, char *cmdS[]);

			static void initMemory();

			// Expand a response file (@file): one object per line, "//" comments, empty lines ignored
			bool readResponseFile(const String &fileName, TDynamicArray<String> &fileList, int level);
	};

};

#endif
