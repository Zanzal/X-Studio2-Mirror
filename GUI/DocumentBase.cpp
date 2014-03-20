#include "stdafx.h"
#include "DocumentBase.h"
#include "MainWnd.h"

/// <summary>User interface documents</summary>
NAMESPACE_BEGIN2(GUI,Documents)

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNCREATE(DocumentBase, CDocument)

   BEGIN_MESSAGE_MAP(DocumentBase, CDocument)
      ON_COMMAND(ID_FILE_CLOSE, &DocumentBase::OnCommand_Close)
      ON_COMMAND(ID_FILE_SAVE, &DocumentBase::OnCommand_Save)
      ON_COMMAND(ID_FILE_SAVE_AS, &DocumentBase::OnCommand_SaveAs)
      ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &DocumentBase::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, &DocumentBase::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, &DocumentBase::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_ALL, &DocumentBase::OnQueryCommand)
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   DocumentBase::DocumentBase() : IsVirtual(false)
   {
   }

   DocumentBase::DocumentBase(DocumentType type, bool isVirtual) : Type(type), IsVirtual(isVirtual)
   {
   }

   DocumentBase::~DocumentBase()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   /// <summary>Gets the active script or language document</summary>
   /// <returns></returns>
   DocumentBase*  DocumentBase::GetActive()
   {
      CMDIChildWnd* pChild = theApp.GetMainWindow()->MDIGetActive();

      if (!pChild)
         return nullptr;

      return (DocumentBase*)pChild->GetActiveDocument();
   }
   
   /// <summary>Renames a file on disc.</summary>
   /// <param name="oldPath">The old path.</param>
   /// <param name="newPath">The new path.</param>
   /// <exception cref="Logic::ApplicationException">New file path already exists -or- project already contains new path</exception>
   /// <exception cref="Logic::IOException">Unable to rename file</exception>
   void DocumentBase::RenameFile(Path oldPath, Path newPath)
   {
      auto proj = ProjectDocument::GetActive();

      // Ensure unique
      if (newPath.Exists())
         throw ApplicationException(HERE, L"A file with that name already exists");
      else if (proj && proj->Contains(newPath))
         throw ApplicationException(HERE, L"Project already contains a file with that path");

      // Rename file
      if (!MoveFileEx(oldPath.c_str(), newPath.c_str(), MOVEFILE_COPY_ALLOWED|MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
         throw IOException(HERE, SysErrorString());
   }
   
   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Activates this document</summary>
   void  DocumentBase::Activate()
   {
      auto pos = GetFirstViewPosition();

      // Activate parent frame of view (if any)
      if (auto view = GetNextView(pos))
         if (auto parent = view->GetParentFrame())
            parent->ActivateFrame();
   }
   
   /// <summary>Invoked by MFC to save the document.</summary>
   /// <param name="szPathName">Path to save under, or nullptr for 'Save As'</param>
   /// <param name="bReplace">TRUE to update project item, FALSE to preserve it</param>
   /// <returns>TRUE if saved, FALSE if cancelled/error</returns>
   BOOL DocumentBase::DoSave(LPCTSTR szPathName, BOOL /*bReplace*/)
   {
	   CString path = szPathName;
      auto proj = ProjectDocument::GetActive();

      try
      {
         // SaveAs/SaveCopyAs: 
         if (!szPathName)
         {
            // Query for new filename
            path = FullPath.c_str();
            if (!AfxGetApp()->DoPromptFileName(path, AFX_IDS_SAVEFILE, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST, FALSE, GetDocTemplate()))
               return FALSE;

            // SaveAs: Rename document
            /*if (bReplace)
               Rename((LPCWSTR)path, true);*/
         }

         // Save document
	      if (!OnSaveDocument(path))
	      {
            // SaveAs/SaveCopyAs: Delete partial file
		      if (!szPathName)
               DeleteFile(path);

		      return FALSE;
	      }

         // Preserve path
         auto oldPath = FullPath;

         // Update path/title/modified
         __super::m_strPathName = path;
         __super::SetModifiedFlag(FALSE);
         SetTitle(FullPath.FileName.c_str());

	      // Project: Update item name [Unless 'SaveAs']
         if (proj && proj->Contains(oldPath) && szPathName)
            proj->OnDocumentRenamed(*this, oldPath, TRUE);

         // Success: Raise 'After Save'
	      OnDocumentEvent(onAfterSaveDocument);
	      return TRUE;     
      }
      // Renaming failed
      catch (ExceptionBase& e) 
      {
         theApp.ShowError(HERE, ApplicationException(e), VString(L"Unable to save document '%s'", (LPCWSTR)path));
         return FALSE;
      }
   }

   /// <summary>Finds and highlights the next match, if any</summary>
   /// <param name="start">Starting offset</param>
   /// <param name="m">Match data</param>
   /// <returns>True if found, false otherwise</returns>
   bool  DocumentBase::FindNext(UINT start, MatchData& m) const
   {
      return false;
   }

   /// <summary>Gets the filename from the title, without the '*'</summary>
   /// <returns></returns>
   /// <remarks>If the file has been renamed the title represents the desired filename, whereas the fullpath contains the actual filename</remarks>
   wstring DocumentBase::GetFileName() const
   {
      wstring title = GetTitle();

      // Pop '*' if present
      if (!title.empty() && title.back() == '*')
         title.pop_back();

      return title;
   }

   /// <summary>Get the full document path.</summary>
   /// <returns></returns>
   Path  DocumentBase::GetFullPath() const
   {
      return Path((const wchar*)GetPathName());
   }
   
   /// <summary>Gets the text selection.</summary>
   /// <returns></returns>
   CHARRANGE  DocumentBase::GetSelection() const
   {
      return {0,0};
   }

   /// <summary>Get document type.</summary>
   /// <returns></returns>
   DocumentType  DocumentBase::GetType() const
   {
      return Type;
   }

   /// <summary>Get whether virtual.</summary>
   bool  DocumentBase::GetVirtual() const
   {
      return IsVirtual;
   }

   /// <summary>Called when opening a document from a template.</summary>
   /// <param name="docPath">New document path</param>
   /// <param name="t">file template.</param>
   /// <returns></returns>
   BOOL  DocumentBase::OnOpenTemplate(Path docPath, const TemplateFile& t)
   {
      // Default implementation uses open document
      return OnOpenDocument(AppPath(t.SubPath).c_str());
   }
   
   /// <summary>Renames the file on disc, updates the project item [if any] and updates the document path and title.</summary>
   /// <param name="newPath">New path.</param>
   /// <param name="overwriteExists">True to overwrite if file exists, false to fail if file exists.</param>
   /// <exception cref="Logic::ApplicationException">New path already exists, or project already contains new path</exception>
   /// <exception cref="Logic::IOException">Unable to rename file</exception>
   void  DocumentBase::Rename(Path newPath, bool /*overwriteExists*/)
   {
      auto proj = ProjectDocument::GetActive();

      // Ensure path different
      if (FullPath == newPath)
         return;

      // Ensure document exists
      if (Virtual)
         throw AlgorithmException(HERE, L"Cannot rename virtual documents");

      // Project Member: Ensure new path unique within project
      if (proj && proj->Contains(FullPath) && proj->Contains(newPath))
         throw ApplicationException(HERE, L"Current project already contains a file with that name");

      // Ensure new path does not exist
      /*if (!overwriteExists && newPath.Exists())
         throw ApplicationException(HERE, L"A file with that name already exists");*/

      // Modified:
      if (IsModified())
      {
         // Update title only
         SetTitle(newPath.FileName.c_str());

         // Project: Update item name only
         if (proj && proj->Contains(FullPath))
            proj->OnDocumentRenamed(*this, FullPath, FALSE);
      }
      // Unmodified:
      else
      {
         // Attempt to Rename file
         RenameFile(FullPath, newPath);

         // Update document path [and title]
         auto oldPath = FullPath;
         FullPath = newPath;

         // Project: Update item name & path
         if (proj && proj->Contains(oldPath))
            proj->OnDocumentRenamed(*this, oldPath, TRUE);
      }
   }

   /// <summary>Replaces the current match</summary>
   /// <param name="m">Match data</param>
   /// <returns>True if replaced, false if match was no longer selected</returns>
   bool  DocumentBase::Replace(MatchData& m)
   {
      return false;
   }
   
   /// <summary>Invoked by MFC to save the modified document before closing.</summary>
   /// <returns>TRUE to continue closing, FALSE to abort</returns>
   BOOL DocumentBase::SaveModified()
   {
	   if (!IsModified())
		   return TRUE;        // ok to continue

      // Query save
      VString msg(L"The document '%s' has been modified, save changes?", FileName.c_str());
      switch (theApp.ShowMessage(msg, MB_ICONQUESTION|MB_YESNOCANCEL))
      {
      // Cancel: Abort closing
      case IDCANCEL:
		   return FALSE;

      // Yes: Attempt saving
      case IDYES:
		   if (!DoSave((FullPath.Folder+FileName).c_str(), TRUE))
			   return FALSE;       // Failed: Abort closing
		   break;
      }

      // Success: Close document
	   return TRUE;    
   }

   /// <summary>Changes the path of the document.</summary>
   /// <returns></returns>
   void  DocumentBase::SetFullPath(Path path) 
   {
      // Change path. (Don't add to MRU - will cause argument exception if file doesn't exist)
      SetPathName(path.c_str(), FALSE);

      // Update title
      SetTitle(path.FileName.c_str());
   }

   /// <summary>Sets the modified flag and updates the document title</summary>
   /// <param name="bModified">modified flag.</param>
   void  DocumentBase::SetModifiedFlag(BOOL bModified)
   {
      // Update flag
      __super::SetModifiedFlag(bModified);

      // Get title
      wstring title = (LPCWSTR)GetTitle();
      if (title.empty())
         return;

      // Add/Remove trailing '*'
      if (bModified && title.back() != '*')
         title += L'*';
      else if (!bModified && title.back() == '*')
         title.pop_back();

      // Update title
      __super::SetTitle(title.c_str());
   }

   /// <summary>Used by MFC to set document path: onCreate + onOpen + onSaveAs</summary>
   /// <param name="szPathName">New path.</param>
   /// <param name="bAddToMRU">Add to MRU [Path must exist].</param>
   void  DocumentBase::SetPathName(LPCTSTR szPathName, BOOL bAddToMRU /*= TRUE*/)
   {
      __super::SetPathName(szPathName, bAddToMRU);
   }

   /// <summary>Sets the selection.</summary>
   /// <param name="rng">char range.</param>
   void  DocumentBase::SetSelection(CHARRANGE rng)
   {
   }

   /// <summary>Sets the title, and adds a trailing '*' if document is modified</summary>
   /// <param name="title">title.</param>
   void  DocumentBase::SetTitle(LPCTSTR title) 
   {
      wstring txt(title);

      // Modified: Append *
      if (IsModified())
         txt += L"*";

      // Set title
      __super::SetTitle(txt.c_str());
   }
   
   /// <summary>Sets whether virtual.</summary>
   /// <param name="v">state.</param>
   void  DocumentBase::SetVirtual(bool v)
   {
      IsVirtual = v;
   }

   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Query to save if modified</summary>
   void  DocumentBase::OnCommand_Close()
   {
      // Save Modified
      if (!SaveModified())
		   return;

      // Close doc
	   OnCloseDocument();
   }

   /// <summary>Save under existing path</summary>
   void  DocumentBase::OnCommand_Save()
   {
      // Save document under existing or modified path
      DoSave((FullPath.Folder+FileName).c_str(), TRUE);
   }

   /// <summary>Query for new path, rename document, and save.</summary>
   void  DocumentBase::OnCommand_SaveAs()
   {
      // Prompt for filename, save copy
      DoSave(nullptr, FALSE);

      //CString path(FullPath.FileName.c_str());

      //try
      //{
      //   // Query for new filename
      //   if (!AfxGetApp()->DoPromptFileName(path, AFX_IDS_SAVEFILE, OFN_HIDEREADONLY|OFN_PATHMUSTEXIST, FALSE, GetDocTemplate()))
			   //return;

      //   // Attempt to Rename document
      //   Rename((LPCWSTR)path, false);

      //   // Save document
      //   OnSaveDocument(FullPath.c_str());
      //}
      //catch (ExceptionBase& e) {
      //   theApp.ShowError(HERE, e, VString(L"Unable to save document to '%s'", (LPCWSTR)path));
      //}
   }

   /// <summary>Queries the state of a menu command.</summary>
   /// <param name="pCmdUI">The command UI.</param>
   void  DocumentBase::OnQueryCommand(CCmdUI* pCmdUI)
   {
      bool state = false;

      switch (pCmdUI->m_nID)
      {
      // Close: Always enabled
      case ID_FILE_CLOSE:    state = true;       break;

      // Save/SaveAs: Require file document
      case ID_FILE_SAVE:
      case ID_FILE_SAVE_AS:  state = !Virtual;   break;

      // SaveAll: Always enabled
      case ID_FILE_SAVE_ALL: state = true;       break;
      }

      // Set state
      pCmdUI->Enable(state ? TRUE : FALSE);
      pCmdUI->SetCheck(FALSE);
   }
   
   // ------------------------------- PRIVATE METHODS ------------------------------
   
   
/// <summary>User interface documents</summary>
NAMESPACE_END2(GUI,Documents)
