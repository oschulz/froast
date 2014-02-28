// Copyright (C) 2011 Oliver Schulz <oliver.schulz@tu-dortmund.de>

// This is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <memory>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

#include <TROOT.h>
#include <THashList.h>

#include "../config.h"

#include "logging.h"
#include "util.h"
#include "FroastTools.h"
#include "Settings.h"
#include "TreeEntryList.h"


/*!	\mainpage	Programme to evaluate CPG pulse shape data
	*
	* Syntax: froast COMMAND ... \n
	* Commands: 
	*	- settings [OPTIONS] INPUTFILE
	*	- map-single [OPTIONS] MAPPERS OUTPUTFILE INPUT_FILE
	*	- map-multi [OPTIONS] MAPPERS OUTPUTFILE_TAG INPUTFILES
	*   - reduce [OPTIONS] MAPPERS OUTPUTFILE INPUTFILES
	*	- tabulate [OPTIONS] ROOT_FILE/TREENAME VAREXP [SELECTION [NENTRIES [STARTENTRY]]]
*/

using namespace std;
using namespace froast;


void handleOptionLogging(const char* optarg) {
	log_debug("Setting logging level to %s", optarg);
	log_level(optarg);
	Settings::global().tenv()->SetValue("logging.level", optarg, kEnvChange);
}


void handleOptionConfig(const char* optarg) {
	log_debug("Reading config/settings from \"%s\"", optarg);
	Settings::global().readAuto(optarg);
	// Don't save a default for logging setting:
	string logLevel = Settings::global()("logging.level", "", false);
	if (logLevel != "") log_level(logLevel.c_str());
}


void writeSettings(const Settings &settings, const std::string &format) {
	if (format == "json") settings.writeJSON(cout);
	else if (format == "rootrc") settings.write(cout);
	else throw invalid_argument("Unknown settings output format");
}


// Format of inputSpec: TFILE_NAME + "/" + TREE_NAME
TChain* openTChain(const TString &inputSpec) {
	ssize_t splitPos = inputSpec.Last('/');
	if ((splitPos < 0) || (splitPos >= inputSpec.Length()-1))
		throw invalid_argument("No tree name in input specification");

	TString inFileName = inputSpec(0, splitPos);
	TString treeName = inputSpec(splitPos + 1, inputSpec.Length() - splitPos - 1);
	TChain *chain = new TChain(treeName);

	if (inFileName.First('*')>=inFileName.Last('/')) {
		chain->Add(inFileName);
	    // O. Reinecke: this should assist the TChain with its retarded shell expansion
	} else {
		TObjArray &files=*chain->GetListOfFiles();
		chain->Add((TString)inFileName(0,splitPos=inFileName.First('/')));
		int length=files.GetEntries();
		inFileName.Remove(0,splitPos+1);
		while (inFileName.Length()>0 && length>0) {
			splitPos=inFileName.First('/');
			if (splitPos<0) splitPos=inFileName.Length();
			for (int entry=0;entry<length;entry++)
				chain->Add(TString(files[entry]->GetTitle())+"/"+inFileName(0,splitPos));
			files.RemoveRange(0,length-1);
			files.Compress();
			length=files.GetEntries();
			inFileName.Remove(0,splitPos+1);
		}
		// O. Reinecke: it seems that the operation above messes up chain's file list, so let's just rebuild it here:
		TObjArray a(*(TObjArray*)files.Clone()); chain->Reset();
		for (int entry=0;entry<length;entry++) chain->Add(a[entry]->GetTitle());
	}
}


void settings_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] [SETTINGS].." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-f FORMAT   Set output format (formats: [rootrc], json)" << endl;
	cerr << "-j          Set output format to JSON. DEPRECATED, use \"-f json\" instead." << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Combine and output settings in specified format (.rootrc format by default)." << endl;
	cerr << "If SETTINGS is not specified, the current settings (including settings loaded" << endl;
	cerr << "with the \"-c\"option) are  used." << endl;
}

