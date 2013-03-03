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
#include <TDirectory.h>


using namespace std;


namespace froast {


bool Settings::get(const char* name, bool dflt, bool saveDflt) {
	bool value = gEnv->GetValue(name, dflt) > 0;
	if (saveDflt && !gEnv->Defined(name)) gEnv->SetValue(name, value);
	return value;
}


int32_t Settings::get(const char* name, int32_t dflt, bool saveDflt) {
	int32_t value = gEnv->GetValue(name, dflt);
	if (saveDflt && !gEnv->Defined(name)) gEnv->SetValue(name, value);
	return value;
}


double Settings::get(const char* name, double dflt, bool saveDflt) {
	double value = gEnv->GetValue(name, dflt);
	if (saveDflt && !gEnv->Defined(name)) gEnv->SetValue(name, value);
	return value;
}

const char* Settings::get(const char* name, const char* dflt, bool saveDflt) {
	const char* value = gEnv->GetValue(name, dflt);
	if (saveDflt && !gEnv->Defined(name)) gEnv->SetValue(name, value);
	return value;
}


std::ostream& Settings::write(THashList *settings, std::ostream &out) {
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while ((record = (TEnvRec*) next())) {
		if (record->GetLevel() >= kEnvLocal)
			out << TString::Format("%s: %s\n", record->GetName(), record->GetValue()).Data();
	}
	return out;
}


std::ostream& Settings::write(std::ostream &out) {
	THashList *settings = getCurrent();
	return write(settings, out);
}
	

void Settings::writeToGDirectory() {
	THashList settingsOut;
	settingsOut.SetName("settings");
	
	THashList *settings = getCurrent();
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while ((record = (TEnvRec*) next())) {
		if (record->GetLevel() >= kEnvLocal)
			settingsOut.AddLast(record->Clone());
	}
	settingsOut.Write("settings", TObject::kSingleKey);
}


THashList* Settings::getFrom(TDirectory *tdir) {
	THashList *settings; tdir->GetObject("settings", settings);
	if (settings == 0) throw runtime_error(string("No settings found in \"") + tdir->GetName() + "\"");
	return settings;
}


THashList* Settings::getCurrent() {
	return gEnv->GetTable();
}


std::string Settings::toString() {
	stringstream out;
	write(out);
	return out.str();
}


} // namespace froast
