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

	///	@brief	Copy object from a ROOT directory
	///	@param	tdir	Directory where to copy from
	///	@param	objName	Name of the object to be copied
	///
	///	Implementation only supports trees for now
	static void copyObject(TDirectory *tdir, const TString &objName);

	///	@brief	Apply selector to TChain and write results to an output file
	///	@param	chain	Chain to which the selector should be applied
	///	@param	selector	Name of the selector
	///	@param	tag	Additional tag to be put to the outputfilename between file label and file extension
	///	@param	keep	Copy this Object (TTree) to the output file
	static void mapMulti(TChain *chain, const TString &selector, const TString &tag, const TString &keep = "");

	///	@brief	Apply mapper (selector or other option) to TTrees in one file and write results to an output file
	///	@param	inFileName	Name of the input file
	///	@param	mappers 		Name(s) of the selector(s) or option to draw or scan a TTree
	///	@param	outFileName	Name of the output file
	///	@param	noRecompile	Option to suppress forced recompilation of selector (by default a recompilation of the selector is forced)
	static void mapSingle(const TString &inFileName, const TString &mappers, const TString &outFileName, bool noRecompile = false);

	///	@brief	Apply mapper (selector or other option) to TTrees in several files and write results to an output file
	///	@param	fileName	Name of the input ROOT file
	///	@param	mappers	Name(s) of the selector(s) or option to draw or scan a TTree
	///	@param	tag	Additional tag to be put to the outputfilename between file label and file extension
	///	@param	noRecompile	Option to suppress forced recompilation of selector (by default a recompilation of the selector is forced)
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
	
	///	@brief	TTree Scan() Method like possibility to show information from TTrees
	///	@param	chain		TTree to be scanned / evaluated
	///	@param	varexp	Expression that shall be evaluated (formula. variable to print out etc.)
	///	@param	selection	Cut criteria to be applied
	///	@param	nEntries	Number of entries to be evaluated, choose -1 to evaluate all entries
	///	@param	startEntry	First entry to be evaluated
	static void tabulate(TTree *chain, ostream &out, const TString &varexp, const TString &selection = "", ssize_t nEntries = -1, ssize_t startEntry = 0);
};



} // namespace froast


#ifdef __CINT__
#pragma link C++ class froast::Selector-;
#endif

#endif // FROAST_SELECTOR_H
