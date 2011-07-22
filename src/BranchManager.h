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


#ifndef FROAST_BRANCHMANAGER_H
#define FROAST_BRANCHMANAGER_H

#include <vector>
#include <list>
#include <string>
#include <stdexcept>
#include <stdint.h>

#include <Rtypes.h>
#include <TString.h>
#include <TTree.h>
#include <TUUID.h>


namespace froast {


class ManagedBranch {
protected:
	std::vector<TString> m_names;

public:
	const TString& name() const;

	const std::vector<TString>& names() const;
	
	void addName(const TString &branchName);

	virtual void outputTo(TTree *tree, const char* branchName = 0) = 0;
	virtual void inputFrom(TTree *tree, const char* branchName = 0) = 0;
	
	virtual void clear() = 0;

	ManagedBranch();

	ManagedBranch(const TString &branchName);
	
	virtual ~ManagedBranch();
};


template<typename A> class ScalarBranch: public ManagedBranch {
protected:
	A value;
	
public:
	virtual void outputTo(TTree *tree, const char* branchName = 0) {
		if (branchName == 0) branchName = name().Data();
		tree->Branch(branchName, &value, 32000, 0);
	}
	
	virtual void inputFrom(TTree *tree, const char* branchName = 0) {
		if ((branchName != 0) && (tree->SetBranchAddress(branchName, &value) >= 0) ) return;
		else for (std::vector<TString>::const_iterator it = names().begin(); it != names().end(); ++it) {
			if (tree->SetBranchAddress(it->Data(), &value) >= 0) return;
		}
		throw std::runtime_error(Form("Could not load branch \"%s\"", branchName ? branchName : name().Data()));
	}
	
	A& content() { return value; }
	const A& content() const { return value; }

	A& operator()() { return value; }
	const A& operator()() const { return value; }

	A& operator=(const A &x) { return value = x; }
	
	operator A& () { return value; }
	operator const A& () const { return value; }

	virtual void clear() { value = 0; }

	ScalarBranch() {}

	ScalarBranch(const TString &branchName)
		: ManagedBranch(branchName) { }
	
	virtual ~ScalarBranch() { }
};


template<typename A> class ObjectBranch: public ManagedBranch {
protected:
	A *value;
	
public:
	virtual void outputTo(TTree *tree, const char* branchName = 0) {
		if (branchName == 0) branchName = name().Data();
		tree->Branch(branchName, &value, 32000, 0);
	}
	
	virtual void inputFrom(TTree *tree, const char* branchName = 0) {
		if ((branchName != 0) && (tree->SetBranchAddress(branchName, &value) >= 0) ) return;
		else for (std::vector<TString>::const_iterator it = names().begin(); it != names().end(); ++it) {
			if (tree->SetBranchAddress(it->Data(), &value) >= 0) return;
		}
		throw std::runtime_error(Form("Could not load branch \"%s\"", branchName ? branchName : name().Data()));
	}

	A& content() { return *value; }
	const A& content() const { return *value; }
	
	A& operator=(const A &v) { return (*value) = v; }
	
	operator A& () { return *value; }
	operator const A& () const { return *value; }

	virtual void clear() { }

	ObjectBranch() { value = new A; }

	ObjectBranch(const TString &branchName)
		: ManagedBranch(branchName) { value = new A; }
	
	virtual ~ObjectBranch() { if (value != 0) delete value; }
};

typedef	ObjectBranch<TString> TStringBranch;
typedef	ObjectBranch<TUUID> TUUIDBranch;


template<typename A> class VectorBranch: public ManagedBranch {
protected:
	std::vector<A> *value;
	
public:
	virtual void outputTo(TTree *tree, const char* branchName = 0) {
		if (branchName == 0) branchName = name().Data();
		tree->Branch(branchName, &value, 32000, 0);
	}
	
	virtual void inputFrom(TTree *tree, const char* branchName = 0) {
		if ((branchName != 0) && (tree->SetBranchAddress(branchName, &value) >= 0) ) return;
		else for (std::vector<TString>::const_iterator it = names().begin(); it != names().end(); ++it) {
			if (tree->SetBranchAddress(it->Data(), &value) >= 0) return;
		}
		throw std::runtime_error(Form("Could not load branch \"%s\"", branchName ? branchName : name().Data()));
	}

	std::vector<A>& content() { return *value; }
	const std::vector<A>& content() const { return *value; }
	
	bool empty() { return value->empty(); }
	size_t size() { return value->size(); }
	size_t capacity() { return value->capacity(); }
	void resize(size_t n) {  value->resize(n); }
	void reserve(size_t n) {  value->reserve(n); }
	void clear() { value->clear(); }
	
	A& operator[](size_t i) { return value->at(i); }
	const A& operator[](size_t i) const { return value->at(i); }
	A& at(size_t i) { return value->at(i); }
	const A& at(size_t i) const { return value->at(i); }
	
	std::vector<A>& operator=(const std::vector<A> &v) { return (*value) = v; }

	void push_back(const A &x) { value->push_back(x); }
	
	operator std::vector<A>& () { return *value; }
	operator const std::vector<A>& () const { return *value; }

	VectorBranch()
		{ value = new std::vector<A>; }

	VectorBranch(const TString &branchName)
		: ManagedBranch(branchName) { value = new std::vector<A>; }
	
	virtual ~VectorBranch() { if (value != 0) delete value; }
};



class BranchManager {
protected:
	std::list<ManagedBranch*> m_branches;

public:
	void add(ManagedBranch &branch);
	
	void outputTo(TTree *tree);

	void inputFrom(TTree *tree);
	
	void clearData();
	
	BranchManager();
	virtual ~BranchManager();
};


} // namespace froast


#ifdef __CINT__
#pragma link C++ class froast::ManagedBranch-;

#pragma link C++ class froast::ScalarBranch<char>-;
#pragma link C++ class froast::ScalarBranch<bool>-;
#pragma link C++ class froast::ScalarBranch<int16_t>-;
#pragma link C++ class froast::ScalarBranch<int32_t>-;
#pragma link C++ class froast::ScalarBranch<float>-;
#pragma link C++ class froast::ScalarBranch<double>-;

#pragma link C++ class froast::ObjectBranch<TString>-;
#pragma link C++ typedef froast::TStringBranch;
#pragma link C++ class froast::ObjectBranch<TUUID>-;
#pragma link C++ typedef froast::TUUIDBranch;

#pragma link C++ class froast::VectorBranch<char>-;
// #pragma link C++ class froast::VectorBranch<bool>-;
#pragma link C++ class froast::VectorBranch<int16_t>-;
#pragma link C++ class froast::VectorBranch<int32_t>-;
#pragma link C++ class froast::VectorBranch<float>-;
#pragma link C++ class froast::VectorBranch<double>-;
#pragma link C++ class froast::VectorBranch<TString>-;

#pragma link C++ class froast::BranchManager-;
#endif

#endif // FROAST_BRANCHMANAGER_H
