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


#include "File.h"


using namespace std;


namespace froast {


std::string File::basename() const
	{ return string(gSystem->BaseName(m_path.c_str())); }

std::string File::dirname() const
	{ return string(gSystem->DirName(m_path.c_str())); }

std::string File::label() const {
	string base = basename();
	size_t pos = base.rfind('.');
	if (pos > base.size()) return base;
	else return base.substr(0, pos);
}

std::string File::extension() const {
	string base = basename();
	size_t pos = base.rfind('.');
	if (pos > base.size()) return "";
	else return base.substr(pos+1);
}


File File::operator/(const std::string &part) const {
	const char *cs = gSystem->ConcatFileName(m_path.c_str(), part.c_str());
	string s(cs); delete cs; return File(s);
}


File File::operator+(const std::string &part) const {
	return File(m_path + part);
}


File File::operator%(const std::string &tag) const {
	string d = dirname();
	string l = label();
	string e = extension();
	return File(d) / (l + tag + (e.empty() ? "" : "." + e));
}

File File::currentDir()
	{ return File(gSystem->WorkingDirectory()); }

File File::homeDir()
	{ return File(gSystem->HomeDirectory()); }


} // namespace froast
