// Copyright (C) 2013 Oliver Schulz <oliver.schulz@tu-dortmund.de>

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


#include "TreeMapperSel.h"

#include <string>
#include <exception>
#include <cassert>

#include "logging.h"
#include "Settings.h"


using namespace std;
using namespace froast;


namespace froast {


TreeMapperSel::TreeMapperSel(TTree *tree) {
	// Internal

	m_logCounter = 0;

	// Settings

	log_info("Input tree: %llu", (unsigned long long) tree);

	sel_log_normal_level = string2LogLevel( GSettings::get("selector.logging.normal.level", logLevel2String( log_level() )) );
	sel_log_increased_level = string2LogLevel( GSettings::get("selector.logging.increased.level", logLevel2String( LogLevel(std::max(int(sel_log_normal_level) - 10, 0)) )) );
	sel_log_increased_every = GSettings::get("selector.logging.increased.every", 10000);

	output_level = 1;

	inputTree = 0;
	inputFile = 0;

	outputTree = 0;
}


Int_t TreeMapperSel::GetEntry(Long64_t entry, Int_t getall) {
	if (inputTree != 0) {
		TTree *tree = inputTree->GetTree();
		if (inputFile != tree->GetCurrentFile()) {
			inputFile = tree->GetCurrentFile();
			log_info("Selector: Processing next file/tree: %s/%s", inputFile->GetName(), tree->GetName());
			m_logCounter = 0;
		}
	}
	log_debug("TreeMapperSel::GetEntry(%llu) [log every %llu]", (unsigned long long) entry, (unsigned long long) sel_log_increased_every);
	if (inputTree != 0) return inputTree->GetTree()->GetEntry(entry, getall);
	else { assert(false); return 0; }
}


void TreeMapperSel::SlaveBegin(TTree *) {
	log_info("TreeMapperSel::SlaveBegin(TTree *)");
	TString option = GetOption();

	outputTree = new TTree("events", "Calibrated Events");
	log_info("Created output TTree(\"%s\", \"%s\")", outputTree->GetName(), outputTree->GetTitle());

	outputManager.outputTo(outputTree, output_level);
}


void TreeMapperSel::Init(TTree *tree) {
	log_info("TreeMapperSel::Init(TTree *)");
	if (!tree) return;

	inputTree = tree;
	inputTree->SetMakeClass(1);
	inputManager.inputFrom(tree);
}


Bool_t TreeMapperSel::Process(Long64_t entry) {
	TmpLogLevel tmpLog(m_logCounter++ % sel_log_increased_every == 0 ? sel_log_increased_level : sel_log_normal_level);

	// Clear data in output
	outputManager.clearData();

	log_debug("Processing entry %llu", (unsigned long long)(entry));

	// Load input entry
	GetEntry(entry);

	return ProcessEntry(entry);
}


void TreeMapperSel::SlaveTerminate() {
	log_info("TreeMapperSel::SlaveTerminate()");

	outputTree->Write();

	log_info("TreeMapperSel::SlaveTerminate() finished");
}


} // namespace froast
