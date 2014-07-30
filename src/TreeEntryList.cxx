// Copyright (C) 2014 Oliver Schulz <oliver.schulz@tu-dortmund.de>

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


#include "TreeEntryList.h"

#include <string>
#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <TFile.h>

#include "util.h"


using namespace std;


namespace froast {


void TreeEntryList::readASCII(std::istream &in) {
	Long64_t idx;
	while (in >> idx) m_eventList->Enter(idx);
}


std::ostream& TreeEntryList::writeASCII(std::ostream &out) const {
	size_t n = m_eventList->GetN();
	for (size_t i = 0; i < n; ++i)
		out << m_eventList->GetEntry(i) << "\n";
	return out;
}


void TreeEntryList::readFromTDirectory(TDirectory *tdir, const TString &name) {
	TEventList *eventListIn; tdir->GetObject(name.Data(), eventListIn);
	if (eventListIn == 0)
		throw runtime_error(string("No event list found in \"") + tdir->GetName() + "\"");
	m_eventList->Add(eventListIn);
}


void TreeEntryList::writeToGDirectory(const TString &name) const {
	TEventList eventListOut(*m_eventList);
	eventListOut.Write(name.Data(), TObject::kSingleKey);
}


void TreeEntryList::readAuto(const TString &fileName) {
	if (Util::isTFileObjName(fileName)) {
		TString rootFileName, objectName;
		Util::splitTFileObjName(fileName, rootFileName, objectName);
		TFile inFile(rootFileName.Data(), "read");
		if (objectName.Length() > 0) readFromTDirectory(&inFile, objectName.Data());
		else readFromTDirectory(&inFile);
	} else if (fileName == "-") {
		readASCII(cin);
	} else { // Assume flat ASCII file
		ifstream in(fileName.Data());
		readASCII(in);
		in.close();
	}
}


void TreeEntryList::writeAuto(const TString &fileName) {
	if (Util::isTFileObjName(fileName)) {
		TString rootFileName, objectName;
		Util::splitTFileObjName(fileName, rootFileName, objectName);
		TFile inFile(rootFileName.Data(), "recreate");
		if (objectName.Length() > 0) writeToGDirectory(objectName);
		else writeToGDirectory();
	} else if (fileName == "-") {
		writeASCII(cout);
	} else { // Assume flat ASCII file
		ofstream out(fileName.Data());
		writeASCII(out);
		out.close();
	}
}


void TreeEntryList::clear() {
	m_eventList->Clear();
}


TreeEntryList::TreeEntryList(TEventList* eventList, bool own)
	: m_eventList(eventList), m_wrappedOwned(own) {}


TreeEntryList::TreeEntryList()
	: m_eventList(new TEventList), m_wrappedOwned(true) {}


TreeEntryList::~TreeEntryList() {
	if (m_wrappedOwned && (m_eventList !=0)) delete m_eventList;
}


} // namespace froast
