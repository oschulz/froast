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


#ifndef FROAST_FILE_H
#define FROAST_FILE_H

#include <string>

#include <TSystem.h>


namespace froast {


class File {
	std::string m_path;

public:
	std::string path() { return m_path; }
	std::string toString() { return m_path; }
	std::string basename() const;
	std::string dirname() const;
	std::string label() const;
	std::string extension() const;
	
	File dir() const { return File(dirname()); }
	File base() const { return File(basename()); }
	
	File operator/(const std::string &part) const;
	File operator+(const std::string &part) const;
	File operator%(const std::string &tag) const;
	
	operator std::string() const { return m_path; }
	
	static File currentDir();
	static File homeDir();

	File(const std::string &filePath) : m_path(filePath) {}
	virtual ~File() {}
};


} // namespace froast


#ifdef __CINT__
#pragma link C++ class froast::File-;
#endif

#endif // FROAST_FILE_H
