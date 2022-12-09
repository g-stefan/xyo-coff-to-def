// Coff To Def
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_COFFTODEF_APPLICATION_HPP
#define XYO_COFFTODEF_APPLICATION_HPP

#ifndef XYO_COFFTODEF_DEPENDENCY_HPP
#	include <XYO/CoffToDef/Dependency.hpp>
#endif

namespace XYO::CoffToDef {

	class Application : public virtual IApplication {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(Application);

		public:
			inline Application(){};

			void showUsage();
			void showLicense();
			void showVersion();

			int main(int cmdN, char *cmdS[]);

			static void initMemory();

			void getCoffSymbolsFromFile(PTSTR pszFileName, TRedBlackTreeOne<String> &retV, int showCoffSymbols);
			int generateDefFile(TRedBlackTreeOne<String> &inList, PTSTR pszFileName, int mode);
	};

};

#endif
