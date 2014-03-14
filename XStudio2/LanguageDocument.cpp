// LanguageDocument.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageDocument.h"
#include "LanguageFrame.h"
#include "LanguageEditView.h"
#include "LanguageStringView.h"
#include "LanguagePageView.h"
#include "PropertiesWnd.h"
#include "Logic/XFileInfo.h"
#include "Logic/LanguageFileReader.h"
#include "Logic/LanguageFileWriter.h"
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
      ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, &LanguageDocument::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, &LanguageDocument::OnQueryCommand)
      ON_COMMAND_RANGE(ID_EDIT_UNDO, ID_EDIT_REDO, &LanguageDocument::OnPerformCommand)
   END_MESSAGE_MAP()

   // -------------------------------- CONSTRUCTION --------------------------------

   LanguageDocument::LanguageDocument() : DocumentBase(DocumentType::Language, false), 
                                          CurrentString(nullptr), CurrentPage(nullptr), CurrentButton(nullptr),
                                          Mode(EditMode::Edit)
   {
   }

   LanguageDocument::~LanguageDocument()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------
   
   /// <summary>Creates a new Page.</summary>
   /// <param name="insertAt">insertion point</param>
   /// <returns>Page with available ID</returns>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   LanguagePage  LanguageDocument::CreatePage(LanguagePage* insertAt /*= nullptr*/)
   {
      // Determine ID
      UINT newID = File.GetAvailableID(insertAt ? insertAt->ID : -1);
      
      // Generate Page 
      return LanguagePage(newID, L"<New Page>", L"", false);
   }

   /// <summary>Creates a new string.</summary>
   /// <param name="page">destination page.</param>
   /// <param name="insertAt">insertion point</param>
   /// <returns>String with available ID</returns>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
   LanguageString  LanguageDocument::CreateString(UINT page, LanguageString* insertAt /*= nullptr*/)
   {
      // Determine ID
      UINT newID = Content.Find(page).GetAvailableID(insertAt ? insertAt->ID : -1);
      
      // Generate string (must identify colour tags before display)
      auto str = LanguageString(newID, page, L"<New String>", GameVersion::Threat);
      str.IdentifyColourTags();

      return str;
   }

   /// <summary>Retrieves the file/library page collection</summary>
   /// <returns></returns>
   LanguageDocument::PageCollection&   LanguageDocument::GetContent() 
   {
      return Virtual ? Library : File.Pages;
   }
   
   /// <summary>Gets the edit mode.</summary>
   /// <returns></returns>
   EditMode  LanguageDocument::GetEditMode() const
   {
      return Mode;
   }

   /// <summary>Gets the selected button.</summary>
   /// <returns></returns>
   LanguageButton*  LanguageDocument::GetSelectedButton() const
   {
      return CurrentButton;
   }

   /// <summary>Gets the selected page.</summary>
   /// <returns></returns>
   LanguagePage*  LanguageDocument::GetSelectedPage() const
   {
      return CurrentPage;
   }
   
   /// <summary>Gets the selected string.</summary>
   /// <returns></returns>
   LanguageString*  LanguageDocument::GetSelectedString() const
   {
      return CurrentString;
   }
   
   /// <summary>Inserts a Page into the file</summary>
   /// <param name="page">The Page.</param>
   /// <exception cref="Logic::ApplicationException">Page ID already in use</exception>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
   void  LanguageDocument::InsertPage(LanguagePage& page)
   {
      // Feedback
      Console << "Inserting document Page: " << page << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Insert into file
      UINT index = File.Insert(page);
      if (index == -1)
         throw ApplicationException(HERE, L"Unable to insert Page - the ID is already in use");
      
      // Update view
      GetView<LanguagePageView>()->InsertPage(index, page, true);
   }

   /// <summary>Inserts a string into the appropriate page</summary>
   /// <param name="str">The string.</param>
   /// <exception cref="Logic::ApplicationException">String ID already in use</exception>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
   void  LanguageDocument::InsertString(LanguageString& str)
   {
      // Feedback
      Console << "Inserting document string: " << str << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Select+Display page
      SelectedPageIndex = Content.IndexOf(str.Page);

      // Insert into languagePage
      UINT index = SelectedPage->Insert(str);
      if (index == -1)
         throw ApplicationException(HERE, L"Unable to insert string - the ID is already in use");
      
      // Update view
      GetView<LanguageStringView>()->InsertString(index, str, true);
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

      // Group: StringID/Author/ColourTag/Title/Version
      if (SelectedString)
      {
         CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(_T("String"));
         
         if (CurrentMode != EditMode::Source)
         {
            LanguageEdit& edit = GetView<LanguageEditView>()->GetEdit();

            group->AddSubItem(new LanguageStringView::IDProperty(*this, *SelectedPage, *SelectedString));
            group->AddSubItem(new LanguageEdit::AuthorProperty(*this, edit));
            group->AddSubItem(new LanguageEdit::ColumnTypeProperty(*this, edit));
            group->AddSubItem(new LanguageEdit::ColumnSpacingProperty(*this, edit));
            group->AddSubItem(new LanguageEdit::ColumnWidthProperty(*this, edit));
            group->AddSubItem(new LanguageStringView::ColourTagProperty(*this, *SelectedString));
            group->AddSubItem(new LanguageEdit::TitleProperty(*this, edit));
            group->AddSubItem(new LanguageStringView::VersionProperty(*this, *SelectedString));
         }
         else
         {
            group->AddSubItem(new LanguageStringView::IDProperty(*this, *SelectedPage, *SelectedString));
            group->AddSubItem(new LanguageStringView::VersionProperty(*this, *SelectedString));
         }
         
         grid.AddProperty(group);
      }

      // Group: ButtonID/ButtonText
      if (CurrentButton)
      {
         auto edit = GetView<LanguageEditView>();
         CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(_T("Button"));
         group->AddSubItem(new LanguageEditView::ButtonIDProperty(*this, *edit, *SelectedButton));
         group->AddSubItem(new LanguageEditView::ButtonTextProperty(*this, *edit, *SelectedButton));
         grid.AddProperty(group);
      }

      // Group: Library files
      if (Virtual)
      {
         group = new CMFCPropertyGridProperty(_T("Components"));
      
         // Enumerate files [backwards]
         for (auto file = StringLib.Files.crbegin(); file != StringLib.Files.crend(); ++file)   
            group->AddSubItem( new FileNameProperty(*this, *file, IsIncluded(file->ID)) );
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
   
   /// <summary>Manually set the path of the string library document</summary>
   /// <param name="deEvent">The de event.</param>
   void LanguageDocument::OnDocumentEvent(DocumentEvent deEvent)
   {
      // Manually set 'String Library' path
      if (Virtual && deEvent == onAfterOpenDocument)
         m_strPathName = L"String Library";
   }

   /// <summary>Initializes new document</summary>
   /// <returns></returns>
   BOOL LanguageDocument::OnNewDocument()
   {
	   if (!DocumentBase::OnNewDocument())
		   return FALSE;
      
      // Set default properties
      File.ID = 0;
      File.Language = GameLanguage::English;

#ifdef RESCINDED
      // Set default title, revert document to 'unmodified'
      Rename(File.ID, File.Language);
      SetModifiedFlag(FALSE);     
#endif

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
         Console << Cons::UserAction << L"Loading language file: " << Path(szPathName) << "...";
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
               File.Language = StringLib.Files.begin()->Language;
         }
         else
         {  
            // Parse input file
            XFileInfo stream(szPathName);
            File = LanguageFileReader(stream.OpenRead()).ReadFile(szPathName);
         }

         // Feedback
         Console << Cons::Success << ENDL;
         data.SendFeedback(ProgressType::Succcess, 0, L"Language file loaded successfully");
         return TRUE;
      }
      catch (ExceptionBase&  e)
      {
         // Feedback/Display error
         Console << Cons::Failure << e.Message << ENDL;
         data.SendFeedback(ProgressType::Failure, 0, L"Failed to load language file");
         theApp.ShowError(HERE, e, GuiString(L"Failed to load language file '%s'", szPathName));
         return FALSE;
      }
   }

   /// <summary>Saves contents to disc</summary>
   /// <param name="szPathName">Full path</param>
   /// <returns></returns>
   BOOL LanguageDocument::OnSaveDocument(LPCTSTR szPathName)
   {
      WorkerData data(Operation::LoadSaveDocument);
      
      try
      {
         // Feedback
         Console << Cons::UserAction << L"Saving language file: " << FullPath << " as " << Path(szPathName) << ENDL;
         data.SendFeedback(ProgressType::Operation, 0, GuiString(L"Saving language file '%s'", szPathName));

         // Verify not library
         if (GuiString(L"String Library") == szPathName)
            throw InvalidOperationException(HERE, L"Cannot save string library");
      
         // Write contents
         LanguageFileWriter w(XFileInfo(FullPath).OpenWrite());
         w.Write(File);
         w.Close();

         // Feedback
         data.SendFeedback(Cons::Success, ProgressType::Succcess, 0, L"Language file saved successfully");
         return TRUE;
      }
      catch (ExceptionBase&  e)
      {
         // Feedback/Display error
         data.SendFeedback(Cons::Error, ProgressType::Failure, 0, L"Failed to save language file");
         theApp.ShowError(HERE, e, GuiString(L"Failed to save language file '%s'", szPathName));
         return FALSE;
      }
   }

   /// <summary>Renames the document.</summary>
   /// <param name="id">File ID.</param>
   /// <param name="lang">language.</param>
   void  LanguageDocument::Rename(UINT id, GameLanguage lang)
   {
      // Change path + title
      __super::Rename(GuiString(L"%04d-L0%02d%s", File.ID, (UINT)File.Language, FullPath.Extension.c_str()));
   }
   
   /// <summary>Changes the ID of a Page.</summary>
   /// <param name="str">The Page.</param>
   /// <param name="newID">The new ID</param>
   /// <exception cref="Logic::ApplicationException">New Page ID already in use</exception>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   void  LanguageDocument::RenamePage(LanguagePage& page, UINT newID)
   {
      // Feedback
      Console << "Renaming document Page: " << page << Cons::White << " newID=" << newID << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Validate new ID
      if (!File.IsAvailable(newID))
         throw ApplicationException(HERE, L"Unable to change Page ID - it is already in use");

      // Generate Page 
      LanguagePage newPage(page);
      newPage.ID = newID;

      // Remove/Re-Insert
      RemovePage(page.ID);
      InsertPage(newPage);
   }

   /// <summary>Changes the ID of a string.</summary>
   /// <param name="str">The string.</param>
   /// <param name="newID">The new ID</param>
   /// <exception cref="Logic::ApplicationException">New String ID already in use</exception>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   void  LanguageDocument::RenameString(LanguageString& str, UINT newID)
   {
      // Feedback
      Console << "Renaming document string: " << str << Cons::White << " newID=" << newID << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Validate new ID
      if (!Content.Find(str.Page).IsAvailable(newID))
         throw ApplicationException(HERE, L"Unable to change string ID - it is already in use");

      // Generate string (must identify colour tags before display)
      LanguageString newStr(newID, str.Page, str.Text, str.Version);
      newStr.IdentifyColourTags();

      // Remove/Re-Insert
      RemoveString(str.Page, str.ID);
      InsertString(newStr);
   }
   
   /// <summary>Removes a page</summary>
   /// <param name="page">Page ID.</param>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
   void  LanguageDocument::RemovePage(UINT page)
   {
      // Feedback
      Console << "Removing document page: id=" << page << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Select+Display page
      //SelectedPageIndex = Content.IndexOf(page);

      // Remove from view first
      GetView<LanguagePageView>()->RemovePage(File.IndexOf(page));

      // Remove from languagePage
      File.Remove(page);
   }
   
   /// <summary>Removes the string from the appropriate page</summary>
   /// <param name="page">Page ID.</param>
   /// <param name="id">string ID.</param>
   /// <exception cref="Logic::InvalidOperationException">Document is virtual</exception>
   /// <exception cref="Logic::PageNotFoundException">Page does not exist</exception>
   /// <exception cref="Logic::StringNotFoundException">String does not exist</exception>
   void  LanguageDocument::RemoveString(UINT page, UINT id)
   {
      // Feedback
      Console << "Removing document string: page=" << page << " id=" << id << ENDL;

      // Ensure not library
      if (Virtual)
         throw InvalidOperationException(HERE, L"Cannot alter virtual documents");

      // Select+Display page
      SelectedPageIndex = Content.IndexOf(page);

      // Remove from languagePage
      UINT index = SelectedPage->Remove(id);

      // Update view
      GetView<LanguageStringView>()->RemoveString(index);
   }
   
   /// <summary>Changes the edit mode.</summary>
   /// <param name="m">mode.</param>
   /// <exception cref="Logic::AlgorithmException">Error in richText parsing algorithm</exception>
   /// <exception cref="Logic::ArgumentException">Error in richText parsing algorithm</exception>
   /// <exception cref="Logic::NotImplementedException">Mode is DISPLAY</exception>
   /// <exception cref="Logic::Language::RichTextException">Error in richText formatting tags</exception>
   void  LanguageDocument::SetEditMode(EditMode m)
   {
      if (m == GetEditMode())
         return;

      // Preserve edit mode
      auto prev = GetEditMode();

      try
      {
         // Raise 'EDIT MODE CHANGED'
         Mode = m;
         EditModeChanged.Raise();

         // Refresh properties
         CPropertiesWnd::Connect(this, true); 
      }
      // Error: Revert mode
      catch (ExceptionBase&) {
         Mode = prev;
         throw;
      }
   }

   /// <summary>Sets the selected button.</summary>
   /// <param name="b">The button.</param>
   void  LanguageDocument::SetSelectedButton(LanguageButton* b)
   {
      if (CurrentButton != b)
      {
         // Display properties
         CurrentButton = b;
         CPropertiesWnd::Connect(this, true);
      }
   }

   /// <summary>Sets the selected page and raises PAGE SELECTION CHANGED.</summary>
   /// <param name="p">The page.</param>
   /// <remarks>Also clears the current button</remarks>
   void  LanguageDocument::SetSelectedPage(LanguagePage* p)
   {
      if (CurrentPage != p)
      {
         // Set page.  Clear button before updating properties
         CurrentPage = p;
         CurrentButton = nullptr;

         // Display properties. Raise PAGE SELECTION CHANGED
         CPropertiesWnd::Connect(this, true);
         PageSelectionChanged.Raise();
      }
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
   /// <remarks>Also clears the current button</remarks>
   void  LanguageDocument::SetSelectedString(LanguageString* s)
   {
      if (CurrentString != s)
      {
         // Set page.  Clear button before updating properties
         CurrentString = s;
         CurrentButton = nullptr;

         // Raise STRING SELECTION CHANGED first, so LanguageEdit can parse the rich-text
         StringSelectionChanged.Raise();

         // Display string properties 
         CPropertiesWnd::Connect(this, true);
      }
   }

   /// <summary>Sets the text of the selected string.  Raises STRING UPDATED.</summary>
   /// <param name="txt">The text.</param>
   void  LanguageDocument::SetSelectedStringText(const wstring& txt)
   {
      REQUIRED(SelectedString);

      // Update text. (Raise STRING UPDATED)
      SelectedString->Text = txt;
      StringUpdated.Raise();

      // Update string properties
      CPropertiesWnd::Connect(this, true);
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

      // Raise 'LIBRARY REBUILT'
      LibraryRebuilt.Raise();
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


