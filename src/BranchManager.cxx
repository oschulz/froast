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


#include "BranchManager.h"

#include <iostream>


using namespace std;


namespace froast {


const TString& ManagedBranch::name() const {
	if (m_names.empty()) throw invalid_argument("No branch name set");
	return m_names.back();
}


const std::vector<TString>& ManagedBranch::names() const {
	return m_names;
}


ManagedBranch& ManagedBranch::addName(const TString &branchName) {
	m_names.push_back(branchName);
	return *this;
}

ManagedBranch& ManagedBranch::addTo(BranchManager &manager) {
    manager.add(*this);
	return *this;
}

ManagedBranch& ManagedBranch::inputFrom(InputBranchManager &manager, bool optional) {
	manager.add(*this, optional);
	return *this;
}

ManagedBranch& ManagedBranch::outputTo(OutputBranchManager &manager, int32_t outputLevel) {
	manager.add(*this, outputLevel);
	return *this;
}


ManagedBranch::ManagedBranch()
	: m_inAvail(false)
{}


ManagedBranch::ManagedBranch(const TString &branchName)
	: m_inAvail(false)
{
	m_names.push_back(branchName);
}


ManagedBranch::~ManagedBranch() {}



void BranchManager::add(ManagedBranch &branch) {
	m_branches.push_back(&branch);
}


void BranchManager::inputFrom(TTree *tree) {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).inputFrom(tree);
}


void BranchManager::outputTo(TTree *tree) {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).outputTo(tree);
}


void BranchManager::clearData() {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).clear();
}


BranchManager::BranchManager() {}


BranchManager::~BranchManager() {}



void InputBranchManager::add(ManagedBranch &branch, bool optional) {
	m_branches.push_back(BranchSpec(&branch, optional));
}


void InputBranchManager::inputFrom(TTree *tree) {
	for (std::list<BranchSpec>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		it->branch->inputFrom(tree, 0, it->optional);
}


void InputBranchManager::clearData() {
	for (std::list<BranchSpec>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		it->branch->clear();
}


InputBranchManager::InputBranchManager() {}


InputBranchManager::~InputBranchManager() {}



void OutputBranchManager::add(ManagedBranch &branch, int32_t outputLevel) {
	m_branches.push_back(BranchSpec(&branch, outputLevel));
}


void OutputBranchManager::outputTo(TTree *tree, int32_t maxOutputLevel) {
	for (std::list<BranchSpec>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		if (it->outputLevel <= maxOutputLevel) it->branch->outputTo(tree, 0);
}


void OutputBranchManager::clearData() {
	for (std::list<BranchSpec>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		it->branch->clear();
}


OutputBranchManager::OutputBranchManager() {}


OutputBranchManager::~OutputBranchManager() {}


} // namespace froast
