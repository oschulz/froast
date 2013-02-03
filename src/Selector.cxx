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

#include <stdexcept>
#include <iostream>
#include <sstream>

#include <TFile.h>
#include <TObjArray.h>
#include <TChainElement.h>

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
		throw std::runtime_error(string("Object ") + objName.Data() + " not found in TDirectory");
	}
}


void Selector::map(TChain *chain, const TString &selector, const TString &tag, const TString &keep) {
	TObjArray *chainElems = chain->GetListOfFiles();
	for (int i = 0; i < chainElems->GetEntriesFast(); ++i) {
		TChainElement *e = dynamic_cast<TChainElement*>(chainElems->At(i));
		string treeName = e->GetName();
		string inFileName = e->GetTitle();

		string outFileName = (File(inFileName).base() % tag.Data()).path();

		cerr << "Mapping " << inFileName << ":" << treeName << " to " << outFileName << endl;

		TFile inFile(inFileName.c_str(), "read");
		TTree *inTree; inFile.GetObject(treeName.c_str(), inTree);
		
		TFile outFile(outFileName.c_str(), "recreate");

		TSelector *sel = TSelector::GetSelector(selector.Data());
		inTree->Process(sel);
		delete sel;

		TObjArray* keeps = keep.Tokenize(",");
		for (int i = 0; i < keeps->GetEntriesFast(); ++i) {
			TString keepObjName = dynamic_cast<TObjString*>(keeps->At(i))->GetString().Strip(TString::kBoth);
			TObject *keepObj = 0;
			inFile.GetObject(keepObjName.Data(), keepObj);
			if (keepObj) {
				cerr <<  "Copying object " << keepObjName << " to output" << endl;
				copyObject(&inFile, keepObjName);
			}
		}
		delete keeps;
		
		outFile.Write();
		outFile.Close();
		inFile.Close();
	}
}


void Selector::map(const TString &fileName, const TString &treeName, const TString &selector, const TString &tag, const TString &keep) {
	TChain chain(treeName.Data());
	chain.Add(fileName.Data());
	map(&chain, selector, tag, keep);
}


} // namespace froast
