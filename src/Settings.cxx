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
#include <set>
#include <sstream>
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <TObjString.h>
#include <TObjArray.h>
#include <TMap.h>
#include <TParameter.h>
#include <TPRegexp.h>

#include "util.h"
#include "JSON.h"


ClassImp(froast::Settings)

using namespace std;


namespace froast {


Param Param::operator()(int32_t idx) const {
	TPRegexp wildcardExpr("[*]");
	if (idx < 0) throw invalid_argument("Negative value passed as Param index");
	Param result(m_name);
	wildcardExpr.Substitute(result.m_name, TString::Format("%li", (long)idx), "", 0, 1);
	return result;
}

	
Param::Param() {}


Param::Param(const TString &name)
	: m_name(name) {}


Param::~Param() {}



Settings Settings::m_global(gEnv, false);
	

void Settings::getInstances(const TString &pattern, std::vector<int32_t> &instances) {
	set<int32_t> found;
	instances.clear();
	
	TString pre, post;
	Int_t wcPos = pattern.Index("*");
	if (wcPos >= 0) {
		pre = pattern(0, wcPos);
		post = pattern(wcPos + 1, pattern.Length() - wcPos - 1);
	} else { pre = pattern; }
	
	const THashList *settings = table();
	if (settings == 0) return;

	TIter next(settings, kIterForward);
	TEnvRec *record;
	while (record = dynamic_cast<TEnvRec*>(next())) {
		TString name(record->GetName());
		if (
			(name.Length() > pre.Length() + post.Length())
			&& (name(0, pre.Length()) == pre)
		) {
			if (post.Length() == 0) {
				TSubString rest = name(pre.Length(), name.Length() - pre.Length());
				Int_t idxEnd = 0;
				while ((idxEnd < rest.Length()) && isdigit(rest[idxEnd]) ) idxEnd++;
				if (idxEnd > 0) {
					int32_t idx = TString(TString(rest)(0, idxEnd)).Atoi();
					if (idx >= 0) found.insert(idx);
				}
			} else if (name(name.Length() - post.Length(), name.Length()) == post) {
				TString idxPart = name(pre.Length(), name.Length() - pre.Length() - post.Length());
				
				if (idxPart.IsDigit()) {
					int32_t idx = idxPart.Atoi();
					if (idx >= 0) found.insert(idx);
				}
			}
		}
	}
	
	instances.resize(found.size());
	copy(found.begin(), found.end(), instances.begin());
}


bool Settings::operator()(const char* name, bool dflt, bool saveDflt) {
	bool value = tenv()->GetValue(name, dflt) > 0;
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


int32_t Settings::operator()(const char* name, int32_t dflt, bool saveDflt) {
	int32_t value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


double Settings::operator()(const char* name, double dflt, bool saveDflt) {
	double value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}

const char* Settings::operator()(const char* name, const char* dflt, bool saveDflt) {
	const char* value = tenv()->GetValue(name, dflt);
	if (saveDflt && !tenv()->Defined(name)) tenv()->SetValue(name, value);
	return value;
}


THashList* Settings::exportNested(EEnvLevel minLevel) const {
	THashList *list = new THashList;

	const THashList *settings = table();
	if (settings == 0) return list;

	TIter next(settings, kIterForward);
	TEnvRec *record;
	while (record = dynamic_cast<TEnvRec*>(next())) {
		TString name(record->GetName());
		TString valueString = TString(record->GetValue()).Strip(TString::kBoth);
		const char *valueCString = valueString.Data();
		if (record->GetLevel() >= minLevel) {
			THashList *currentList = list;
			TObjArray* nameParts = name.Tokenize(".");
			int nameDepth =  nameParts->GetEntriesFast();
			for (int i = 0; i < nameDepth; ++i) {
				TObjString *key = dynamic_cast<TObjString*>(nameParts->At(i));
				assert(key != 0);
				TObject *obj = currentList->FindObject(key);
				TPair *pair = dynamic_cast<TPair*>(obj);
				if (i < nameDepth-1) {
					if (obj != 0) {
						assert(pair != 0);
						THashList *child = dynamic_cast<THashList*>(pair->Value());
						assert(child != 0);
						currentList = child;
					} else {
						THashList *child = new THashList;
						currentList->AddLast(new TPair(key->Clone(), child));
						currentList = child;
					}
				} else {
					TObject *value = 0;
					
					char *numEnd;
					double doubleValue = strtod(valueCString, &numEnd);
					
					if (valueString.Length() == 0) {
						value = 0;
					} else if (valueCString+strlen(valueCString) == numEnd) {
						int32_t intValue = int32_t(doubleValue);
						if ((double(intValue) != doubleValue) || valueString.Contains(".") || valueString.Contains("e") || valueString.Contains("E"))
							value = new TParameter<double>("", doubleValue);
						else value = new TParameter<int32_t>("", intValue);
					} else if ((valueString == "true") || (valueString == "false")) {
						value = new TObjString(valueString);
					} else {
						value = new TObjString(valueString);
					}
					currentList->AddLast(new TPair(key->Clone(), value));
				}
			}
			delete nameParts;		
		}
	}
	
	return list;
}


void Settings::importNested(const THashList *nested, EEnvLevel level, const TString &prefix) {
	int counter = 0;
	TIter next(nested, kIterForward);
	const TPair *member;
	while (member = dynamic_cast<const TPair*>(next())) {
		const TObjString *keyObj = dynamic_cast<const TObjString*>(member->Key());
		assert( keyObj != 0 );
		TString key = keyObj->GetString();
		TString fullKey = (prefix.Length() == 0) ? key : prefix + "." + key;
		const TObject *value = member->Value();
		
		if (dynamic_cast<const THashList*>(value)) {
			const THashList *obj = dynamic_cast<const THashList*>(value);
			importNested(obj, level, fullKey);
		} else if (dynamic_cast<const TObjString*>(value)) {
			const TObjString *s = dynamic_cast<const TObjString*>(value);
			tenv()->SetValue(fullKey, s->GetString(), level);
		} else {
			tenv()->SetValue(fullKey, JSON::toString(value).c_str(), level);
		}
	}
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
	if (settings == 0) return out;

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
	if (settings == 0) {
    if (this==&m_global)
      cerr << "No settings found in \"" << tdir->GetName() << "\"";
    else
      throw runtime_error(string("No settings found in \"") + tdir->GetName() + "\"");
    return;
  }
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
		TString name(record->GetName());
		if ((record->GetLevel() >= minLevel) && !name.Contains("Path"))
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


Settings::~Settings() {}


} // namespace froast
