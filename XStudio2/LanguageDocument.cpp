// LanguageDocument.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageDocument.h"
#include "LanguageFrame.h"
#include "LanguageEditView.h"
#include "PropertiesWnd.h"
#include "Logic/FileStream.h"
#include "Logic/LanguageFileReader.h"
#include "Logic/FileIdentifier.h"
#include "Logic/WorkerFeedback.h"
#include "Logic/StringLibrary.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Documents)

   // ---------------------------------- TEMPLATE ----------------------------------

   IMPLEMENT_DYNAMIC(LanguageDocTemplate, CMultiDocTemplate)

   /// <summary>Creates language document template</summary>
   LanguageDocTemplate::LanguageDocTemplate() 
      : CMultiDocTemplate(IDR_LANGUAGEVIEW, RUNTIME_CLASS(LanguageDocument), RUNTIME_CLASS(LanguageFrame), nullptr)
   {}

   /// <summary>Queries whether an external file should be opened as a language file</summary>
   /// <param name="lpszPathName">Path of file.</param>
   /// <param name="rpDocMatch">The already open document, if any.</param>
   /// <returns>yesAlreadyOpen if already open, yesAttemptNative if language file, noAttempt if unrecognised</returns>
   CDocTemplate::Confidence LanguageDocTemplate::MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch)
   {
      Confidence conf;

      // Ensure document not already open
      if ((conf = CMultiDocTemplate::MatchDocType(lpszPathName, rpDocMatch)) == yesAlreadyOpen)
         return yesAlreadyOpen;

      // Identify language file from header
      rpDocMatch = nullptr;
      return GuiString(L"String Library") == lpszPathName || FileIdentifier::Identify(lpszPathName) == FileType::Language ? yesAttemptNative : noAttempt;
   }

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNCREATE(LanguageDocument, DocumentBase)
   
   BEGIN_MESSAGE_MAP(LanguageDocument, DocumentBase)
      ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &LanguageDocument::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &LanguageDocument::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &LanguageDocument::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &LanguageDocument::OnQueryCommand)
      ON_COMMAND_RANGE(ID_EDIT_UNDO, ID_EDIT_REDO, &LanguageDocument::OnPerformCommand)
   END_MESSAGE_MAP()

   // -------------------------------- CONSTRUCTION --------------------------------

   LanguageDocument::LanguageDocument() : DocumentBase(DocumentType::Language), CurrentString(nullptr), CurrentPage(nullptr), Virtual(false)
   {
   }

   LanguageDocument::~LanguageDocument()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------
   
   
   /// <summary>Retrieves the file/library page collection</summary>
   /// <returns></returns>
   LanguageDocument::PageCollection&   LanguageDocument::GetContent() 
   {
      return Virtual ? Library : Content.Pages;
   }


   /// <summary>Gets the selected page.</summary>
   /// <returns></returns>
   LanguagePage*  LanguageDocument::GetSelectedPage() const
   {
      return CurrentPage;
   }
   
   /// <summary>Gets the index of the selected page.</summary>
   /// <returns>Zero-based index, or -1 if no selection</returns>
   int  LanguageDocument::GetSelectedPageIndex() const
   {
      return GetView<LanguagePageView>()->GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
   }

   /// <summary>Gets the selected string.</summary>
   /// <returns></returns>
   LanguageString*  LanguageDocument::GetSelectedString() const
   {
      return CurrentString;
   }

   /// <summary>Gets the index of the selected string.</summary>
   /// <returns>Zero-based index, or -1 if no selection</returns>
   int  LanguageDocument::GetSelectedStringIndex() const
   {
      return GetView<LanguageStringView>()->GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
   }
   
   /// <summary>Populates the properties window</summary>
   /// <param name="grid">The grid.</param>
   void  LanguageDocument::OnDisplayProperties(CMFCPropertyGridCtrl& grid)
   {
      // Group: ID/Language
      CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(Virtual ? L"Library" : L"File");
      if (!Virtual)
         group->AddSubItem(new LanguageDocument::IDProperty(*this));
      group->AddSubItem(new LanguageDocument::GameLanguageProperty(*this));
      grid.AddProperty(group);

      // Group: PageID/Title/Description/Voiced
      if (SelectedPage)
      {
         CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(_T("Page"));
         group->AddSubItem(new LanguagePageView::IDProperty(*this, *SelectedPage));
         group->AddSubItem(new LanguagePageView::TitleProperty(*this, *SelectedPage));
         group->AddSubItem(new LanguagePageView::DescriptionProperty(*this, *SelectedPage));
         group->AddSubItem(new LanguagePageView::VoicedProperty(*this, *SelectedPage));
         grid.AddProperty(group);
      }

      // Group: StringID/ColourTag/Version
      if (SelectedString)
      {
         CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(_T("String"));
         group->AddSubItem(new LanguageStringView::IDProperty(*this, *SelectedPage, *SelectedString));
         group->AddSubItem(new LanguageStringView::ColourTagProperty(*this, *SelectedString));
         group->AddSubItem(new LanguageStringView::VersionProperty(*this, *SelectedString));
         grid.AddProperty(group);
      }

      // Group: Library files
      if (Virtual)
      {
         group = new CMFCPropertyGridProperty(_T("Components"));
      
         // Enumerate files [backwards]
         for (auto file = StringLib.Files.crbegin(); file != StringLib.Files.crend(); ++file)    //for (auto& file : reverse_adapter<StringLibrary::FileCollection>(StringLib.Files))
         {
            bool included = (Components.find(file->ID) != Components.end());
            group->AddSubItem(new FileNameProperty(*this, *file, included));
         }
         grid.AddProperty(group);
      }
   }
   
   /// <summary>Called when document closed.</summary>
   void LanguageDocument::OnCloseDocument()
   {
      // Disconnect properties
      CPropertiesWnd::Connect(this, false);

      __super::OnCloseDocument();
   }

   /// <summary>Initializes new document</summary>
   /// <returns></returns>
   BOOL LanguageDocument::OnNewDocument()
   {
	   if (!DocumentBase::OnNewDocument())
		   return FALSE;
	   return TRUE;
   }

   /// <summary>Populates from a file path</summary>
   /// <param name="szPathName">Full path, or 'String Library'</param>
   /// <returns></returns>
   BOOL LanguageDocument::OnOpenDocument(LPCTSTR szPathName)
   {
      WorkerData data(Operation::LoadSaveDocument);

      try
      {
         // Feedback
         Console << Cons::UserAction << L"Loading language file: " << Path(szPathName) << ENDL;
         data.SendFeedback(ProgressType::Operation, 0, GuiString(L"Loading language file '%s'", szPathName));

         // Identify whether file or library
         Virtual = GuiString(L"String Library") == szPathName;
         
         // Library:
         if (Virtual)
         {
            // Initially display all files
            for (auto& file : StringLib.Files)
               Components.insert(file.ID);
            
            // Populate
            Populate();

            // Set language (for display purposes only) from main language file
            if (!StringLib.Files.empty())
               Content.Language = StringLib.Files.begin()->Language;
         }
         else
         {  
            // Parse input file
            StreamPtr fs2( new FileStream(szPathName, FileMode::OpenExisting, FileAccess::Read) );
            Content = LanguageFileReader(fs2).ReadFile(Path(szPathName).FileName);
         }

         data.SendFeedback(Cons::Green, ProgressType::Succcess, 0, L"Language file loaded successfully");
         return TRUE;
      }
      catch (ExceptionBase&  e)
      {
         // Feedback/Display error
         data.SendFeedback(ProgressType::Failure, 0, L"Failed to load language file");
         theApp.ShowError(HERE, e, GuiString(L"Failed to load language file '%s'", szPathName));
         return FALSE;
      }
   }

   /// <summary>Sets the selected page and raises PAGE SELECTION CHANGED.</summary>
   /// <param name="p">The page.</param>
   void  LanguageDocument::SetSelectedPage(LanguagePage* p)
   {
      CurrentPage = p;
      CPropertiesWnd::Connect(this, true);
      PageSelectionChanged.Raise();
   }
   
   /// <summary>Programatically select a page (raises PAGE SELECTION CHANGED).</summary>
   /// <param name="index">Zero-based item index, or -1 to clear selection</param>
   /// <exception cref="Logic::IndexOutOfRangeException">Invalid index</exception>
   void  LanguageDocument::SetSelectedPageIndex(int index)
   {
      auto& ctrl = GetView<LanguagePageView>()->GetListCtrl();

      // Validate index
      if (index > ctrl.GetItemCount())
         throw IndexOutOfRangeException(HERE, index, ctrl.GetItemCount());

      // Select+display page
      ctrl.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
      ctrl.EnsureVisible(index, FALSE);
   }

   /// <summary>Sets the selected string and raises STRING SELECTION CHANGED.</summary>
   /// <param name="s">The string.</param>
   void  LanguageDocument::SetSelectedString(LanguageString* s)
   {
      CurrentString = s;
      CPropertiesWnd::Connect(this, true);
      StringSelectionChanged.Raise();
   }
   
   /// <summary>Programatically select a string in the currently selected page (raises STRING SELECTION CHANGED).</summary>
   /// <param name="index">Zero-based item index, or -1 to clear selection</param>
   /// <exception cref="Logic::IndexOutOfRangeException">Invalid index</exception>
   void  LanguageDocument::SetSelectedStringIndex(int index)
   {
      auto& ctrl = GetView<LanguageStringView>()->GetListCtrl();

      // Validate index
      if (index > ctrl.GetItemCount())
         throw IndexOutOfRangeException(HERE, index, ctrl.GetItemCount());

      // Select+display string
      ctrl.SetItemState(index, LVIS_SELECTED, LVIS_SELECTED);
      ctrl.EnsureVisible(index, FALSE);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Includes or excludes a file.</summary>
   /// <param name="id">file ID.</param>
   /// <param name="include">include/exclude.</param>
   void LanguageDocument::IncludeFile(UINT id, bool include)
   {
      // Add/Remove ID from component set
      if (include)
         Components.insert(id);
      else
         Components.erase(id);

      // Re-populate
      Populate();
   }

   /// <summary>Determines whether a file is currently included in the library.</summary>
   /// <param name="id">The identifier.</param>
   /// <returns></returns>
   bool LanguageDocument::IsIncluded(UINT id) const
   {
      return Components.find(id) != Components.end();
   }

   /// <summary>Populates or re-populates the library.</summary>
   void LanguageDocument::Populate()
   {
      // Clear selection (if any)
      SelectedString = nullptr;
      SelectedPage = nullptr;

      // Clear library
      Library.clear();

      // Copy strings from all included library files
      for (auto& file : StringLib.Files)
         if (IsIncluded(file.ID))
            // Enum pages/strings
            for (auto& page : file)
               Library.Add(page);

      // Raise 'LIBRARY REBUILT'
      LibraryRebuilt.Raise();
   }

   /// <summary>Performs a menu command</summary>
   /// <param name="nID">Command identifier.</param>
   void LanguageDocument::OnPerformCommand(UINT nID)
   {
      //AfxMessageBox(L"LanguageDocument::OnPerformCommand");

      try 
      {
         switch (nID)
         {
         // Undo/Redo
         case ID_EDIT_UNDO:  Undo();    break;
         case ID_EDIT_REDO:  Redo();    break;
         }
      }
      catch (ExceptionBase& e) {
         theApp.ShowError(HERE, e);
      }
   }
   

   /// <summary>Queries the state of a menu command.</summary>
   /// <param name="pCmdUI">The command UI.</param>
   void LanguageDocument::OnQueryCommand(CCmdUI* pCmdUI)
   {
      bool state = false;

      switch (pCmdUI->m_nID)
      {
      // Save/SaveAs: Require file document
      case ID_FILE_SAVE:
      case ID_FILE_SAVE_AS:
         pCmdUI->Enable(!Virtual ? TRUE : FALSE);
         break;

      // Undo: Query document
      case ID_EDIT_UNDO:   
         state = CanUndo();
         pCmdUI->SetText(GetUndoMenuItem().c_str());
         break;
      
      // Redo: Query document
      case ID_EDIT_REDO:   
         state = CanRedo();
         pCmdUI->SetText(GetRedoMenuItem().c_str());
         break;
      }

      // Set state
      pCmdUI->Enable(state ? TRUE : FALSE);
      pCmdUI->SetCheck(FALSE);
   }

   // ------------------------------- PRIVATE METHODS ------------------------------
   
NAMESPACE_END2(GUI,Documents)


