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


#ifndef FROAST_UTIL_H
#define FROAST_UTIL_H

#include <stdexcept>
#include <string>
#include <vector>
#include <limits>

#include <TString.h>
#include <TCollection.h>
#include <TPRegexp.h>


namespace froast {



class Util {
public:
	inline static double floatNaN() { return std::numeric_limits<float>::quiet_NaN(); }
	inline static double doubleNaN() { return std::numeric_limits<double>::quiet_NaN(); }

	/// @brief Copies    entries of TCollection of TStrings to a std::vector of TString
	/// @param from     TCollection to be copied from
	/// @param to       std::string to be copied into
	static void copy(const TCollection *from, std::vector<TString> &to);
	/// @brief Copies entries of TCollection of TStrings to a std::vector of TString
	/// @param from     TCollection to be copied from
	/// @param to       std::string to be copied into
	/// @param strip    Whether to strip leading/trailing blanks
	static void copy(const TCollection *from, std::vector<TString> &to, TString::EStripType strip);

	/// @brief split    string into parts according to separation character and put them into std::vector of TString
	/// @param s        string to be split
	/// @param sep      Character/expression separating the parts of s
	/// @param parts    std::vector to write splitted parts into
	static void split(const TString &s, const TString &sep, std::vector<TString> &parts);
	/// @brief split string into parts according to separation character and put them into std::vector of TString
	/// @param s  string to be split
	/// @param sep  Character/expression separating the parts of s
	/// @param parts std::vector to write splitted parts into
	/// @param strip  Whether to strip leading/trailing blanks
	static void split(const TString &s, const TString &sep, std::vector<TString> &parts, TString::EStripType strip);

	/// @brief Returns std::vector matching regular expressions to TString
	/// @brief Match a string with a regular expression
	/// @param s        String to be matched
	/// @param expr     Regular expression
	/// @param groups   std::vector to copy matched groups into
	static void match(const TString &s, TPRegexp &expr, std::vector<TString> &groups);
	/// @brief Match a string with a regular expression
	/// @param s        String to be matched
	/// @param expr     Regular expression
	/// @param groups   std::vector to copy matched groups into
	/// @param strip    Strip leading and/or trailing blanks (see TString::EStripType)
	static void match(const TString &s, TPRegexp &expr, std::vector<TString> &groups, const TString::EStripType strip);
};


} // namespace froast


#endif // FROAST_UTIL_H
