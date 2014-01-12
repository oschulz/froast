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


#ifndef FROAST_JSON_H
#define FROAST_JSON_H

#include <string>
#include <iostream>

#include <THashList.h>
#include <TString.h>
#include <TParameter.h>


namespace froast {


class JSON {
public:
	static THashList* read(const char* json);
	static THashList* read(std::istream &json);
	static THashList* read(const std::string &json);
	static THashList* read(const TString &json);

	static std::string toString(const TObject* list);

	static std::ostream& write(std::ostream &json, const TObject* list);
};


} // namespace froast


#endif // FROAST_JSON_H
