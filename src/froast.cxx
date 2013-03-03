// Copyright (C) 2011 Oliver Schulz <oliver.schulz@tu-dortmund.de>

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include <TROOT.h>
#include <TFile.h>
#include <THashList.h>

#include "util.h"
#include "Selector.h"
#include "Settings.h"

using namespace std;
using namespace froast;



int settings(int argc, char *argv[], char *envp[]) {
	if (argc > 2) {
		cerr << "Syntax: " << argv[0] << " ROOT_FILE" << endl;
		return 1;
	}

	if (argc == 1) {
		Settings::global().write(cout);
		return 0;
	} else if ((argc >= 2) && (argc <= 3)) {
		string inFileName = argv[1];
		TFile inFile(inFileName.c_str(), "read");
		Settings settings;
		if (argc >= 3) settings.read(&inFile, argv[2]);
		else settings.read(&inFile);
		settings.write(cout);
		return 0;
	} else {
		cerr << "Syntax: " << argv[0] << " ROOT_FILE" << endl;
		return 1;
	}
}


int map_single(int argc, char *argv[], char *envp[]) {
	if (argc != 4) {
		cerr << "Syntax: " << argv[0] << " INPUT_FILE MAPPER OUTPUT_FILE" << endl;
		return 1;
	}
	string inFileName = argv[1];
	string mappers = argv[2];
	string outFileName = argv[3];
	Selector::mapSingle(inFileName, mappers, outFileName);

	return 0;
}


int map_multi(int argc, char *argv[], char *envp[]) {
	const size_t firstInputArg = 3;
	if (argc < firstInputArg) {
		cerr << "Syntax: " << argv[0] << " MAPPERS TAG [INPUT]..." << endl;
		return 1;
	}
	string mappers = argv[1];
	string tag = argv[2];
	for (size_t arg = firstInputArg; arg < argc; ++arg) {
		Selector::mapMulti(argv[arg], mappers, tag, arg > firstInputArg);
	}

	return 0;
}


int main(int argc, char *argv[], char *envp[]) {
	try {
		// Have to tell ROOT to load vector dlls, otherwise ROOT will produce
		// "is not of a class known to ROOT" errors on creation of STL vector
		// branches:
		gROOT->ProcessLine("#include <vector>");
		
		string progName(argv[0]);

		if (argc < 2) {
			cerr << "Syntax: " << progName << " COMMAND ..." << endl << endl;
			cerr << "Commands: " << endl;
			cerr << "  settings" << endl;
			cerr << "  map-single" << endl;
			cerr << "  map-multi" << endl;
			return 1;
		}
		
		string cmd(argv[1]);

		int cmd_argc = argc - 1;
		char **cmd_argv = new char*[cmd_argc];
		cmd_argv[0] = strdup((progName + " " + cmd).c_str());
		for (int i = 1; i < cmd_argc; ++i) cmd_argv[i] = strdup(argv[i+1]);
		cmd_argv = argv + 1;

		if (cmd == "settings") return settings(cmd_argc, cmd_argv, envp);
		else if (cmd == "map-single") return map_single(cmd_argc, cmd_argv, envp);
		else if (cmd == "map-multi") return map_multi(cmd_argc, cmd_argv, envp);
		else {
			cerr << "ERROR: " << progName << " does not support command \"" << cmd << "\"" << endl;
		}
		
		for (int i = 0; i < cmd_argc; ++i) ::free(cmd_argv[i]);
		delete [] cmd_argv;
	}
	catch(std::exception &e) {
		cerr << endl << endl << "Exception: " << e.what() << endl << endl;

		return 1;
	}
}
