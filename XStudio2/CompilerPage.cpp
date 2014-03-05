#include "stdafx.h"
#include "CompilerPage.h"
#include "afxdialogex.h"


/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Preferences)

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNAMIC(CompilerPage, PreferencesPage)

   BEGIN_MESSAGE_MAP(CompilerPage, PreferencesPage)
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   CompilerPage::CompilerPage() 
      : PreferencesPage(CompilerPage::IDD),
        CaseSensitiveVariables(nullptr), 
        CheckArgumentNames(nullptr), 
        CheckArgumentTypes(nullptr),
        UseDoIfSyntax(nullptr), 
        UseCppOperators(nullptr)
   {
   }

   CompilerPage::~CompilerPage()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Performs data exchange.</summary>
   /// <param name="pDX">The dx.</param>
   void CompilerPage::DoDataExchange(CDataExchange* pDX)
   {
	   __super::DoDataExchange(pDX);
   }


   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Populates this page.</summary>
   void CompilerPage::Populate()
   {
      // General
      auto group = new PropertyBase(*this, L"General");
      group->AddSubItem(UseCppOperators = new UseCppOperatorsProperty(*this));
      group->AddSubItem(UseDoIfSyntax = new UseDoIfSyntaxProperty(*this));
      group->AddSubItem(CheckArgumentNames = new CheckArgumentNamesProperty(*this));
      group->AddSubItem(CheckArgumentTypes = new CheckArgumentTypesProperty(*this));
      group->AddSubItem(CaseSensitiveVariables = new CaseSensitiveVariablesProperty(*this));
      Grid.AddProperty(group);
   }
   
   // ------------------------------- PRIVATE METHODS ------------------------------
   
   
NAMESPACE_END2(GUI,Preferences)

