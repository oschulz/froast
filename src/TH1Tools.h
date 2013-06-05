// Copyright (C) 2013 Oliver Schulz <oliver.schulz@tu-dortmund.de>

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


#ifndef FROAST_TH1TOOLS_H
#define FROAST_TH1TOOLS_H

#include <TString.h>
#include <TH1.h>
#include <TPad.h>


namespace froast {


class TH1Tools {
protected:
	TH1 *m_hist;

public:
	TH1* hist() { return m_hist; }
	const TH1* hist() const { return m_hist; }

	operator TH1* () { return m_hist; }
	operator const TH1* () const { return m_hist; }

	TH1& operator*() const { return *m_hist; }
	TH1* operator->() const { return m_hist; }

	TH1Tools& title(const TString& title) {
		m_hist->SetTitle(title.Data());
		return *this;
	}

	TH1Tools& xTitle(const TString& label) {
		m_hist->SetXTitle(label.Data());
		return *this;
	}

	TH1Tools& yTitle(const TString& label) {
		m_hist->SetYTitle(label.Data());
		return *this;
	}

	TH1Tools& zTitle(const TString& label) {
		m_hist->SetZTitle(label.Data());
		return *this;
	}

	TH1Tools& lineColor(Color_t color) {
		m_hist->SetLineColor(color);
		return *this;
	}

	TH1Tools& fillColor(Color_t color) {
		m_hist->SetFillColor(color);
		return *this;
	}

	TH1Tools& stats(Bool_t stats = kTRUE) {
		m_hist->SetStats(stats);
		return *this;
	}

	TH1Tools(TH1 *h);
	virtual ~TH1Tools();
};


} // namespace froast

#endif // FROAST_TH1TOOLS_H
