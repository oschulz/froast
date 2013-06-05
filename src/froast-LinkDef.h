#ifndef froast_LinkDef_h
#define froast_LinkDef_h

#ifdef __CINT__

// util.h
#pragma link C++ class froast::Util-;

// BranchManager.h

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
#pragma link C++ class froast::InputBranchManager-;
#pragma link C++ class froast::OutputBranchManager-;

// File.h
#pragma link C++ class froast::File-;

// JSON.h
#pragma link C++ class froast::JSON-;

// Selector.h
#pragma link C++ class froast::Selector-;

// Settings.h
#pragma link C++ class froast::Param-;
#pragma link C++ class froast::Settings-;
#pragma link C++ class froast::GSettings-;

// TH1Tools.h
#pragma link C++ class froast::TH1Tools-;

#endif // __CINT__


#endif // froast_LinkDef_h
