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

///	@brief	Class to manage file names
	
class File {
	std::string m_path;

public:
	///	@brief	Returns path of the file.
	std::string path() { return m_path; }
	///	@brief	Returns path of the file.
	std::string toString() { return m_path; }
	///	@brief	Returns basename of the file. 
	/**	Preceding paths will be stripped.*/
	std::string basename() const;
	///	@brief	Returns directory part of the filename
	std::string dirname() const;
	///	@brief	Returns file name stripped from the file extension. 
	/**	Returns full file name if no extension was recognised. */
	std::string label() const;
	///	@brief	Returns file extension
	std::string extension() const;
	
	///	@brief	Returns File initialised with the directory name of the current file
	File dir() const { return File(dirname()); }
	///	@brief	Returns File initialised with the basename of the current file
	File base() const { return File(basename()); }
	
	///	@brief	Concatenate a File with another one
	/**	File("/root/data") / File("filename.root") -> /root/data/filename.root */
	File operator/(const std::string &part) const;
	///	@brief	Combine two Files
	File operator+(const std::string &part) const;
	///	@brief	Insert tag between label and extension of File
	File operator%(const std::string &tag) const;
	///	@brief	Cast to std::string
	operator std::string() const { return m_path; }
	
	///	@brief	Returns current directory
	static File currentDir();
	///	@brief	Returns the users home directory
	static File homeDir();

	///	@brief	Constructor
	File(const std::string &filePath) : m_path(filePath) {}
	virtual ~File() {}
};


} // namespace froast


#ifdef __CINT__
#pragma link C++ class froast::File-;
#endif

#endif // FROAST_FILE_H
