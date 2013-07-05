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


class BranchManager;
class InputBranchManager;
class OutputBranchManager;



class ManagedBranch {
protected:
	std::vector<TString> m_names;
	bool m_inAvail;

	template <typename Value> ManagedBranch& inputValueFrom(Value &value, TTree *tree, const char* branchName = 0, bool optional = false);

	template<typename Value> ManagedBranch& outputValueTo(Value &value, TTree *tree, const char* branchName = 0);

public:
	const TString& name() const;

	const std::vector<TString>& names() const;
	
	bool inputAvailable() const { return m_inAvail; }

	ManagedBranch& addName(const TString &branchName);

	virtual ManagedBranch& inputFrom(TTree *tree, const char* branchName = 0, bool optional = false) = 0;

	virtual ManagedBranch& outputTo(TTree *tree, const char* branchName = 0) = 0;

	ManagedBranch& addTo(BranchManager &manager);

	ManagedBranch& inputFrom(InputBranchManager &manager, bool optional = false);

	ManagedBranch& outputTo(OutputBranchManager &manager, int32_t outputLevel = 0);
	
	virtual void clear() = 0;

	ManagedBranch();

	ManagedBranch(const TString &branchName);

	virtual ~ManagedBranch();
};


template <typename Value> ManagedBranch& ManagedBranch::inputValueFrom(Value &value, TTree *tree, const char* branchName, bool optional) {
	if ((branchName != 0) && (tree->SetBranchAddress(branchName, &value) >= 0) ) { m_inAvail = true; return *this; }
	else for (std::vector<TString>::const_iterator it = names().begin(); it != names().end(); ++it) {
		// TChain::SetBranchAddress seems to return kNoCheck in every case, so:
		if (tree->GetBranch(it->Data())) {
			tree->SetBranchStatus(it->Data(), true);
			if (tree->SetBranchAddress(it->Data(), &value) >= 0) {
				tree->AddBranchToCache(it->Data());
				m_inAvail = true;
				return *this;
			}
		}
	}
	m_inAvail = false;
	if (!optional) throw std::runtime_error(Form("Could not load branch \"%s\"", branchName ? branchName : name().Data()));
	return *this;
}


template<typename Value> ManagedBranch& ManagedBranch::outputValueTo(Value &value, TTree *tree, const char* branchName) {
	if (branchName == 0) branchName = name().Data();
	tree->Branch(branchName, &value, 32000, 0);
	return *this;
}



template<typename A> class ScalarBranch: public ManagedBranch {
protected:
	A value;
	
public:
	virtual ManagedBranch& inputFrom(TTree *tree, const char* branchName = 0, bool optional = false)
		{ return inputValueFrom(value, tree, branchName, optional); }

	virtual ManagedBranch& outputTo(TTree *tree, const char* branchName = 0)
		{ return outputValueTo(value, tree, branchName); }
	
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
	virtual ManagedBranch& inputFrom(TTree *tree, const char* branchName = 0, bool optional = false)
		{ return inputValueFrom(value, tree, branchName, optional); }

	virtual ManagedBranch& outputTo(TTree *tree, const char* branchName = 0)
		{ return outputValueTo(value, tree, branchName); }

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
	virtual ManagedBranch& inputFrom(TTree *tree, const char* branchName = 0, bool optional = false)
		{ return inputValueFrom(value, tree, branchName, optional); }

	virtual ManagedBranch& outputTo(TTree *tree, const char* branchName = 0)
		{ return outputValueTo(value, tree, branchName); }

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

	typename std::vector<A>::iterator begin() { return value->begin(); }
	typename std::vector<A>::const_iterator begin() const { return value->begin(); }
	typename std::vector<A>::iterator end() { return value->end(); }
	typename std::vector<A>::const_iterator end() const { return value->end(); }
	
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
	
	/// @deprecated, Use InputBranchManager or OutputBranchManager instead
	void inputFrom(TTree *tree);

	/// @deprecated, Use InputBranchManager or OutputBranchManager instead
	void outputTo(TTree *tree);

	void clearData();
	
	BranchManager();
	virtual ~BranchManager();
};


// Note: Once deprecated methods inputFrom and outputTo have been removed
// from BranchManager, InputBranchManager should be changed to inherit
// from BranchManager
class InputBranchManager {
protected:
	struct BranchSpec {
		ManagedBranch* branch;
		bool optional;
		BranchSpec(ManagedBranch* managedBranch, bool isOptional)
			: branch(managedBranch), optional(isOptional) {}
	};

	std::list<BranchSpec> m_branches;

public:

	void add(ManagedBranch &branch, bool optional = false);

	void inputFrom(TTree *tree);

	void clearData();

	InputBranchManager();
	virtual ~InputBranchManager();
};


// Note: Once deprecated methods inputFrom and outputTo have been removed
// from BranchManager, OutputBranchManager should be changed to inherit
// from BranchManager
class OutputBranchManager {
protected:
	struct BranchSpec {
		ManagedBranch* branch;
		int32_t outputLevel;
		BranchSpec(ManagedBranch* managedBranch, int32_t branchOutputLevel)
			: branch(managedBranch), outputLevel(branchOutputLevel) {}
	};

	std::list<BranchSpec> m_branches;

public:

	void add(ManagedBranch &branch, int32_t outputLevel = 0);

	void outputTo(TTree *tree, int32_t maxOutputLevel = 0);

	void clearData();

	OutputBranchManager();
	virtual ~OutputBranchManager();
};



} // namespace froast


#endif // FROAST_BRANCHMANAGER_H
