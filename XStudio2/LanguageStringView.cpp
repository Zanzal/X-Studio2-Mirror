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
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   LanguageStringView::LanguageStringView() : CustomDraw(this)
   {
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
   
   /// <summary>Called when activate view.</summary>
   /// <param name="bActivate">activated.</param>
   /// <param name="pActivateView">The activate view.</param>
   /// <param name="pDeactiveView">The deactive view.</param>
   void LanguageStringView::OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView)
   {
      // Show properties
      if (bActivate)
         DisplayProperties();

      __super::OnActivateView(bActivate, pActivateView, pDeactiveView);
   }

   /// <summary>Populates the properties window</summary>
   /// <param name="grid">The grid.</param>
   /// <exception cref="Logic::ArgumentException">String colour tags still 'undetermined'</exception>
   /// <exception cref="Logic::ArgumentNullException">No string or page is selected</exception>
   void  LanguageStringView::OnDisplayProperties(CMFCPropertyGridCtrl& grid)
   {
      REQUIRED(GetDocument()->SelectedString);
      REQUIRED(GetDocument()->SelectedPage);
      
      // Init
      LanguageString& str = *GetDocument()->SelectedString;
      LanguageDocument& doc = *GetDocument();

      // Group: String
      CMFCPropertyGridProperty* group = new CMFCPropertyGridProperty(_T("String"));

      // ID/Description/Title/Voiced
      group->AddSubItem(new IDProperty(doc, *GetDocument()->SelectedPage, str));
      group->AddSubItem(new ColourTagProperty(doc, str));
      group->AddSubItem(new VersionProperty(doc, str));
      grid.AddProperty(group);
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
   
   /// <summary>Displays string or document properties.</summary>
   void LanguageStringView::DisplayProperties()
   {
      // Show string properties (if any), otherwise document properties
      if (GetDocument()->SelectedString)
         CPropertiesWnd::Connect(this, true);
      else 
         CPropertiesWnd::Connect(GetDocument(), true);
   }

   /// <summary>Retrieves the language string representing the current selection.</summary>
   /// <returns>Selected string if any, otherwise nullptr</returns>
   /// <exception cref="Logic::IndexOutOfRangeException">Selected item index is invalid</exception>
   LanguageString*   LanguageStringView::GetSelected() const
   {
      int item = GetListCtrl().GetNextItem(-1, LVNI_SELECTED);
      return item != -1 ? &GetDocument()->SelectedPage->Strings.FindByIndex(item) : nullptr;
   }
   
   /// <summary>Custom draw the strings</summary>
   /// <param name="pNMHDR">header.</param>
   /// <param name="pResult">result.</param>
   void LanguageStringView::OnCustomDraw(NMHDR *pNMHDR, LRESULT *pResult)
   {
      NMCUSTOMDRAW* pDraw = reinterpret_cast<NMCUSTOMDRAW*>(pNMHDR);
      *pResult = CustomDraw.OnCustomDraw(pDraw);
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
         GetListCtrl().ModifyStyle(WS_BORDER, LVS_SHOWSELALWAYS|LVS_SINGLESEL);
         GetListCtrl().SetView(LV_VIEW_DETAILS);
         GetListCtrl().InsertColumn(0, L"ID", LVCFMT_LEFT, 60, 0);
         GetListCtrl().InsertColumn(1, L"Text", LVCFMT_LEFT, 240, 1);
         GetListCtrl().SetExtendedStyle(LVS_EX_FULLROWSELECT);
         GetListCtrl().SetImageList(&Images, LVSIL_SMALL);

         // Listen for PageClicked
         auto pageView = GetDocument()->GetView<LanguagePageView>();
         fnPageSelectionChanged = pageView->SelectionChanged.Register(this, &LanguageStringView::onPageSelectionChanged);
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
         {
            // Update document
            GetDocument()->SelectedString = GetSelected();

            // Refresh properties
            DisplayProperties();

            // Raise SELECTION CHANGED
            SelectionChanged.Raise();
         }
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
         CWaitCursor c;

         // Clear items + selection
         GetListCtrl().DeleteAllItems();
         GetDocument()->SelectedString = nullptr;

         // Raise SELECTION CHANGED
         SelectionChanged.Raise();

         // Get selection, if any
         if (LanguagePage* page = GetDocument()->SelectedPage)
         {
            int item = -1;
            //Console << L"User has clicked on page: " << (page?page->ID:-1) << L" : " << (page?page->Title:L"") << ENDL;

            // Re-Populate strings
            GetListCtrl().SetRedraw(FALSE);
            for (auto& pair : page->Strings)
            {
               LanguageString& str = pair.second;

               // First display: Identify colour tags 
               if (str.TagType == ColourTag::Undetermined)
                  str.IdentifyColourTags();

               // Add item, resolve and set Text
               GetListCtrl().InsertItem(++item, GuiString(L"%d", str.ID).c_str(), 2+GameVersionIndex(str.Version).Index);
               GetListCtrl().SetItemText(item, 1, str.ResolvedText.c_str());
            }
            GetListCtrl().SetRedraw(TRUE);
            GetListCtrl().UpdateWindow();
         }
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e); 
      }
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
   
   // ------------------------------- PRIVATE METHODS ------------------------------
   
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

            RichTextRenderer::DrawLine(dc, item.Rect, RichStringParser((const wchar*)src).Output, flags);
         }
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
      }
   }





NAMESPACE_END2(GUI,Views)


