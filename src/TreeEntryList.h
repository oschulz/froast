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


#ifndef FROAST_TREEENTRYLIST_H
#define FROAST_TREEENTRYLIST_H

#include <iostream>
#include <vector>
#include <stdint.h>

#include <TString.h>
#include <TEventList.h>
#include <TDirectory.h>


namespace froast {


class TreeEntryList {
protected:
	TEventList *m_eventList;
	bool m_wrappedOwned;

public:
	bool contains(Long64_t entry) { return m_eventList->Contains(entry); }

	const TEventList* tevtList() const { return m_eventList; }
	TEventList* tevtList() { return m_eventList; }

	// Note: If input is not sorted, performance will be horrible
	void readASCII(std::istream &in);
	std::ostream& writeASCII(std::ostream &out) const;

	void readFromTDirectory(TDirectory *tdir, const TString &name = "eventList");
	void writeToGDirectory(const TString &name = "eventList") const;

	void readAuto(const TString &fileName);
	void writeAuto(const TString &fileName);

	void clear();

	TreeEntryList(TEventList* eventList, bool own = false);

	TreeEntryList();
	virtual ~TreeEntryList();
};


} // namespace froast


#endif // FROAST_TREEENTRYLIST_H
