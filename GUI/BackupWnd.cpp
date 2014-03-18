#include "stdafx.h"
#include "BackupWnd.h"
#include "MainWnd.h"
#include "ScriptDocument.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Windows)

   // --------------------------------- APP WIZARD ---------------------------------

   BEGIN_MESSAGE_MAP(BackupWnd, CDockablePane)
	   ON_WM_CREATE()
	   ON_WM_SIZE()
      ON_WM_PAINT()
      ON_WM_SETFOCUS()
	   ON_WM_SETTINGCHANGE()
      ON_WM_DRAWITEM()
      ON_WM_MEASUREITEM()
      ON_WM_CONTEXTMENU()
      ON_COMMAND(ID_BACKUP_DIFF, &BackupWnd::OnCommandDiff)
      ON_COMMAND(ID_BACKUP_REVERT, &BackupWnd::OnCommandRevert)
      ON_COMMAND(ID_BACKUP_DELETE, &BackupWnd::OnCommandDelete)
      ON_UPDATE_COMMAND_UI(ID_BACKUP_COMMIT, &BackupWnd::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_BACKUP_DIFF, &BackupWnd::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_BACKUP_QUICK_COMMIT, &BackupWnd::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_BACKUP_REVERT, &BackupWnd::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_BACKUP_DELETE, &BackupWnd::OnQueryCommand)
      ON_WM_CTLCOLOR()
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   BackupWnd::BackupWnd() 
      : fnDocumentSwitched(MainWnd::DocumentSwitched.Register(this, &BackupWnd::OnDocumentSwitched)),
        fnBackupChanged(ProjectDocument::BackupChanged.Register(this, &BackupWnd::OnBackupChanged)),
        fnProjectClosed(ProjectDocument::Closed.Register(this, &BackupWnd::OnProjectClosed)),
        fnProjectLoaded(ProjectDocument::Loaded.Register(this, &BackupWnd::OnProjectLoaded)),
        Backup(BackupType::MSCI)
   {
   }

   BackupWnd::~BackupWnd()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Creates the window.</summary>
   /// <param name="parent">The parent.</param>
   /// <exception cref="Logic::Win32Exception">Unable to create window</exception>
   void BackupWnd::Create(CWnd* parent)
   {
      DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_RIGHT | CBRS_FLOAT_MULTI;

      // Create window
      if (!__super::Create(GuiString(IDR_BACKUP).c_str(), parent, MainWnd::DefaultSize, TRUE, IDR_BACKUP, style))
	      throw Win32Exception(HERE, L"Unable to create backup window");

      SetIcon(theApp.LoadIconW(IDR_BACKUP, ::GetSystemMetrics(SM_CXSMICON)), FALSE);

      // Dock left/right
      EnableDocking(CBRS_ORIENT_VERT|CBRS_FLOAT_MULTI);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------

   /// <summary>Adjusts the layout.</summary>
   void BackupWnd::AdjustLayout()
   {
      // Destroyed/Minimised
	   if (!GetSafeHwnd() || theApp.IsMimized())
         return;

	   ClientRect wnd(this);

      // Toolbar height
	   int barHeight = ToolBar.CalcFixedLayout(FALSE, TRUE).cy;

      // Snap toolbar to top, stretch list over remainder
	   ToolBar.SetWindowPos(nullptr, wnd.left, wnd.top, wnd.Width(), barHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	   List.SetWindowPos(nullptr, wnd.left+1, wnd.top+barHeight+1, wnd.Width()-2, wnd.Height()-barHeight-2, SWP_NOACTIVATE | SWP_NOZORDER);
   }
   
   /// <summary>Draws an item or calculates the necessary space.</summary>
   /// <param name="dc">The dc.</param>
   /// <param name="rc">item rect (drawing) or client rect (measuring).</param>
   /// <param name="index">zero-based index, or -1 if box is empty.</param>
   /// <param name="state">item state.</param>
   /// <param name="calculate">calculate only.</param>
   void BackupWnd::DrawItem(CDC& dc, CRect& rc, int index, UINT state, bool calculate)
   {
      // Empty: Draw focus only
      if (index == -1)
         dc.DrawFocusRect(rc);
      else
      {
         // Prepare
         auto& item     = Backup.Revisions.FindByIndex(index);
         bool  Selected = (state & ODS_SELECTED) != 0, 
               Focused  = (state & ODS_FOCUS) != 0;   
         auto  font     = dc.SelectObject(List.GetFont());
         auto  itemRect = rc;

         // Left/Right edge
         rc.DeflateRect(GetSystemMetrics(SM_CXEDGE), 0);

         // Background:
         if (!calculate)
         {
            dc.FillSolidRect(itemRect, GetSysColor(Selected?COLOR_HIGHLIGHT:COLOR_WINDOW));
            dc.SetTextColor(GetSysColor(Selected?COLOR_HIGHLIGHTTEXT:COLOR_WINDOWTEXT));   
         }

         // Icon:
         if (!calculate)
            Images.Draw(dc, GameVersionIndex(item.Game).Index, CRect(rc.left, rc.top, rc.left+24, rc.bottom), CSize(24,24), Selected ? ODS_SELECTED : NULL);
         rc.left += 24 + GetSystemMetrics(SM_CXEDGE);

         // Version/Date:
         dc.SelectObject(&BoldFont);
         if (!calculate)
         { 
            dc.DrawText(VString(L"Version: %d", item.Version).c_str(), rc, DT_LEFT|DT_TOP);      // Version xyz
            dc.DrawText(item.Date.Format(L"%d %b '%y %I:%M%p"), rc, DT_RIGHT|DT_TOP);              // dd mmm yy hh:mm pm
         }
         rc.top += dc.GetTextExtent(L"ABC").cy;

         // Title:
         dc.SelectObject(List.GetFont());
         rc.top += GetSystemMetrics(SM_CYEDGE);
         dc.DrawText(item.Title.c_str(), rc, DT_LEFT|DT_WORDBREAK|(calculate?DT_CALCRECT:0));

         // Calculate: Return height of version/date + title
         if (calculate)
         {
            rc.top = itemRect.top;
            rc.bottom += GetSystemMetrics(SM_CYEDGE);
         }
         // Focus/Separator:
         else if (Focused)
            dc.DrawFocusRect(itemRect);
         else 
            dc.DrawEdge(itemRect, EDGE_ETCHED, BF_BOTTOM);
         
         // Cleanup
         dc.SelectObject(font);
      }
   }
   
   /// <summary>Re-Loads backups for the active document</summary>
   void BackupWnd::OnBackupChanged(ProjectItem* item)
   {
      Populate();
   }
   

   /// <summary>Create child controls</summary>
   /// <param name="lpCreateStruct">create params</param>
   /// <returns></returns>
   int BackupWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
   {
      try
      {
	      if (__super::OnCreate(lpCreateStruct) == -1)
		      throw Win32Exception(HERE, L"Failed to create dockable pane");

	      CRect rectDummy;
	      rectDummy.SetRectEmpty();
         
         // Toolbar
         ToolBar.Create(this, IDR_BACKUP, L"Backup");

         // Create List  [Disable until population]
         DWORD style = WS_DISABLED|WS_VISIBLE|WS_CHILD|WS_VSCROLL|LBS_OWNERDRAWVARIABLE|LBS_NOTIFY;   //LBS_DISABLENOSCROLL
	      if (!List.Create(style, rectDummy, this, IDC_BACKUP_LIST))
	         throw Win32Exception(HERE, L"Failed to create backup list");

         // ImageList
         Images.Create(IDB_BACKUP_ICONS, 24, 3, RGB(255,0,255));
         
         // Layout
         UpdateFont();
	      AdjustLayout();
	      return 0;
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
         return -1;
      }
   }
   
   /// <summary>Called when context menu.</summary>
   /// <param name="pWnd">The p WND.</param>
   /// <param name="">The .</param>
   void BackupWnd::OnContextMenu(CWnd* pWnd, CPoint point)
   {
      // Ensure List was clicked
      if (pWnd && List.m_hWnd == pWnd->m_hWnd)
      {
         BOOL none = FALSE;
          
         // Select item beneath cursor
         int item = List.ItemFromPoint(CursorPoint(&List), none);
         List.SetCurSel(!none ? item : LB_ERR);

         // Popup
         theApp.GetContextMenuManager()->ShowPopupMenu(IDM_BACKUP_POPUP, point.x, point.y, this, TRUE);
      }
   }
   
   /// <summary>Set list background colour.</summary>
   /// <param name="pDC">The p dc.</param>
   /// <param name="pWnd">The p WND.</param>
   /// <param name="nCtlColor">Color of the n control.</param>
   /// <returns></returns>
   HBRUSH BackupWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
   {
      // ListBackground: Grey on disabled
      if (nCtlColor == CTLCOLOR_LISTBOX)
         return GetSysColorBrush(List.IsWindowEnabled() ? COLOR_WINDOW : COLOR_3DSHADOW);

      // Default
      return __super::OnCtlColor(pDC, pWnd, nCtlColor);
   }

   /// <summary>Loads backups for the active document</summary>
   void BackupWnd::OnDocumentSwitched()
   {
      // Ignore excessive calls before documents exist
      if (!List.GetSafeHwnd())    
         return;

      auto p = ProjectDocument::GetActive();
      auto doc = ScriptDocument::GetActive();

      // Warn user if backup file is missing
      if (p && doc && p->Contains(*doc) && !p->HasBackup(*doc))
      {
         auto msg = VString(L"The backup file '%s' is missing, revisions are not available.", p->GetBackupPath(*doc).c_str());
         Console << Cons::Warning << msg << ENDL;
         theApp.ShowMessage(msg, MB_OK|MB_ICONERROR);
      }

      // Populate for active document
      Populate();
   }
   

   /// <summary>Owner draw items.</summary>
   /// <param name="id">The identifier.</param>
   /// <param name="draw">The draw.</param>
   void BackupWnd::OnDrawItem(int id, LPDRAWITEMSTRUCT draw)
   {
      try
      {
         SharedDC dc(draw->hDC);
         CRect    rc(draw->rcItem);

         // Draw item
         DrawItem(dc, rc, draw->itemID, draw->itemState, false);
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }
   }

   /// <summary>Called when measure item.</summary>
   /// <param name="id">The identifier.</param>
   /// <param name="measure">The measure.</param>
   void BackupWnd::OnMeasureItem(int id, LPMEASUREITEMSTRUCT measure)
   {
      try
      {
         ClientRect rc(this);
         CClientDC  dc(this);

         // Measure item
         DrawItem(dc, rc, measure->itemID, NULL, true);

         // Set width/height
         measure->itemHeight = rc.Height();
         measure->itemWidth = rc.Width();
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }
   }

   /// <summary>Manually paints border around grid.</summary>
   void BackupWnd::OnPaint()
   {
	   CPaintDC dc(this); // device context for painting
      CtrlRect rc(this, &List);

      // Paint border
	   rc.InflateRect(1, 1);
	   dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
   }
   
   /// <summary>Perform command</summary>
   /// <param name="nID">ID</param>
   void BackupWnd::OnPerformCommand(UINT nID)
   {
      auto script = ScriptDocument::GetActive();
      
      // Require document/selection
      if (!script || List.GetCurSel() == LB_ERR)
         return;

      // Lookup revision
      auto rev = Backup[List.GetCurSel()];
      switch (nID)
      {
      // Display diff document with selected revision
      case ID_BACKUP_DIFF: 
         theApp.OpenDiffDocument(*script, rev.Content);  
         break;

      // Delete selected revision
      case ID_BACKUP_DELETE: 
         if (theApp.ShowMessage(L"Are you sure you want to delete this revision?", MB_YESNO|MB_ICONQUESTION) == IDYES)
            if (auto proj = ProjectDocument::GetActive())   // should always exist
               proj->DeleteRevision(*script, List.GetCurSel());
         break;

      // Replace document contents with revision
      case ID_BACKUP_REVERT:  
         if (theApp.ShowMessage(L"Are you sure you want to revert document to this revision?", MB_YESNO|MB_ICONQUESTION) == IDYES)
            script->OnRevertDocument(rev);
         break;
      }
   }

   /// <summary>Clears currently displayed backups (if any)</summary>
   void BackupWnd::OnProjectClosed()
   {
      Populate();
   }

   /// <summary>Loads backups for the active document (if any)</summary>
   void BackupWnd::OnProjectLoaded()
   {
      Populate();
   }

   /// <summary>Query state of context menu command</summary>
   /// <param name="pCmdUI">UI object</param>
   void BackupWnd::OnQueryCommand(CCmdUI *pCmdUI)
   {
      BOOL state = TRUE, 
           check = FALSE;

      //
      switch (pCmdUI->m_nID)
      {
      case ID_BACKUP_COMMIT:
      case ID_BACKUP_DELETE:  
      case ID_BACKUP_DIFF:    
      case ID_BACKUP_REVERT:  
      case ID_BACKUP_QUICK_COMMIT:
         state = (List.GetCurSel() != LB_ERR ? TRUE : FALSE);
         break;
      }

      // Set state
      pCmdUI->SetCheck(check);
      pCmdUI->Enable(state);
   }
   

   
   /// <summary>Set focus to properties grid</summary>
   /// <param name="pOldWnd">The p old WND.</param>
   void BackupWnd::OnSetFocus(CWnd* pOldWnd)
   {
      __super::OnSetFocus(pOldWnd);
	   List.SetFocus();
   }

   /// <summary>Update font when settings change.</summary>
   /// <param name="uFlags">The flags.</param>
   /// <param name="lpszSection">The section.</param>
   void BackupWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
   {
	   __super::OnSettingChange(uFlags, lpszSection);

      // Update font, Adjust layout
	   UpdateFont();
      AdjustLayout();

      // Refresh contents
      Refresh();
   }

   /// <summary>Adjusts the layout</summary>
   /// <param name="nType">Type of the n.</param>
   /// <param name="cx">The width.</param>
   /// <param name="cy">The height.</param>
   void BackupWnd::OnSize(UINT nType, int cx, int cy)
   {
	   __super::OnSize(nType, cx, cy);

      // Adjust layout
	   AdjustLayout();

      // Refresh contents [Forces WM_MEASUREITEM to be sent]
      if (List.GetSafeHwnd())    // Does not exist on first WM_SIZE
         Refresh();
   }
   
   /// <summary>Loads the appropriate backup file and populates items.</summary>
   void BackupWnd::Populate()
   {
      auto proj = ProjectDocument::GetActive();
      auto doc = ScriptDocument::GetActive();

      // Clear
      List.SetRedraw(FALSE);
      List.ResetContent();
      Backup.Clear();

      try
      {
         // Check document (if any) belongs to current project (if any)
         if (doc && proj && proj->Contains(doc->FullPath))
         {
            // Load backup
            Backup = proj->LoadBackupFile(*doc);
            List.EnableWindow(TRUE);

            // Fill list with dummy items
            for (auto& rev : Backup.Revisions)
               List.InsertString(-1, L"-");
         }
         else
            List.EnableWindow(FALSE);
      }
      catch (ExceptionBase& e) 
      {
         // Backup file missing/corrupt: Fail silently. Display nothing
         Backup.Clear();
         List.ResetContent();
         List.EnableWindow(FALSE);
         Console.Log(HERE, e);
      }

      // Redraw
      List.SetRedraw(TRUE);
      List.UpdateWindow();
   }
   
   /// <summary>Re-Populates items from current backup file.</summary>
   void BackupWnd::Refresh()
   {
      auto proj = ProjectDocument::GetActive();
      auto doc = ScriptDocument::GetActive();

      // Clear
      List.SetRedraw(FALSE);
      List.ResetContent();

      // Fill list with dummy items
      for (auto& rev : Backup.Revisions)
         List.InsertString(-1, L"-");

      // Redraw
      List.SetRedraw(TRUE);
      List.UpdateWindow();
   }
   
   /// <summary>Updates the font.</summary>
   void BackupWnd::UpdateFont()
   {
	   List.SetFont(&theApp.ToolWindowFont);

      // Generate bold font
      LOGFONT lf;
      theApp.ToolWindowFont.GetLogFont(&lf);
      lf.lfWeight = FW_BOLD;

      // Create
      BoldFont.DeleteObject();
      BoldFont.CreateFontIndirectW(&lf);
   }
   
   
   // ------------------------------- PRIVATE METHODS ------------------------------

   

NAMESPACE_END2(GUI,Windows)


