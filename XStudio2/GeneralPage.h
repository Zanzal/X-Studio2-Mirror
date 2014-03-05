#pragma once
#include "PreferencesPage.h"


/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Preferences)
   
   /// <summary></summary>
   class GeneralPage : public PreferencesPage
   {
      // ------------------------ TYPES --------------------------
   public:
      enum { IDD = IDD_GENERAL_PAGE };

   protected:
      /// <summary>Game data folder property</summary>
      class GameDataFolderProperty : public FolderProperty
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'game data folder' property</summary>
         /// <param name="page">Owner page.</param>
         GameDataFolderProperty(PreferencesPage& page) 
            : FolderProperty(page, L"Base Folder", PrefsLib.GameDataFolder, L"Folder containing X3 executable")
         {}
      };

      /// <summary>Game data language property</summary>
      class GameLanguageProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create game language property.</summary>
         /// <param name="page">Owner page.</param>
         GameLanguageProperty(PreferencesPage& page)
            : PropertyBase(page, L"Language", ::GetString(PrefsLib.GameDataLanguage).c_str(),  L"Change which language to use when loading game data")
         {
            // Populate game languages
            for (int i = 0; i <= 7; i++)
               AddOption(::GetString(GameLanguageIndex(i).Language).c_str(), FALSE);

            // Strict dropdown
            AllowEdit(FALSE);
         }

         // ---------------------- ACCESSORS ------------------------	
      public:
         /// <summary>Gets the selected language.</summary>
         /// <returns></returns>
         GameLanguage  GetLanguage() const
         {
            return GameLanguageIndex(Find(GetString())).Language;
         }
         // ----------------------- MUTATORS ------------------------
         
         // -------------------- REPRESENTATION ---------------------
      protected:
      };

      /// <summary>Game version property</summary>
      class GameVersionProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create game version property.</summary>
         /// <param name="page">Owner page.</param>
         GameVersionProperty(PreferencesPage& page)
            : PropertyBase(page, L"Version", VersionString(PrefsLib.GameDataVersion).c_str(),  L"Change which version to use when loading game data")  
         {
            // Populate game versions
            AddOption(VersionString(GameVersion::Threat).c_str(), FALSE);
            AddOption(VersionString(GameVersion::Reunion).c_str(), FALSE);
            AddOption(VersionString(GameVersion::TerranConflict).c_str(), FALSE);
            AddOption(VersionString(GameVersion::AlbionPrelude).c_str(), FALSE);

            // Strict dropdown
            AllowEdit(FALSE);
         }

         // ---------------------- ACCESSORS ------------------------	
      public:
         /// <summary>Gets the selected version.</summary>
         /// <returns></returns>
         GameVersion  GetVersion() const
         {
            return GameVersionIndex(Find(GetString())).Version;
         }

         // ----------------------- MUTATORS ------------------------

         // -------------------- REPRESENTATION ---------------------
      protected:
      };

      /// <summary>Large menus property</summary>
      class LargeMenusProperty : public BooleanProperty
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'large menus' property</summary>
         /// <param name="page">Owner page.</param>
         LargeMenusProperty(PreferencesPage& page) 
            : BooleanProperty(page, L"Large Menus", PrefsLib.LargeMenus, L"Enlarge all menu items and icons")
         {}
      };

      /// <summary>Large toolbars property</summary>
      class LargeToolbarsProperty : public BooleanProperty
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'large toolbars' property</summary>
         /// <param name="page">Owner page.</param>
         LargeToolbarsProperty(PreferencesPage& page) 
            : BooleanProperty(page, L"Large Toolbars", PrefsLib.LargeToolbars, L"Enlarge all toolbar buttons")
         {}
      };
      
      /// <summary>SkipBrokenFiles property</summary>
      class SkipBrokenFilesProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'SkipBrokenFiles' property</summary>
         /// <param name="page">Owner page.</param>
         SkipBrokenFilesProperty(PreferencesPage& page) 
            : PropertyBase(page, L"Corrupt Files", L"", L"Change whether to continue loading game data if a language file cannot be loaded")
         {
            AddOption(L"Skip and Continue Loading", FALSE);
            AddOption(L"Abort Loading", FALSE);
            AllowEdit(FALSE);
            // Set initial value
            SetValue(GetOption(PrefsLib.SkipBrokenFiles ? 0 : 1));
         }

         // ---------------------- ACCESSORS ------------------------
      public:
         /// <summary>Gets value as bool.</summary>
         /// <returns></returns>
         bool GetBool() const
         {
            return GetString() == GetOption(0);
         }

         // ----------------------- MUTATORS ------------------------
      
         // -------------------- REPRESENTATION ---------------------
      };

      /// <summary>Tool window font property</summary>
      class ToolWindowFontProperty : public FontProperty
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'tool window font' property</summary>
         /// <param name="page">Owner page.</param>
         ToolWindowFontProperty(PreferencesPage& page) 
            : FontProperty(page, L"Window Font", PrefsLib.ToolWindowFont, L"Change the font used in all tool/helper windows like properties, project etc.")
         {}
      };

      /// <summary>Tooltip font property</summary>
      class TooltipFontProperty : public FontProperty
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create 'tooltip font' property</summary>
         /// <param name="page">Owner page.</param>
         TooltipFontProperty(PreferencesPage& page) 
            : FontProperty(page, L"Tooltip Font", PrefsLib.TooltipFont, L"Change the font used in all tooltips")
         {}
      };
	  
      // --------------------- CONSTRUCTION ----------------------
   public:
      GeneralPage();    
      virtual ~GeneralPage();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNAMIC(GeneralPage)
      DECLARE_MESSAGE_MAP()
	  
      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
   public:

      // ----------------------- MUTATORS ------------------------
   public:
      void OnOK() override;

   protected:
      void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	   void Populate() override;

      // -------------------- REPRESENTATION ---------------------
   protected:
      SkipBrokenFilesProperty* SkipBrokenFiles;
      GameDataFolderProperty*  GameDataFolder;
      GameLanguageProperty*    GameDataLanguage;
      GameVersionProperty*     GameDataVersion;
      LargeMenusProperty*      LargeMenus;
      LargeToolbarsProperty*   LargeToolbars;
      TooltipFontProperty*     TooltipFont;
      ToolWindowFontProperty*  ToolWindowFont;
      
   public:
      
   };
   
NAMESPACE_END2(GUI,Preferences)

