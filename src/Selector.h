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


#ifndef FROAST_SELECTOR_H
#define FROAST_SELECTOR_H

#include <iostream>

#include <Rtypes.h>
#include <TString.h>
#include <TFile.h>
#include <TChain.h>
#include <TSelector.h>

#include "File.h"


namespace froast {


class Selector {
public:
	// Implementation only supports trees for now
	static void copyObject(TDirectory *tdir, const TString &objName);

	static void mapMulti(TChain *chain, const TString &selector, const TString &tag, const TString &keep = "");

	static void mapSingle(const TString &inFileName, const TString &mappers, const TString &outFileName, bool noRecompile = false);

	static void mapMulti(const TString &fileName, const TString &mappers, const TString &tag, bool noRecompile = false);

	// JSON output format
	//
	// With column names:
	//     {"rows":[
	//     {COLNAME:VALUE,...},
	//     ...
	//     ]}
	//
	// Without column names:
	//     {"rows":[
	//     [VALUE,...],
	//     ...
	//     ]}
	static void tabulate(TTree *chain, ostream &out, const TString &varexp, const TString &selection = "", ssize_t nEntries = -1, ssize_t startEntry = 0);
};



} // namespace froast


#ifdef __CINT__
#pragma link C++ class froast::Selector-;
#endif

#endif // FROAST_SELECTOR_H
