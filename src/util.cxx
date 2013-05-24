// Copyright (C) 2011 Oliver Schulz <oliver.schulz@tu-dortmund.de>

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


#include "util.h"

#include <TObjString.h>
#include <TObjArray.h>


using namespace std;


namespace froast {


void Util::copy(const TCollection *from, std::vector<TString> &to)
	{ copy(from, to, TString::EStripType(-1)); }


void Util::copy(const TCollection *from, std::vector<TString> &to, TString::EStripType strip) {
	int n = from->GetEntries();
	to.resize(n);
	TIter next(from, kIterForward);
	TObject *entry;
	for (int i = 0; (entry = next()); ++i) {
		if (entry != 0) {
			const TObjString *s = dynamic_cast<TObjString*>(entry);
			if (s == 0) throw invalid_argument("TObjArray contains an element which is not of expected type TString.");
			if (int(strip) >= 0) to[i] = s->GetString().Strip(TString::kBoth); // strip blanks at beginning and / or end of string
			else to[i] = s->GetString();
		} else {
			to[i] = TString("");
		}
	}
}


void Util::split(const TString &s, const TString &sep, std::vector<TString> &parts) {
	TObjArray* result = s.Tokenize(sep);
	copy(result, parts);
	delete result;
}

void Util::split(const TString &s, const TString &sep, std::vector<TString> &parts, TString::EStripType strip) {
	TObjArray* result = s.Tokenize(sep);
	copy(result, parts, strip);
	delete result;
}


void Util::match(const TString &s, TPRegexp &expr, std::vector<TString> &matches) {
	TObjArray* result = expr.MatchS(s);
	copy(result, matches);
	delete result;
}

void Util::match(const TString &s, TPRegexp &expr, std::vector<TString> &matches, TString::EStripType strip) {
	TObjArray* result = expr.MatchS(s);
	copy(result, matches, strip);
	delete result;
}


} // namespace froast
