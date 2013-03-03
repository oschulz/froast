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


#include "util.h"

#include <sstream>
#include <cassert>

#include <TEnv.h>
#include <THashList.h>


using namespace std;


namespace froast {


void Settings::write(std::ostream &out) {
	THashList *settings = gEnv->GetTable();
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while ((record = (TEnvRec*) next()))
		if (record->GetLevel() >= kEnvLocal)
			out << TString::Format("%s: %s\n", record->GetName(), record->GetValue()).Data();
}


void Settings::writeToGDirectory() {
	THashList settingsOut;
	settingsOut.SetName("settings");
	
	THashList *settings = gEnv->GetTable();
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while ((record = (TEnvRec*) next())) {
		if (record->GetLevel() >= kEnvLocal)
			settingsOut.AddLast(record);
	}
	settingsOut.Write("settings");
}


std::string Settings::toString() {
	stringstream out;
	write(out);
	return out.str();
}


} // namespace froast