int settings(int argc, char *argv[], char *envp[]) {
	string outputFormat("rootrc");

	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:f:j")) != -1) {
		switch (opt) {
			case '?': { settings_printUsage(argv[0]); return 0; }
			case 'f': {
				log_debug("Setting output format to %s", optarg);
				outputFormat = string(optarg);
				break;
			}
			case 'j': {
				log_debug("Setting output format to JSON", optarg);
				log_info("Option \"-j\" is deprecated, use \"-f json\" instead");
				outputFormat = "json";
				break;
			}
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (argc - optind == 0) {
		writeSettings(Settings::global(), outputFormat);
		return 0;
	} else if (argc - optind >= 1) {
		Settings settings;
		while (optind < argc) {
			TString inFileName(argv[optind++]);
			log_debug("Using settings from (\"%s\")", inFileName.Data());
			settings.readAuto(inFileName);
		}
		writeSettings(settings, outputFormat);
		return 0;
	} else {
		settings_printUsage(argv[0]);
		return 1;
	}
}


void map_single_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] MAPPERS OUTPUT_FILE INPUT_FILE" << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Apply selectors / operators specified by MAPPERS to INPUT_FILE, writing output" << endl;
	cerr << "and current settings to OUTPUT_FILE." << endl;
}

int map_single(int argc, char *argv[], char *envp[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:")) != -1) {
		switch (opt) {
			case '?': { map_single_printUsage(argv[0]); return 0; }
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}


	if (argc - optind != 3) { map_single_printUsage(argv[0]); return 1;	}
	string mappers = argv[optind++];
	string outFileName = argv[optind++];
	string inFileName = argv[optind++];

	log_debug("FroastTools::mapSingle(\"%s\", \"%s\", \"%s\")", inFileName.c_str(), mappers.c_str(), outFileName.c_str());
	FroastTools::mapSingle(inFileName, mappers, outFileName);

	return 0;
}


void map_multi_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] MAPPERS TAG [INPUT]..." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Apply selectors / operators specified by MAPPERS to INPUTs one by one," << endl;
	cerr << "producing multiple output files. Output file names are generated from input" << endl;
	cerr << "file names by adding TAG. Input file names may contain wildcards." << endl;
}

int map_multi(int argc, char *argv[], char *envp[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:")) != -1) {
		switch (opt) {
			case '?': { map_multi_printUsage(argv[0]); return 0; }
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (argc - optind < 3) { map_multi_printUsage(argv[0]); return 1; }
	string mappers =argv[optind++];
	string tag = argv[optind++];
	bool firstInput = true;
	while (optind < argc) {
		// -> inputfilename(s), mappers, filename extension tag, bool noRecompile
		// By default the selector is compiled (++ ROOT compile option).
		// If there are more than one inputfile, the selector is not recompiled
		string input = argv[optind++];
		log_debug("FroastTools::mapMulti(\"%s\", \"%s\", \"%s\", %s)", input.c_str(), mappers.c_str(), tag.c_str(), !firstInput ? "true" : "false");
		FroastTools::mapMulti(input, mappers, tag, !firstInput);
		firstInput = false;
	}

	return 0;
}


void reduce_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] MAPPERS OUTPUT_FILE [INPUT]..." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Simmilar to map-single, but applies MAPPERS to one or more input files, writing" << endl;
	cerr << "writing output and settings to a single output file OUTPUT_FILE (unlike," << endl;
	cerr << "map-multi which produces one output file for each input file)." << endl;
}

