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


#ifndef FROAST_SETTINGS_H
#define FROAST_SETTINGS_H

#include <iostream>
#include <vector>
#include <stdint.h>

#include <TString.h>
#include <TEnv.h>
#include <THashList.h>
#include <TDirectory.h>


namespace froast {


class Param {
protected:
	TString m_name;

public:
	operator const TString& () const { return m_name; }

	operator const char* () const { return m_name.Data(); }

	operator std::string () const { return std::string(m_name.Data()); }
	std::string str() const { return std::string(m_name.Data()); }

	Param operator()(const TString &name) const;
	Param operator()(int32_t idx) const;

	Param operator%(const TString &name) const;
	Param operator%(int32_t idx) const;
	
	Param();
	Param(const TString &name);
	virtual ~Param();
};


class Settings {
protected:
	static Settings m_global;

	TEnv *m_env;
	bool m_envOwned;
	
public:
	static Settings &global() { return m_global; }

	bool defined(const char* name);

	void getInstances(const TString &pattern, std::vector<int32_t> &instances) const;

	bool operator()(const char* name, bool dflt, bool saveDflt = true);
	int32_t operator()(const char* name, int32_t dflt, bool saveDflt = true);
	double operator()(const char* name, double dflt, bool saveDflt = true);
	const char* operator()(const char* name, const char* dflt, bool saveDflt = true);

	bool set(const char* name, bool value, EEnvLevel level = kEnvLocal);
	int32_t set(const char* name, int32_t value, EEnvLevel level = kEnvLocal);
	double set(const char* name, double value, EEnvLevel level = kEnvLocal);
	const char* set(const char* name, const char* value, EEnvLevel level = kEnvLocal);

	const TEnv* tenv() const { return m_env; }
	TEnv* tenv() { return m_env; }
	const THashList* table() const { return tenv()->GetTable(); }
	THashList* table() { return tenv()->GetTable(); }

	THashList* exportNested(EEnvLevel minLevel = kEnvLocal) const;
	void importNested(const THashList *nested, EEnvLevel level = kEnvLocal, const TString &prefix = "");

	void write(const TString &fileName, EEnvLevel minLevel = kEnvLocal) const;
	void read(const TString &fileName, EEnvLevel level = kEnvLocal);

	void writeJSON(std::ostream &out, EEnvLevel minLevel = kEnvLocal) const;
	void readJSON(std::istream &in, EEnvLevel level = kEnvLocal);

	std::ostream& write(std::ostream &out, EEnvLevel minLevel = kEnvLocal) const;

	void read(TDirectory *tdir, const TString &name = "settings");
	void writeToGDirectory(const TString &name = "settings", EEnvLevel minLevel = kEnvLocal) const;

	void readAuto(const TString &fileName, EEnvLevel level = kEnvLocal);

	std::string toString() const;
	
	void clear();

	Settings(TEnv* env, bool own = false);

	Settings();
	virtual ~Settings();
};



class GSettings {
public:
	static void getInstances(const TString &pattern, std::vector<int32_t> &instances) { Settings::global().getInstances(pattern, instances); }

	static bool get(const char* name, bool dflt, bool saveDflt = true) { return Settings::global()(name, dflt, saveDflt); }
	static int32_t get(const char* name, int32_t dflt, bool saveDflt = true) { return Settings::global()(name, dflt, saveDflt); }
	static double get(const char* name, double dflt, bool saveDflt = true) { return Settings::global()(name, dflt, saveDflt); }
	static const char* get(const char* name, const char* dflt, bool saveDflt = true) { return Settings::global()(name, dflt, saveDflt); }

	static void readAuto(const TString &fileName, EEnvLevel level = kEnvLocal)
		{ Settings::global().readAuto(fileName, level); }
};


} // namespace froast


#endif // FROAST_SETTINGS_H
