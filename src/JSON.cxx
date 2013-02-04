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


#include "JSON.h"


#include <iomanip>
#include <limits>
#include <sstream>
#include <cstdlib>
#include <cassert>
#include <stdint.h>

#include <TList.h>
#include <TMap.h>
#include <TObjArray.h>
#include <TOrdCollection.h>
#include <TObjString.h>
#include <TParameter.h>

#include "vjson.h"


using namespace std;


namespace {


using namespace froast;

TObject* importJSON(json_value* value) {
	switch(value->type) {
		case JSON_NULL: {
			return 0;
		}
		case JSON_OBJECT: {
			THashList *list = new THashList;
			for (json_value *it = value->first_child; it; it = it->next_sibling) {
				assert (it->name);
				list->AddLast(new TPair(new TObjString(it->name), importJSON(it)));
			}
			return list;
		}
		case JSON_ARRAY: {
			TList *list = new TList;
			for (json_value *it = value->first_child; it; it = it->next_sibling) {
				assert (!it->name);
				list->AddLast(importJSON(it));
			}
			return list;
		}
		case JSON_STRING: {
			return new TObjString(value->string_value);
		}
		case JSON_INT: {
			return new TParameter<int32_t>("", value->int_value);
		}
		case JSON_FLOAT: {
			return new TParameter<double>("", value->float_value);
		}
		case JSON_BOOL: {
			return new TObjString(value->int_value ? "true" : "false");
		}
		default: assert(false);
	}
}


void exportJSON(ostream &json, const TObject* value) {
	if (value == 0) json << "null";
	else {
		if (
			dynamic_cast<const THashList*>(value)
			|| dynamic_cast<const THashTable*>(value)
		) {
			const TCollection* objectValue = dynamic_cast<const TCollection*>(value);
			int counter = 0;
			json << "{";
			TIter next(objectValue, kIterForward);
			const TPair *member;
			while (member = dynamic_cast<const TPair*>(next())) {
				if (counter > 0) json << ", ";
				assert(dynamic_cast<const TObjString*>(member->Key()));
				exportJSON(json, member->Key());
				json << ": ";
				exportJSON(json, member->Value());
				++counter;
			}
			json << "}";
		} else if (dynamic_cast<const TMap*>(value)) {
			const TMap* map = dynamic_cast<const TMap*>(value);
			exportJSON(json, map->GetTable());
		} else if (
			dynamic_cast<const TList*>(value)
			|| dynamic_cast<const TObjArray*>(value)
			|| dynamic_cast<const TOrdCollection*>(value)
		) {
			const TCollection* arrayValue = dynamic_cast<const TCollection*>(value);
			int counter = 0;
			json << "[";
			TIter next(arrayValue, kIterForward);
			TObject *member;
			while (member = next()) {
				if (counter > 0) json << ", ";
				exportJSON(json, member);
				++counter;
			}
			json << "]";
		} else if (dynamic_cast<const TObjString*>(value)) {
			const TObjString* stringValue = dynamic_cast<const TObjString*>(value);
			TString s = stringValue->GetString();
			if ((s == "true") || (s == "false")) json << s;
			else json << "\"" << s << "\"";
		} else if (dynamic_cast<const TParameter<int32_t>*>(value)) {
			const TParameter<int32_t>* param = dynamic_cast<const TParameter<int32_t>*>(value);
			int32_t x = param->GetVal();
			json << x;
		} else if (dynamic_cast<const TParameter<int64_t>*>(value)) {
			const TParameter<int64_t>* param = dynamic_cast<const TParameter<int64_t>*>(value);
			int64_t x = param->GetVal();
			json << x;
		} else if (dynamic_cast<const TParameter<float>*>(value)) {
			const TParameter<float>* param = dynamic_cast<const TParameter<float>*>(value);
			float x = param->GetVal();
			if (x == int32_t(x)) json << int32_t(x) << ".";
			else json << x;
		} else if (dynamic_cast<const TParameter<double>*>(value)) {
			const TParameter<double>* param = dynamic_cast<const TParameter<double>*>(value);
			double x = param->GetVal();
			if (x == int64_t(x)) json << int64_t(x) << ".";
			else json << x;
		} else assert(false);
	}
}


} // namespace



namespace froast {


THashList* JSON::read(const char* json) {
	char *src = ::strdup(json);
	char *errorPos = 0;
	char *errorDesc = 0;
	int errorLine = 0;
	block_allocator allocator(1 << 10); // 1 KB per block

	json_value *root = json_parse(src, &errorPos, &errorDesc, &errorLine, &allocator);

	THashList *result = dynamic_cast<THashList*>(importJSON(root));

	::free(src);
	return result;
}

THashList* JSON::read(istream &json) {
	stringstream in;
	in << json.rdbuf();
	return read(in.str().c_str());
}

THashList* JSON::read(const std::string &json) { return read(json.c_str()); }
THashList* JSON::read(const TString &json) { return read(json.Data()); }


std::string JSON::toString(const TObject* list) {
	stringstream json;
	write(json, list);
	return json.str();
}


ostream& JSON::write(ostream &json, const TObject* list) {
	json.unsetf(ios::fixed | ios::scientific);
	json << std::setprecision(std::numeric_limits<double>::digits10);
	exportJSON(json, list);
	return json;
}


} // namespace froast
