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
#include <sstream>
#include <cassert>
#include <cassert>

#include <TFile.h>
#include <TObjArray.h>
#include <TChainElement.h>
#include <TPRegexp.h>

#include "util.h"
#include "File.h"


ClassImp(froast::Selector)

using namespace std;


using namespace std;


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
	TObjArray *chainElems = chain->GetListOfFiles();
	for (int chainEntry = 0; chainEntry < chainElems->GetEntriesFast(); ++chainEntry) {
		TChainElement *e = dynamic_cast<TChainElement*>(chainElems->At(chainEntry));
		string treeName = e->GetName();
		string inFileName = e->GetTitle();

		string outFileName = (File(inFileName).base() % tag.Data()).path();
		cerr << "Mapping " << inFileName << ":" << treeName << " to " << outFileName << endl;

		TFile inFile(inFileName.c_str(), "read");
		TTree *inTree; inFile.GetObject(treeName.c_str(), inTree);
		
		TFile outFile(outFileName.c_str(), "recreate");

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
		
		Settings::writeToGDirectory();
		outFile.Write();
		outFile.Close();
		inFile.Close();
	}
}


void Selector::mapSingle(const TString &inFileName, const TString &mappers, const TString &outFileName, bool noRecompile) {
	TFile inFile(inFileName.Data(), "read");
	TFile outFile(outFileName.Data(), "recreate");

	TPRegexp mapperSpecExpr("^(.*)\\((.*)\\)$");
	TPRegexp xxExp("\\+\\+$");

	TObjArray* mapperNames = mappers.Tokenize(",");
	for (int mapperEntry = 0; mapperEntry < mapperNames->GetEntriesFast(); ++mapperEntry) {
		TString mapper = dynamic_cast<TObjString*>(mapperNames->At(mapperEntry))->GetString().Strip(TString::kBoth);
		TObjArray* matches = mapperSpecExpr.MatchS(mapper);
		if (matches->GetEntriesFast() == 3) {
			TString fctName = dynamic_cast<TObjString*>(matches->At(1))->GetString();
			if (noRecompile) xxExp.Substitute(fctName, "+"); 
			TString objName = dynamic_cast<TObjString*>(matches->At(2))->GetString();
			assert ((fctName != 0) && (objName != 0));
			delete matches;
			
			cerr << "Applying " << fctName << "(" << objName << ")" << endl;
			
			TObject *inObj; inFile.GetObject(objName.Data(), inObj);
			if (inObj != 0) {
				TTree *inTree = dynamic_cast<TTree*>(inObj);
				if (inTree != 0) {
					if (fctName == "copy") {
						TTree *copied = inTree->CloneTree();
						copied->Write();
					} else {
						TSelector *sel = TSelector::GetSelector(fctName.Data());
						if (sel == 0) throw runtime_error(string("Cannot load selector ") + fctName.Data());
						inTree->Process(sel);
						delete sel;
					}
				} else throw invalid_argument(string("Objects of type ") + inObj->Class()->GetName() + " not supported yet");
			} else {
				delete matches;
				throw runtime_error(string("Object ") + objName.Data() + " not found in TDirectory");
			}
		} else {
			delete matches;
			throw invalid_argument(string("Invalid mapper specification: \"") + mapper.Data() + "\"");
		}
	}
	delete mapperNames;
	
	Settings::writeToGDirectory();
	outFile.Write();
	outFile.Close();
	inFile.Close();
}


void Selector::mapMulti(const TString &fileName, const TString &mappers, const TString &tag, bool noRecompile) {
	cerr << TString::Format("Selector::map(%s, %s, %s)", fileName.Data(), mappers.Data(), tag.Data()) << endl;

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


} // namespace froast