int reduce(int argc, char *argv[], char *envp[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:")) != -1) {
		switch (opt) {
			case '?': { reduce_printUsage(argv[0]); return 0; }
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (argc - optind < 3) { reduce_printUsage(argv[0]); return 1; }
	string mappers = argv[optind++];
	string outFileName = argv[optind++];
	string inFiles = argv[optind++];
	while (optind < argc) {
		inFiles+=" ";
		inFiles+=argv[optind++];
	}
	log_debug("FroastTools::reduce(\"%s\", \"%s\", \"%s\")", inFiles.c_str(), mappers.c_str(), outFileName.c_str());
	FroastTools::reduce(inFiles, mappers, outFileName);
	return 0;
}


void tabulate_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] ROOT_FILE/TREENAME VAREXP [SELECTION [NENTRIES [STARTENTRY]]]" << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Applied the given expression on a tree in a ROOT file and writes the result." << endl;
	cerr << "to the standard output in text form. This is similar to TTree::Scan, but the" << endl;
	cerr << "output format is tab-separated-values (by default) or JSON (selected inside" << endl;
	cerr << "the expression)" << endl;
}

int tabulate(int argc, char *argv[], char *envp[]) {
	string outputFormat("rootrc");

	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:")) != -1) {
		switch (opt) {
			case '?': { tabulate_printUsage(argv[0]); return 0; }
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}


	if (argc - optind < 2) { tabulate_printUsage(argv[0]); return 1; }

	TString input(argv[optind++]);
	TString varexp(argv[optind++]);
	const TString selection = (optind < argc) ? argv[optind++] : "";
	ssize_t nEntries = (optind < argc) ? atol(argv[optind++]) : -1;
	ssize_t startEntry = (optind < argc) ? atol(argv[optind++]) : 0;

	TChain *chain = openTChain(input);

	log_debug("FroastTools::tabulate(\"%s\", cout, \"%s\", \"%s\", %li, %li)", chain->GetName(), varexp.Data(), selection.Data(), (long int)nEntries, (long int)startEntry);
	FroastTools::tabulate(chain, cout, varexp, selection, nEntries, startEntry);

	delete chain;
	
	return 0;
}


void filter_multi_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " [OPTIONS] TAG [INPUT]..." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-e EXPR     Filter expression (as in TTree::Draw and similar)" << endl;
	cerr << "-f IDX      Copy from entry IDX (default: 0)" << endl;
	cerr << "-n N        Copy until entry IDX + N (default: -1 = no limit)" << endl;
	cerr << "-c SETTINGS Load configuration/settings" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Copy TFiles with TTrees, optionally applying entry selection criteria." << endl;
	cerr << "If a filter expression is given, it is evaluated on the first input" << endl;
	cerr << "and the resulting entry selection then applied to all inputs." << endl;
}

