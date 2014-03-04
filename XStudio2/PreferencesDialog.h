#pragma once
#include "GeneralPage.h"
#include "CompilerPage.h"
#include "EditorPage.h"
#include "GameDataPage.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Preferences)
   
   /// <summary></summary>
   class PreferencesDialog : public CMFCPropertySheet
   {
      // ------------------------ TYPES --------------------------
   public:

      // --------------------- CONSTRUCTION ----------------------
   public:
      PreferencesDialog();    
      virtual ~PreferencesDialog();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNAMIC(PreferencesDialog)
      DECLARE_MESSAGE_MAP()
	  
      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
   public:

      // ----------------------- MUTATORS ------------------------
   protected:
      void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	  
      // -------------------- REPRESENTATION ---------------------
   protected:
      CompilerPage  Compiler;
      EditorPage    Editor;
      GameDataPage  GameData;
      GeneralPage   General;
   };
   
NAMESPACE_END2(GUI,Preferences)
