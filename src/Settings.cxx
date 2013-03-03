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


#include "Settings.h"

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cassert>

#include "util.h"


ClassImp(froast::Settings)

using namespace std;


namespace froast {


Settings Settings::m_global(gEnv, false);
	
	
bool Settings::get(const char* name, bool dflt, bool saveDflt) {
	bool value = tenv()->GetValue(name, dflt) > 0;
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


int32_t Settings::get(const char* name, int32_t dflt, bool saveDflt) {
	int32_t value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


double Settings::get(const char* name, double dflt, bool saveDflt) {
	double value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}

const char* Settings::get(const char* name, const char* dflt, bool saveDflt) {
	const char* value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


void Settings::write(const TString &fileName, EEnvLevel minLevel) {
	ofstream out(fileName.Data());
	write(out, minLevel);
	out.close();
}


void Settings::read(const TString &fileName, EEnvLevel level) {
	tenv()->ReadFile(fileName.Data(), level);
}


std::ostream& Settings::write(std::ostream &out, EEnvLevel minLevel) {
	THashList *settings = table();
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while (record = dynamic_cast<TEnvRec*>(next())) {
		if (record->GetLevel() >= minLevel)
			out << TString::Format("%s: %s\n", record->GetName(), record->GetValue()).Data();
	}
	return out;
}


void Settings::read(TDirectory *tdir, const TString &name) {
	THashList *settings; tdir->GetObject(name.Data(), settings);
	if (settings == 0) throw runtime_error(string("No settings found in \"") + tdir->GetName() + "\"");
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while (record = dynamic_cast<TEnvRec*>(next()))
		tenv()->SetValue(record->GetName(), record->GetValue(), record->GetLevel());
}


void Settings::writeToGDirectory(const TString &name, EEnvLevel minLevel) {
	THashList settingsOut;
	settingsOut.SetName(name.Data());
	
	THashList *settings = table();
	assert (settings != 0);
	TIter next(settings, kIterForward);
	TEnvRec *record;
	while (record = dynamic_cast<TEnvRec*>(next())) {
		if ((record->GetLevel() >= kEnvLocal) && !TString(record->GetName()).Contains("Path"))
			settingsOut.AddLast(record->Clone());
	}
	settingsOut.Write(name.Data(), TObject::kSingleKey);
}


std::string Settings::toString() {
	stringstream out;
	write(out);
	return out.str();
}


void Settings::clear() {
	table()->Clear();
}


Settings::Settings(TEnv* env, bool own)
	: m_env(env), m_envOwned(own) {}


Settings::Settings()
	: m_env(new TEnv), m_envOwned(true) {}


} // namespace froast
