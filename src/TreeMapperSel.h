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


#ifndef FROAST_TREEMAPPERSEL_H
#define FROAST_TREEMAPPERSEL_H

#include <TChain.h>
#include <TFile.h>
#include <TSelector.h>

#include "BranchManager.h"
#include "logging.h"


namespace froast {


class TreeMapperSel : public TSelector {
protected:
	// Internal

	Long64_t m_logCounter;

	// Settings

	LogLevel sel_log_normal_level;
	LogLevel sel_log_increased_level;
	Int_t sel_log_increased_every;

	Int_t output_level;

	// Input

	froast::InputBranchManager inputManager;

	TTree *inputTree;
	TFile *inputFile;

	// Output

	froast::OutputBranchManager outputManager;

	TTree *outputTree;

public:
	TreeMapperSel(TTree *tree = 0);
	virtual ~TreeMapperSel() { }

	virtual Int_t	Version() const { return 2; }
	virtual Int_t	GetEntry(Long64_t entry, Int_t getall = 0);

	virtual void	SlaveBegin(TTree *tree);
	virtual void	Init(TTree *tree);
	virtual Bool_t  Process(Long64_t entry);
	virtual void	SlaveTerminate();

	virtual Bool_t  ProcessEntry(Long64_t entry) = 0;

	ClassDef(TreeMapperSel, 0);
};


} // namespace froast

#endif // FROAST_TREEMAPPERSEL_H