int filter_multi(int argc, char *argv[], char *envp[]) {
	string selection;
	ssize_t nEntries = -1;
	ssize_t startEntry = 0;

	int opt = 0;
	while ((opt = getopt(argc, argv, "?e:f:n:c:l:")) != -1) {
		switch (opt) {
			case '?': { filter_multi_printUsage(argv[0]); return 0; }
			case 'e': {
				log_debug("Setting filter expression to %s", optarg);
				selection = string(optarg);
				break;
			}
			case 'f': {
				startEntry = atol(optarg);
				log_debug("Starting at entry %lli", (long long) startEntry);
				break;
			}
			case 'n': {
				nEntries = atol(optarg);
				log_debug("Processing %lli entries", (long long) nEntries);
				break;
			}
			case 'c': { handleOptionConfig(optarg); break; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (optind >= argc) { filter_multi_printUsage(argv[0]); return 1;	}
	string tag = argv[optind++];

	list<TString> inputs;
	while (optind < argc) inputs.push_back(TString(argv[optind++]));

	auto_ptr<TEventList> eventList;

	if (! selection.empty()) {
		if (inputs.empty()) throw invalid_argument("No input to evaluate filter expression on");
	 	TString firstFileName, firstTreeName;
	 	Util::splitTFileObjName(*inputs.begin(), firstFileName, firstTreeName);
		TFile *firstFile(new TFile(firstFileName, "read"));
		TTree *firstTree = dynamic_cast<TTree*>(firstFile->Get(firstTreeName));
		if (firstTree == 0) throw runtime_error("Can't open input TTree");
		log_debug("Generating event list");
		eventList = auto_ptr<TEventList>(FroastTools::genEventList(firstTree, "eventList", selection, nEntries, startEntry));
		log_debug("%lli events selected", (long long)eventList->GetN());
		delete firstFile;		
	}

	if (&*eventList != 0) FroastTools::filter(inputs, tag, &*eventList);
	else FroastTools::filter(inputs, tag, 0, nEntries, startEntry);

	return 0;
}


void entrylist_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " FILENAME ..." << endl;
	cerr << "" << endl;
	cerr << "Options:" << endl;
	cerr << "-?          Show help" << endl;
	cerr << "-l LEVEL    Set logging level (default: \"info\")" << endl;
	cerr << "" << endl;
	cerr << "Read event/entry list from files and output entry numbers." << endl;
}

int entrylist(int argc, char *argv[], char *envp[]) {
	int opt = 0;
	while ((opt = getopt(argc, argv, "?c:l:f:j")) != -1) {
		switch (opt) {
			case '?': { entrylist_printUsage(argv[0]); return 0; }
			case 'l': { handleOptionLogging(optarg); break; }
			default: throw invalid_argument("Unkown command line option");
		}
	}

	if (argc - optind >= 1) {
		TreeEntryList entries;
		while (optind < argc) {
			TString inFileName(argv[optind++]);
			log_debug("Reading entries from (\"%s\")", inFileName.Data());
			entries.clear();
			entries.readAuto(inFileName);
			entries.writeASCII(cout);
		}
		return 0;
	} else {
		entrylist_printUsage(argv[0]);
		return 1;
	}
}


void main_printUsage(const char* progName) {
	cerr << "Syntax: " << progName << " COMMAND ..." << endl << endl;
	cerr << "Commands: " << endl;
	cerr << "  settings" << endl;
	cerr << "  map-single" << endl;
	cerr << "  map-multi" << endl;
	cerr << "  reduce" << endl;
	cerr << "  filter-multi" << endl;
	cerr << "  tabulate" << endl;
	cerr << "  entrylist" << endl;
	cerr << "" << endl;
	cerr << "Use" << endl;
	cerr << "" << endl;
	cerr << "    " << progName << " COMMAND -?" << endl;
	cerr << "" << endl;
	cerr << "to get help for the individual commands." << endl;
}

int main(int argc, char *argv[], char *envp[]) {
	try {
		// Have to tell ROOT to load vector dlls, otherwise ROOT will produce
		// "is not of a class known to ROOT" errors on creation of STL vector
		// branches:
		gROOT->ProcessLine("#include <vector>");

		gSystem->SetProgname(PACKAGE_TARNAME);

		string progName(argv[0]);

		if (argc < 2) { main_printUsage(argv[0]); return 1; }

		string cmd(argv[1]);

		int cmd_argc = argc - 1;
		char **cmd_argv = argv + 1;

		if (cmd == "-?") { main_printUsage(argv[0]); return 0; }
		if (cmd == "settings") return settings(cmd_argc, cmd_argv, envp);
		else if (cmd == "map-single") return map_single(cmd_argc, cmd_argv, envp);
		else if (cmd == "map-multi") return map_multi(cmd_argc, cmd_argv, envp);
		else if (cmd == "reduce") return reduce(cmd_argc, cmd_argv, envp);
		else if (cmd == "filter-multi") return filter_multi(cmd_argc, cmd_argv, envp);
		else if (cmd == "tabulate") return tabulate(cmd_argc, cmd_argv, envp);
		else if (cmd == "entrylist") return entrylist(cmd_argc, cmd_argv, envp);
		else throw invalid_argument("Command not supported.");
	}
	catch(std::exception &e) {
		log_error("%s (%s)",e.what(), typeid(e).name());
		return 1;
	}

	log_info("Done");
}
