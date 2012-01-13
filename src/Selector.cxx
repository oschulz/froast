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


#include "Selector.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cstdlib>
#include <cassert>

#include <TFile.h>
#include <TObjArray.h>
#include <TChainElement.h>
#include <TPRegexp.h>
#include <TTreeFormula.h>
#include <TTreeFormulaManager.h>
#include <TEntryList.h>

#include "util.h"
#include "File.h"
#include "Settings.h"


using namespace std;


namespace {

class TTreeCurrentFile : public TTreeFormula {
protected:
	virtual Bool_t IsString(Int_t oper) const { return true; }

public:
	virtual const char *EvalStringInstance(Int_t i = 0)
		{ return fTree->GetTree()->GetCurrentFile()->GetName(); }

	TTreeCurrentFile() : TTreeFormula() {}
	TTreeCurrentFile(const char *name, TTree *tree) : TTreeFormula(name, "1.", tree) {}
};

} // namespace



namespace froast {


void Selector::copyObject(TDirectory *tdir, const TString &objName) {
	TObject *keepObj = 0;
	tdir->GetObject(objName.Data(), keepObj);
	if (keepObj != 0) {
		TTree *keepTree = dynamic_cast<TTree*>(keepObj);
		if (keepTree != 0) {
			TTree *cloned = keepTree->CloneTree();
			cloned->Write();
		}
	} else {
		throw runtime_error(string("Object ") + objName.Data() + " not found in TDirectory");
	}
}


void Selector::mapMulti(TChain *chain, const TString &selector, const TString &tag, const TString &keep) {
	
	///	For the syntax of the selector expression see masSingle

	TObjArray *chainElems = chain->GetListOfFiles();
	for (int chainEntry = 0; chainEntry < chainElems->GetEntriesFast(); ++chainEntry) {
		TChainElement *e = dynamic_cast<TChainElement*>(chainElems->At(chainEntry));
		string treeName = e->GetName();
		string inFileName = e->GetTitle();

		///	The name of the output files  is created from the filenames
		///	of the TTrees concatenated in the TChain.
		string outFileName = (File(inFileName).base() % tag.Data()).path();
		cerr << "Mapping " << inFileName << ":" << treeName << " to " << outFileName << endl;

		TFile inFile(inFileName.c_str(), "read");
		TTree *inTree; inFile.GetObject(treeName.c_str(), inTree);
		
		TFile outFile(outFileName.c_str(), "recreate");
		outFile.SetCompressionLevel(GSettings::get("froast.tfile.compression.level", 1));

		TSelector *sel = TSelector::GetSelector(selector.Data());
		if (sel == 0) throw runtime_error(string("Cannot load selector ") + selector.Data());
		inTree->Process(sel);
		delete sel;

		TObjArray* keeps = keep.Tokenize(",");
		for (int i = 0; i < keeps->GetEntriesFast(); ++i) {
			TString keepObjName = dynamic_cast<TObjString*>(keeps->At(i))->GetString().Strip(TString::kBoth);
			cerr << "Copying object " << keepObjName << " to output" << endl;
			copyObject(&inFile, keepObjName);
		}
		delete keeps;
		
		Settings::global().writeToGDirectory();
		outFile.Write();
		outFile.Close();
		inFile.Close();
	}
}


void Selector::mapSingle(const TString &inFileName, const TString &mappers, const TString &outFileName, bool noRecompile) {
	TFile inFile(inFileName.Data(), "read");
	TFile outFile(outFileName.Data(), "recreate");
	outFile.SetCompressionLevel(GSettings().get("froast.tfile.compression.level", 1));

	TPRegexp mapperSpecExpr("^([^(]*)\\((.*)\\)$");
	TPRegexp xxExp("\\+\\+$"); // selecor compile option

	vector<TString> mapperSpecs; // mapper expressions
	/// Mappers (selectors, draw/scan options) are separated by ";"
	Util::split(mappers, ";", mapperSpecs, TString::kBoth);
	
	for (int m = 0; m < mapperSpecs.size(); ++m) {
		
		vector<TString> mapperFctArgs; 
		Util::match(mapperSpecs[m], mapperSpecExpr, mapperFctArgs, TString::kBoth);
		// mapperFctArgs[0]: Full string
		// mapperFctArgs[1]: selector name
		// mapperFctArgs[2]: name of TTree(s) / TChain(s)
		if (mapperFctArgs.size() != 3) throw invalid_argument(string("Invalid mapper specification: \"") + mapperSpecs[m].Data() + "\"");
		TString fctName = mapperFctArgs[1];
		if (noRecompile) xxExp.Substitute(fctName, "+"); 
		///	Arguments for the mappers (given in brakets after mapper name) are separated by "," character.
		vector<TString> fctArgs; Util::split(mapperFctArgs[2], ",", fctArgs, TString::kBoth);

		if (fctArgs.size() < 1) throw invalid_argument(string("Invalid number of parameters for operation ") + fctName.Data() + ", expecting at least one.");
		TString objName = fctArgs[0]; // name of the TTree to be read in
		
		cerr << "Applying " << fctName << "(";
		for (size_t i = 0; i < fctArgs.size(); ++i) cerr << (i>0 ? "," : "") << fctArgs[i];
		cerr << ")" << endl;
		
		TObject *inObj; inFile.GetObject(objName.Data(), inObj);
		if (inObj == 0) throw runtime_error(string("Object ") + objName.Data() + " not found in TDirectory");

		TTree *inTree = dynamic_cast<TTree*>(inObj);
		inTree->ResetBranchAddresses(); // in case they have been in use previously
		if (inTree != 0) {
			if (fctName == "copy") {
				///	When choosing the "copy" argument/mapper the selected TTree (or subbranches) will be
				///	copied to a new file. Up to 5 arguments are allowed, example \n
				///	copy(tree, branches, selection, nentries, fistentry)
				if (fctArgs.size() <= 1) {
					TTree *copied = inTree->CloneTree();
					copied->Write();
				} else {
					///	The ordering of the arguments to the mapper are expected to be
					///	<ol>
					///	<li> Name of the TTree to be processed
					///	<li> Specifications of branches (optional)
					TString branchSpec = (fctArgs.size() > 1) ? fctArgs[1] : TString("");
					///	<li> A selection expression (optional). By default all branches will be enabled.
					TString selection = (fctArgs.size() > 2) ? fctArgs[2] : TString("");
					///	<li> Number of entries to process (optional)
					Long64_t nEntries = (fctArgs.size() > 3) ? atol(fctArgs[3]) : numeric_limits<Long64_t>::max();
					///	<li> Startnumber of the first entry to be processed (optional) </ol>
					Long64_t startEntry = (fctArgs.size() > 4) ? atol(fctArgs[4]) : 0;
					if (fctArgs.size() > 5) throw invalid_argument(string("Invalid number of parameters for operation ") + fctName.Data() + ", expecting 1 to 5.");

					TPRegexp branchSpecExpr("^(([^>]|>[^>])*)?\\s*(>>\\s*(\\w+))?$");
					vector<TString> branchSpecParts;
					Util::match(branchSpec, branchSpecExpr, branchSpecParts, TString::kBoth);

					vector<TString> branches;
					///	The branch specifications are separated by ":" sign
					if (branchSpecParts.size() > 1) Util::split(branchSpecParts[1], ":", branches, TString::kBoth);
					TString outTreeName = (branchSpecParts.size() > 4) ? branchSpecParts[4] : TString(inTree->GetName());

					for (size_t i = 0; i < branches.size(); ++i) if (branches[i].Length() > 0) {
						TString &b = branches[i];
						if (b[0] == '^') {
							if (i == 0) {
								cerr << "Enabling all branches" << endl;
								inTree->SetBranchStatus("*", 1);
							}
							TString bDis = b(1, b.Length()-1).Data();
							cerr << "Disabling branch \"" << bDis << "\"" << endl;
							inTree->SetBranchStatus(bDis.Data(), 0);
						} else {
							if (i == 0) {
								cerr << "Disabling all branches" << endl;
								inTree->SetBranchStatus("*", 0);
							}
							cerr << "Enabling branch \"" << b << "\"" << endl;
							inTree->SetBranchStatus(b.Data(), 1);
						}
					}
					
					TTree* outTree = inTree->CopyTree(selection.Data(), "", nEntries, startEntry);
					if (outTreeName != outTree->GetName()) outTree->SetName(outTreeName.Data());
					outTree->Write();
					inTree->SetBranchStatus("*", 1); // reactivate branches for later use
				}
			} else if (fctName == "draw") {
				///	With the "draw" argument it is possible to access the
				///	TTree draw method. The options are the same as for
				///	the TTree draw method: \n
				/// 	draw(tree, varexp, selection, option, nentries, fistentry) 
				///	<ol>
				///	<li> Name of the tree to draw from
				///	<li> Formula / expression / data to draw
				TString varexp = (fctArgs.size() > 1) ? fctArgs[1] : TString("");
				///	<li> Cut criteria / selection of entries (optional)
				TString selection = (fctArgs.size() > 2) ? fctArgs[2] : TString("");
				///	<li> Draw option(s) (optional)
				TString option = (fctArgs.size() > 3) ? fctArgs[3] : TString("");
				///	<li> Number of entries to process (optional)
				Long64_t nEntries = (fctArgs.size() > 4) ? atol(fctArgs[4]) : numeric_limits<Long64_t>::max();
				///	<li> First entry to process (optional) </ol>
				Long64_t startEntry = (fctArgs.size() > 5) ? atol(fctArgs[5]) : 0;
				if (fctArgs.size() > 6) throw invalid_argument(string("Invalid number of parameters for operation ") + fctName.Data() + ", expecting 1 to 6.");

				inTree->Draw(varexp.Data(), selection.Data(), (TString("goff ")+option).Data(), nEntries, startEntry);
			} else {
				///	If as argument / mapper the name of a selector is given the syntax is \n
				/// 	selector(tree, option, nentries, fistentry) \n
				///	The parameters are the typical parameters of a TTree call to a TSelector <ol>
				///	<li> Name of the TTree to process
				///	<li> option (?) (optional)
				TString option = (fctArgs.size() > 1) ? fctArgs[1] : TString("");
				///	<li> Number of entries to process (optional)
				Long64_t nEntries = (fctArgs.size() > 2) ? atol(fctArgs[2]) : numeric_limits<Long64_t>::max();
				///	<li> Number of first entry to process (optional) </ol>
				Long64_t startEntry = (fctArgs.size() > 3) ? atol(fctArgs[3]) : 0;
				if (fctArgs.size() > 4) throw invalid_argument(string("Invalid number of parameters for operation ") + fctName.Data() + ", expecting 1 to 4.");

				TSelector *sel = TSelector::GetSelector(fctName.Data());
				if (sel == 0) throw runtime_error(string("Cannot load selector ") + fctName.Data());
				inTree->Process(sel, option.Data(), nEntries, startEntry);
				delete sel;
			}
		} else throw invalid_argument(string("Objects of type ") + inObj->Class()->GetName() + " not supported yet");
	}
	
	Settings::global().writeToGDirectory();
	outFile.Write(0,TObject::kOverwrite);
	outFile.Close();
	inFile.Close();
}


void Selector::mapMulti(const TString &fileName, const TString &mappers, const TString &tag, bool noRecompile) {

	cerr << TString::Format("Selector::map(%s, %s, %s)", fileName.Data(), mappers.Data(), tag.Data()) << endl;
	
	///	For description of mappers see Selector::mapSingle

	TChain chain("");
	chain.Add(fileName.Data());
	TObjArray *chainElems = chain.GetListOfFiles();
	for (int chainEntry = 0; chainEntry < chainElems->GetEntriesFast(); ++chainEntry) {
		TChainElement *e = dynamic_cast<TChainElement*>(chainElems->At(chainEntry));
		string inFileName = e->GetTitle();

		string outFileName = (File(inFileName).base() % tag.Data()).path();
		cerr << "Mapping " << inFileName << " to " << outFileName << endl;
		// Don't recompile even if fct ends with "++" after first run:
		mapSingle(inFileName, mappers, outFileName, (chainEntry > 0) || noRecompile);
	}
	cerr << "Selector::map(...) finished" << endl;
}

void Selector::reduce(const TString &inFileNames, const TString &mappers, const TString &outFileName, bool noRecompile) {
	cerr << TString::Format("Selector::reduce(%s, %s, %s)", inFileNames.Data(), mappers.Data(), outFileName.Data()) << endl;
	cerr << "Selector::reduce(...) finished" << endl;
}


// Based in part on TTreePlayer::scan (Copyright (C) 1995-2000, Rene Brun
// and Fons Rademakers)
void Selector::tabulate(TTree *chain, ostream &out, const TString &varexp, const TString &selection, ssize_t nEntries, ssize_t startEntry) {
	cerr << TString::Format("Selector::tabulate(TChain*, ostream, \"%s\", \"%s\")", varexp.Data(), selection.Data()) << endl;

	ssize_t logEvery = GSettings::get("selector.log.every", 10000);

	const TString FS_TSV = "tsv";
	const TString FS_JSON = "json";
	
	out.unsetf(ios::fixed | ios::scientific);
	out << std::setprecision(std::numeric_limits<double>::digits10);

	TPRegexp varexpExpr("^(([^>]|>[^>])*)\\s*(>>\\s*(\\w+)\\s*(\\((.*)\\))?)?$");
	vector<TString> varexpParts;
	Util::match(varexp, varexpExpr, varexpParts, TString::kBoth);

	if (varexpParts.size() <= 1) throw invalid_argument("Invalid varexp");
	vector<TString> functions;
	Util::split(varexpParts[1], ":", functions, TString::kBoth);
	TString format = FS_TSV;
	if (varexpParts.size() > 4) format = varexpParts[4];
	vector<TString> labels;
	if (varexpParts.size() > 5) {
		if (varexpParts.size() > 6) Util::split(varexpParts[6], ":", labels, TString::kBoth);
		size_t nSpecLabels = labels.size();
		labels.resize(functions.size());
		for (size_t col = nSpecLabels; col < labels.size(); ++col) {
			labels[col] = functions[col];
			if (format == FS_JSON) {
				for (ssize_t i = 0; i < labels[col].Length(); ++i)
					if (labels[col](i) == '.') labels[col](i) = '$';
			}
		}
	}
	
	cerr << "Tabulation expression: ";
	for (size_t i = 0; i < functions.size(); ++i) cerr << (i>0 ? ":" : "") << functions[i];
	cerr << " >> " << format;
	if (labels.size() > 0) {
		cerr << "(";
		for (size_t i = 0; i < labels.size(); ++i) cerr << (i>0 ? ":" : "") << labels[i];
		cerr << ")";
	}
	cerr << endl;

	const size_t ncols = functions.size();

	TList tformulas;
	
	TTreeFormula *select  = 0;
	if (selection.Length() > 0) {
		select = new TTreeFormula("Selection", selection.Data(), chain);
		if (!select) throw invalid_argument("Invalid selection expression");
		if (!select->GetNdim()) { throw invalid_argument("Invalid selection expression"); }
		tformulas.Add(select);
	}
	
	vector<TTreeFormula*> colFormulas; colFormulas.reserve(ncols);
	for (size_t col = 0; col < ncols; ++col) {
		const TString &formula = functions[col];
		const TString name = TString::Format("col%lli", (long long)(col));
		if (formula == "File$") colFormulas.push_back(new TTreeCurrentFile(name.Data(), chain));
		else colFormulas.push_back(new TTreeFormula(name.Data(), formula.Data(), chain));
		tformulas.Add(colFormulas[col]);
	}

	vector<double> doubleValues(ncols);

	TTreeFormulaManager *manager=0;
	Bool_t hasArray = false;
	Bool_t forceDim = false;
	if (!tformulas.IsEmpty()) {
		if (select) {
			if (select->GetManager()->GetMultiplicity() > 0 ) {
				manager = new TTreeFormulaManager;
				for (int i=0; i <= tformulas.LastIndex(); ++i)
					manager->Add(dynamic_cast<TTreeFormula*>(tformulas.At(i)));
				manager->Sync();
			}
		}
		for (int i = 0; i <= tformulas.LastIndex(); ++i) {
			TTreeFormula *form = dynamic_cast<TTreeFormula*>(tformulas.At(i));
			switch( form->GetManager()->GetMultiplicity() ) {
				case  1: case  2: hasArray = true;
				case -1: forceDim = true;
			}
		}
	}

	if (format == FS_TSV) {
		if (!labels.empty()) {
			out << "# ";
			for (size_t i = 0; i < labels.size(); ++i) {
				if (i > 0) out << "\t";
				out << labels[i];
			}
			out << endl;
		}
	} else if (format == FS_JSON) {
		out << "{\"rows\":[" << endl;
	} else {
		throw invalid_argument(TString::Format("Unknown tabulation format \"%s\"", format.Data()).Data());
	}
	
	Int_t treeNumber = -1;
	ssize_t entry = startEntry;
	for (; (nEntries < 0) || entry < startEntry + nEntries; ++entry) {
		if (entry % logEvery == 0) cerr << "Tabulating entry " << entry << " [log every " << logEvery << "]" << endl;
		
		ssize_t entryNumber = chain->GetEntryNumber(entry);
		if (entryNumber < 0) break;
		ssize_t localEntry = chain->LoadTree(entryNumber);
		if (localEntry < 0) break;
		if (treeNumber != chain->GetTreeNumber()) {
			cerr << "Tabulating file \"" << chain->GetTree()->GetCurrentFile()->GetName() << "\"" << endl;
			treeNumber = chain->GetTreeNumber();
			if (manager) manager->UpdateFormulaLeaves();
			else for (ssize_t i = 0; i <= tformulas.LastIndex(); ++i) {
				dynamic_cast<TTreeFormula*>(tformulas.At(i))->UpdateFormulaLeaves();
			}
		}

		int ndata = 1;
		if (forceDim) {
			if (manager) ndata = manager->GetNdata(true);
			else {
				for (size_t col = 0; col < ncols; ++col) {
					ndata = std::max(ndata, colFormulas[col]->GetNdata());
				}
				if (select && select->GetNdata() == 0) ndata = 0;
			}
		}

		bool loaded = false;
		for (int inst = 0; inst < ndata; ++inst) {
			if ((select) && (select->EvalInstance(inst) == 0)) continue;
			if (inst==0) loaded = true;
			else if (!loaded) {
				// EvalInstance(0) always needs to be called so that
				// the proper branches are loaded.
				for (size_t col = 0; col < ncols; ++col) colFormulas[col]->EvalInstance(0);
				loaded = true;
			}
			if (format == FS_TSV) {
				for (size_t col = 0; col < ncols; ++col) {
					if (col > 0) out << "\t";
					bool isValid = (colFormulas[col]->GetNdim() > 0);
					if (colFormulas[col]->IsString()) {
						if (isValid) out << colFormulas[col]->EvalStringInstance(inst);
						else out << "null";
					} else {
						if (isValid) out << colFormulas[col]->EvalInstance(inst);
						else out << "NaN";
					}
				}
				out << endl;
			} else if (format == FS_JSON){
				if (entry > startEntry) out << "," << endl;
				out << (!labels.empty() ? "{" : (ncols > 1 ? "[" : ""));
				for (size_t col = 0; col < ncols; ++col) {
					if (col > 0) out << ",";
					if (!labels.empty()) out << "\"" << labels[col] << "\":";
					bool isValid = (colFormulas[col]->GetNdim() > 0);
					if (colFormulas[col]->IsString()) {
						if (isValid) out << "\"" << colFormulas[col]->EvalStringInstance(inst) << "\"";
						else out << "null";
					} else {
						if (isValid) out << colFormulas[col]->EvalInstance(inst);
						else out << "NaN";
					}
				}
				out << (!labels.empty() ? "}" : (ncols > 1 ? "]" : ""));
			}
		}
	}

	if (format == FS_JSON) {
		if (entry > startEntry) out << "," << endl;
		out << "]}" << endl;
	}
	
	tformulas.Clear();
}


} // namespace froast
