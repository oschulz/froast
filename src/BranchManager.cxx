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


#include "BranchManager.h"

#include <iostream>


using namespace std;


namespace froast {


const TString& ManagedBranch::name() const {
	if (m_names.empty()) throw invalid_argument("No branch name set");
	return m_names[0];
}


const std::vector<TString>& ManagedBranch::names() const {
	return m_names;
}


void ManagedBranch::addName(const TString &branchName) {
	m_names.push_back(branchName);
}


ManagedBranch::ManagedBranch() {}


ManagedBranch::ManagedBranch(const TString &branchName) {
	m_names.push_back(branchName);
}


ManagedBranch::~ManagedBranch() {}



void BranchManager::add(ManagedBranch &branch) {
	m_branches.push_back(&branch);
}


void BranchManager::outputTo(TTree *tree) {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).outputTo(tree);
}


void BranchManager::inputFrom(TTree *tree) {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).inputFrom(tree);
}


void BranchManager::clearData() {
	for (std::list<ManagedBranch*>::iterator it = m_branches.begin(); it != m_branches.end(); ++it)
		(**it).clear();
}


BranchManager::BranchManager() {
}


BranchManager::~BranchManager() {
}


} // namespace froast
