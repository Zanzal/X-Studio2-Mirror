#include "stdafx.h"
#include "LanguageStringView.h"
#include "PropertiesWnd.h"
#include "Logic/StringResolver.h"
#include "Logic/RichStringParser.h"
#include "RichTextRenderer.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Views)

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNCREATE(LanguageStringView, CListView)

   BEGIN_MESSAGE_MAP(LanguageStringView, CListView)
      ON_WM_SIZE()
      ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &LanguageStringView::OnItemStateChanged)
      ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &LanguageStringView::OnCustomDraw)
      ON_COMMAND(ID_EDIT_CUT, &LanguageStringView::OnCommandEditCut)
      ON_COMMAND(ID_EDIT_COPY, &LanguageStringView::OnCommandEditCopy)
      ON_COMMAND(ID_EDIT_PASTE, &LanguageStringView::OnCommandEditPaste)
      ON_COMMAND(ID_EDIT_CLEAR, &LanguageStringView::OnCommandEditClear)
      ON_COMMAND(ID_EDIT_SELECT_ALL, &LanguageStringView::OnCommandEditSelectAll)
      ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, &LanguageStringView::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, &LanguageStringView::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, &LanguageStringView::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, &LanguageStringView::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_FIND, &LanguageStringView::OnQueryCommand)
      ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, &LanguageStringView::OnQueryCommand)
      ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &LanguageStringView::OnRetrieveItem)
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   LanguageStringView::LanguageStringView() : CustomDraw(this)
   {
      m_dwDefaultStyle |= LVS_SHOWSELALWAYS|LVS_SINGLESEL|LVS_OWNERDATA;
   }

   LanguageStringView::~LanguageStringView()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Gets the document.</summary>
   /// <returns></returns>
   LanguageDocument* LanguageStringView::GetDocument() const 
   {
#ifdef _DEBUG
	   return dynamic_cast<LanguageDocument*>(m_pDocument);
#else
      return reinterpret_cast<LanguageDocument*>(m_pDocument);
#endif
   }
   
   /// <summary>Inserts a string.</summary>
   /// <param name="index">The index.</param>
   /// <param name="str">The string.</param>
   void LanguageStringView::InsertString(UINT index, LanguageString& str)
   {
      throw InvalidOperationException(HERE, L"");

      // First display: Identify colour tags 
      if (str.TagType == ColourTag::Undetermined)
         str.IdentifyColourTags();

      // Add item, resolve and set Text
      GetListCtrl().InsertItem(index, GuiString(L"%d", str.ID).c_str(), 2+GameVersionIndex(str.Version).Index);
      GetListCtrl().SetItemText(index, 1, str.ResolvedText.c_str());
   }

   /// <summary>Removes a string.</summary>
   /// <param name="index">The index.</param>
   void LanguageStringView::RemoveString(UINT index)
   {
      throw InvalidOperationException(HERE, L"");

      GetListCtrl().DeleteItem(index);
   }

   /// <summary>Called when activate view.</summary>
   /// <param name="bActivate">activated.</param>
   /// <param name="pActivateView">The activate view.</param>
   /// <param name="pDeactiveView">The deactive view.</param>
   void LanguageStringView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
   {
      // Show properties
      if (bActivate)
         CPropertiesWnd::Connect(GetDocument(), true); 

      __super::OnActivateView(bActivate, pActivateView, pDeactiveView);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Arrange controls</summary>
   void  LanguageStringView::AdjustLayout()
   {
      // Destroyed/Minimised
	   if (GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
         return;
         
      CRect wnd;
      GetClientRect(wnd);

      // TODO: Layout code
      GetListCtrl().SetColumnWidth(1, wnd.Width()-GetListCtrl().GetColumnWidth(0));  //LVSCW_AUTOSIZE_USEHEADER); 
   }
   
   /// <summary>Retrieves the language string representing the current selection.</summary>
   /// <returns>Selected string if any, otherwise nullptr</returns>
   /// <exception cref="Logic::IndexOutOfRangeException">Selected item index is invalid</exception>
   LanguageString*   LanguageStringView::GetSelected() const
   {
      int item = GetListCtrl().GetNextItem(-1, LVNI_SELECTED);

      // Return item/null
      return item != -1 ? &GetDocument()->SelectedPage->FindByIndex(item) : nullptr;
   }
   
   /// <summary>Custom draw the strings</summary>
   /// <param name="pNMHDR">header.</param>
   /// <param name="pResult">result.</param>
   void LanguageStringView::OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
   {
      NMCUSTOMDRAW* pDraw = reinterpret_cast<NMCUSTOMDRAW*>(pNMHDR);
      *pResult = CustomDraw.OnCustomDraw(pDraw);
   }

   
   /// <summary>Custom draws the text column</summary>
   /// <param name="dc">Device context</param>
   /// <param name="item">Item data.</param>
   void  LanguageStringView::StringCustomDraw::onDrawSubItem(CDC* dc, ItemData& item) 
   {
      try
      {
         // ID:
         if (item.SubItem == 0)
            __super::onDrawSubItem(dc, item);
         
         // Text:
         else if (item.SubItem == 1)
         {
            auto src = ListView.GetItemText(item.Index, 1);
            auto flags = item.Selected ? RenderFlags::Selected : RenderFlags::Inverted;

            RichStringParser parser((const wchar*)src);
            RichTextRenderer::DrawLine(dc, item.Rect, parser.Output, flags);
         }
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }
   }

   /// <summary>Initialise control</summary>
   void LanguageStringView::OnInitialUpdate()
   {
      __super::OnInitialUpdate();

      try
      {
         // Icons
         Images.Create(IDB_LANGUAGE_ICONS, 16, 6, RGB(255,0,255));
	      
         // Setup listView
         GetListCtrl().ModifyStyle(WS_BORDER, 0);
         GetListCtrl().SetView(LV_VIEW_DETAILS);
         GetListCtrl().SetItemCountEx(0);
         GetListCtrl().InsertColumn(0, L"ID", LVCFMT_LEFT, 60, 0);
         GetListCtrl().InsertColumn(1, L"Text", LVCFMT_LEFT, 240, 1);
         GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT);
         GetListCtrl().SetImageList(&Images, LVSIL_SMALL);

         // Listen for Page content/selection changed
         fnPageSelectionChanged = GetDocument()->PageSelectionChanged.Register(this, &LanguageStringView::onPageSelectionChanged);
         fnPageContentChanged = GetDocument()->PageContentChanged.Register(this, &LanguageStringView::onPageContentChanged);
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }
   }
   
   /// <summary>Raise the STRING SELECTION CHANGED event</summary>
   /// <param name="pNMHDR">Item data</param>
   /// <param name="pResult">Notify result.</param>
   void LanguageStringView::OnItemStateChanged(NMHDR *pNMHDR, LRESULT *pResult)
   {
      LPNMLISTVIEW pItem = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
      
      try
      {
         // Selection (not focus) has changed
         if ((pItem->uOldState | pItem->uNewState) & LVIS_SELECTED)
            // Update document
            GetDocument()->SelectedString = GetSelected();
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }

      *pResult = 0;
   }

   /// <summary>Populates the strings for the currently selected page</summary>
   void LanguageStringView::onPageSelectionChanged()
   {
      try
      {
         // Clear selection
         GetDocument()->SelectedString = nullptr;

         // Update items
         UpdateItemCount();
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e); 
      }
   }

   /// <summary>Updates the strings</summary>
   void LanguageStringView::onPageContentChanged()
   {
      try
      {
         // Update items
         UpdateItemCount();
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e); 
      }
   }
   
   /// <summary>Performs a menu command</summary>
   /// <param name="nID">Command identifier.</param>
   void LanguageStringView::OnPerformCommand(UINT nID)
   {
      //AfxMessageBox(L"LanguageStringView::OnPerformCommand");

      try 
      {
         switch (nID)
         {
         // TODO:
         case ID_EDIT_COPY:   
            break;
            
         // Cut Selected:
         case ID_EDIT_CUT:    
            GetDocument()->Execute(new CutSelectedString(*this, *GetDocument()));  
            break;

         case ID_EDIT_PASTE:  
            GetDocument()->Execute(new PasteString(*this, *GetDocument()));  
            break;

         // Select All
         //case ID_EDIT_SELECT_ALL:  GetListCtrl().SetItemState(-1, LVIS_SELECTED, LVIS_SELECTED);            break;

         // Delete selected:
         case ID_EDIT_CLEAR:  
            GetDocument()->Execute(new RemoveSelectedString(*this, *GetDocument())); 
            break;
         }
      }
      catch (ExceptionBase& e) {
         theApp.ShowError(HERE, e);
      }
   }
   

   /// <summary>Queries the state of a menu command.</summary>
   /// <param name="pCmdUI">The command UI.</param>
   void LanguageStringView::OnQueryCommand(CCmdUI* pCmdUI)
   {
      bool state = false;

      switch (pCmdUI->m_nID)
      {
      // Require selection
      case ID_EDIT_CLEAR: 
      case ID_EDIT_COPY:   
      case ID_EDIT_CUT:         state = (GetDocument()->SelectedString != nullptr);  break;
     
      // Require string on clipboard
      case ID_EDIT_PASTE:       state = theClipboard.HasLanguageString();              break;

      // Always Enabled/Disabled
      case ID_EDIT_SELECT_ALL:  state = true;  break;
      case ID_EDIT_FIND:        state = false; break;
      }

      // Set state
      pCmdUI->Enable(state ? TRUE : FALSE);
      pCmdUI->SetCheck(FALSE);
   }


   /// <summary>Called when retrieve item].</summary>
   /// <param name="pNMHDR">The NMHDR.</param>
   /// <param name="pResult">The result.</param>
   void LanguageStringView::OnRetrieveItem(NMHDR *pNMHDR, LRESULT *pResult)
   {
      LVITEM& item = reinterpret_cast<NMLVDISPINFO*>(pNMHDR)->item;
      
      try
      {
         // BUGFIX: Because the UpdateItemCount() BugFix requires we scroll to first item before changing count, we sometimes 
         //         receive LVN_GETDISPINFO when the selectedPage is nullptr
         if (!GetDocument()->SelectedPage)
            return;     

         // Lookup string
         auto& str = GetDocument()->SelectedPage->FindByIndex(item.iItem);
      
         // First display: Identify colour tags 
         if (str.TagType == ColourTag::Undetermined)
            str.IdentifyColourTags();

         // Text
         if (item.mask & LVIF_TEXT)
         {
            if (item.iSubItem == 0)
               StringCchCopy(item.pszText, item.cchTextMax, GuiString(L"%d", str.ID).c_str());
            else
               StringCchCopy(item.pszText, item.cchTextMax, str.ResolvedText.c_str());
         }

         // Image
         if (item.mask & LVIF_IMAGE)
            item.iImage = GameVersionIndex(str.Version).Index + 2;
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }

      *pResult = 0;
   }

   /// <summary>Adjusts layout</summary>
   /// <param name="nType">Type of the resize</param>
   /// <param name="cx">The new width</param>
   /// <param name="cy">The new height</param>
   void LanguageStringView::OnSize(UINT nType, int cx, int cy)
   {
      __super::OnSize(nType, cx, cy);
      AdjustLayout();
   }
   
   /// <summary>Updates the item count.</summary>
   void LanguageStringView::UpdateItemCount()
   {
      int count = GetDocument()->SelectedPage ? GetDocument()->SelectedPage->Strings.size() : 0;

      // Update items
      GetListCtrl().EnsureVisible(0, FALSE);    // BUGFIX: Scroll to first item before changing count, otherwise scroll position remains Unchanged
      GetListCtrl().SetItemCountEx(count, 0); 
   }

   // ------------------------------- PRIVATE METHODS ------------------------------
   
   



NAMESPACE_END2(GUI,Views)


