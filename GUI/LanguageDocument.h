#pragma once
#include "../Logic/LanguageFile.h"
#include "DocumentBase.h"
#include "DocTemplateBase.h"
#include "PropertySource.h"
#include "GuiCommand.h"

/// <summary>Forward declaration</summary>
FORWARD_DECLARATION2(GUI,Controls,class LanguageButton)

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Documents)

   // ------------------------- ENUMS -------------------------

   /// <summary>Language document editor mode</summary>
   enum class EditMode { Source, Edit, Display };

   // ----------------- EVENTS AND DELEGATES ------------------

   typedef Event<>                             SelectionChangedEvent;
   typedef SelectionChangedEvent::DelegatePtr  SelectionChangedHandler;

   // ------------------------ CLASSES ------------------------

   /// <summary>Language document template</summary>
   class LanguageDocTemplate : public DocTemplateBase
   {
      // --------------------- CONSTRUCTION ----------------------
   public:
      LanguageDocTemplate();

      // ------------------------ STATIC -------------------------

      DECLARE_DYNAMIC(LanguageDocTemplate)

      // ---------------------- ACCESSORS ------------------------	

      virtual Confidence MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch);
   };

   /// <summary>Language document</summary>
   class LanguageDocument : public DocumentBase, public PropertySource, public CommandQueue
   {
	   // ------------------------ TYPES --------------------------
   public:
      /// <summary>LanguageFile Page collection</summary>
	   typedef LanguageFile::PageCollection PageCollection;

      /// <summary>Language button</summary>
      typedef GUI::Controls::LanguageButton ButtonData;

   public:
      /// <summary>Base class for all Language document properties</summary>
      class PropertyBase : public ValidatingProperty
      {
      public:
         /// <summary>Create item properties</summary>
         /// <param name="d">document.</param>
         /// <param name="name">name.</param>
         /// <param name="val">value.</param>
         /// <param name="desc">description.</param>
         PropertyBase(LanguageDocument& d, wstring name, _variant_t val, wstring desc)
            : Document(d), File(d.File), ValidatingProperty(name.c_str(), val, desc.c_str(), NULL, nullptr, nullptr, nullptr)
         {
            // Disable if document is virtual
            Enable(Document.Virtual ? FALSE : TRUE);
         }

         /// <summary>Create group property.</summary>
         /// <param name="d">Document.</param>
         /// <param name="name">Group name.</param>
         /// <param name="valueList">Whether a value list.</param>
         PropertyBase(LanguageDocument& d, wstring name, bool valueList = false)
            : Document(d), File(d.File), ValidatingProperty(name.c_str(), 0, valueList ? TRUE : FALSE)
         {
            // Disable if document is virtual
            Enable(Document.Virtual ? FALSE : TRUE);
         }

      protected:
         /// <summary>Modify document</summary>
         /// <param name="value">value text</param>
         void OnValueChanged(GuiString value) override
         {
            if (!Document.Virtual)
               Document.SetModifiedFlag(TRUE);
         }

      protected:
         LanguageDocument&  Document;
         LanguageFile&      File;
      };

      /// <summary>Base class for all language document GUI commands</summary>
      class CommandBase : public GuiCommand
      {
         // ------------------------ TYPES --------------------------

         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create command</summary>
         /// <param name="doc">The document.</param>
         CommandBase(LanguageDocument& doc) : Document(doc)
         {}

         // ---------------------- ACCESSORS ------------------------			
      public:
         /// <summary>Enable Undo</summary>
         /// <returns></returns>
         bool CanUndo() const override { return true; }

         // ----------------------- MUTATORS ------------------------
      
         // -------------------- REPRESENTATION ---------------------
      protected:
         LanguageDocument&  Document;
      };

   protected:
      /// <summary>Language filename property grid item</summary>
      class FileNameProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create filename property.</summary>
         /// <param name="doc">Language document.</param>
         /// <param name="file">Language file.</param>
         /// <param name="included">Whether included or excluded.</param>
         FileNameProperty(LanguageDocument& doc, const LanguageFile& file, bool included)
            : File(file), PropertyBase(doc, file.FullPath.FileName, included ? L"Included" : L"Excluded", L"Include or exclude file from library")
         {
            // Strict list
            AddOption(L"Included");
            AddOption(L"Excluded");
            AllowEdit(FALSE);
            // Enable property
            Enable(TRUE);
         }

         // ---------------------- ACCESSORS ------------------------	

         // ----------------------- MUTATORS ------------------------
      protected:
         /// <summary>Include/Exclude selected file, rebuild library</summary>
         /// <param name="value">value text</param>
         void OnValueChanged(GuiString value) override
         {
            Document.IncludeFile(File.ID, value == L"Included");
         }

         // -------------------- REPRESENTATION ---------------------
      protected:
         const LanguageFile& File;
      };

      /// <summary>Game language property grid item</summary>
      class GameLanguageProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create game language property.</summary>
         /// <param name="doc">Language document.</param>
         GameLanguageProperty(LanguageDocument& doc)
            : PropertyBase(doc, L"Language", GetString(doc.File.Language).c_str(),  L"Language of all strings in document")
         {
            // Populate game languages
            for (int i = 0; i <= 7; i++)
               AddOption(GetString(GameLanguageIndex(i).Language).c_str(), FALSE);

            // Strict dropdown
            AllowEdit(FALSE);
         }

         // ---------------------- ACCESSORS ------------------------	

         // ----------------------- MUTATORS ------------------------
      protected:
         /// <summary>Update language</summary>
         /// <param name="value">value text</param>
         /// <exception cref="Logic::ApplicationException">New path already exists, or project already contains new path</exception>
         /// <exception cref="Logic::IOException">Unable to rename file</exception>
         void OnValueChanged(GuiString value) override
         {
            auto lang = GameLanguageIndex(Find(value.c_str())).Language;      // Convert zero-based index into GameLanguage

            // Attempt to Rename file/document/projectItem/title
            Document.Rename(File.ID, lang);

            // [Success] Change language + Modify document
            File.Language = lang;
            __super::OnValueChanged(value);  
         }

         // -------------------- REPRESENTATION ---------------------
      protected:
      };

      /// <summary>Language file ID property grid item</summary>
      class IDProperty : public PropertyBase
      {
         // --------------------- CONSTRUCTION ----------------------
      public:
         /// <summary>Create language file ID property.</summary>
         /// <param name="doc">Language document.</param>
         IDProperty(LanguageDocument& doc)
            : PropertyBase(doc, L"ID", VString(L"%d", doc.File.ID).c_str(),  L"File ID")
         {}

         // ---------------------- ACCESSORS ------------------------	

         // ----------------------- MUTATORS ------------------------
      protected:
         /// <summary>Ensures ID is between 1 and 9999</summary>
         /// <param name="value">The value.</param>
         /// <returns>True to accept, false to reject</returns>
         bool OnValidateValue(GuiString& value) override
         {
            return value.length() && value.IsNumeric()               // Not empty
                && value.ToInt() >= 1 && value.ToInt() <= 9999;      // 1 <= val <= 9999
         }

         /// <summary>Update file ID and title</summary>
         /// <param name="value">value text</param>
         /// <exception cref="Logic::ApplicationException">New path already exists, or project already contains new path</exception>
         /// <exception cref="Logic::IOException">Unable to rename file</exception>
         void OnValueChanged(GuiString value) override
         {
            // Attempt Rename file/document/projectItem/title
            Document.Rename(value.ToInt(), File.Language);

            // [Success] Change ID + Modify document
            File.ID = value.ToInt();
            __super::OnValueChanged(value);  
         }

         // -------------------- REPRESENTATION ---------------------
      protected:
      };

      // --------------------- CONSTRUCTION ----------------------
   public:
      LanguageDocument();
	   virtual ~LanguageDocument();

      // ------------------------ STATIC -------------------------
      DECLARE_DYNCREATE(LanguageDocument)
      DECLARE_MESSAGE_MAP()

      // --------------------- PROPERTIES ------------------------
   public:
      PROPERTY_GET(PageCollection&,Content,GetContent);
      PROPERTY_GET_SET(EditMode,CurrentMode,GetEditMode,SetEditMode);
      PROPERTY_GET_SET(ButtonData*,SelectedButton,GetSelectedButton,SetSelectedButton);
      PROPERTY_GET_SET(LanguagePage*,SelectedPage,GetSelectedPage,SetSelectedPage);
      PROPERTY_GET_SET(LanguageString*,SelectedString,GetSelectedString,SetSelectedString);
      PROPERTY_SET(int,SelectedPageIndex,SetSelectedPageIndex);
      PROPERTY_SET(wstring,SelectedStringText,SetSelectedStringText);

      // ---------------------- ACCESSORS ------------------------			
   public:
      PageCollection&  GetContent();
      EditMode         GetEditMode() const;
      ButtonData*      GetSelectedButton() const;
      LanguagePage*    GetSelectedPage() const;
      LanguageString*  GetSelectedString() const;

      /// <summary>Gets any language view</summary>
      /// <typeparam name="VIEW">View type</typeparam>
      /// <returns></returns>
      /// <exception cref="Logic::AlgorithmException">View not found</exception>
      template<typename VIEW>
      VIEW* GetView() const
      {
         // Iterate thru views
         for (POSITION pos = GetFirstViewPosition(); pos != NULL; )
            if (VIEW* v = dynamic_cast<VIEW*>(GetNextView(pos)))
               return v;

         // Error: Not found
         throw AlgorithmException(HERE, L"Cannot find desired View");
      }

   protected:
      bool  IsIncluded(UINT id) const;

      // ----------------------- MUTATORS ------------------------
   public:
      LanguagePage    CreatePage(LanguagePage* insertAt = nullptr);
      LanguageString  CreateString(UINT page, LanguageString* insertAt = nullptr);
      void  InsertPage(LanguagePage& page);
      void  InsertString(LanguageString& str);
      void  Rename(UINT id, GameLanguage lang);
      void  RenamePage(LanguagePage& page, UINT newID);
      void  RenameString(LanguageString& str, UINT newID);
      void  RemovePage(UINT page);
      void  RemoveString(UINT page, UINT id);
      void  OnDisplayProperties(CMFCPropertyGridCtrl& grid) override;
      void  OnCloseDocument() override;
      void  OnDocumentEvent(DocumentEvent deEvent) override;
      BOOL  OnNewDocument() override;
      BOOL  OnOpenDocument(LPCTSTR szPathName) override;
      BOOL  OnSaveDocument(LPCTSTR szPathName) override;
      void  SetEditMode(EditMode m);
      void  SetSelectedButton(ButtonData* b);
      void  SetSelectedPage(LanguagePage* p);
      void  SetSelectedPageIndex(int index);
      void  SetSelectedString(LanguageString* s);
      void  SetSelectedStringText(const wstring& txt);

   protected:
      void  IncludeFile(UINT id, bool include);
      void  Populate();
      void  OnPerformCommand(UINT nID);
      void  OnQueryCommand(CCmdUI* pCmdUI);

      // -------------------- REPRESENTATION ---------------------
   public:
      LanguageFile     File;
      PageCollection   Library;
      
      SelectionChangedEvent  StringSelectionChanged,
                             PageSelectionChanged;
      SimpleEvent            LibraryRebuilt,
                             EditModeChanged,
                             PageUpdated,
                             StringUpdated;

   protected:
      EditMode         Mode;             // Current editing mode
      ButtonData*      CurrentButton;    // Currently selected button
      LanguageString*  CurrentString;    // Currently selected string
      LanguagePage*    CurrentPage;      // Currently selected page
      set<UINT>        Components;       // [LIBRARY] IDs of currently included files
};


NAMESPACE_END2(GUI,Documents)

