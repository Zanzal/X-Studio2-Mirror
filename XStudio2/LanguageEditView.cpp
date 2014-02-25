// LanguageEditView.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditView.h"
#include "Logic/RtfStringWriter.h"
#include "Helpers.h"
#include "afxcview.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Views)

   // --------------------------------- APP WIZARD ---------------------------------
  
   // -------------------------------- CONSTRUCTION --------------------------------

   LanguageEditView::LanguageEditView() : CFormView(LanguageEditView::IDD)
   {
   }

   LanguageEditView::~LanguageEditView()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   IMPLEMENT_DYNCREATE(LanguageEditView, CFormView)

   BEGIN_MESSAGE_MAP(LanguageEditView, CFormView)
      ON_WM_SIZE()
      ON_WM_CREATE()
      ON_WM_SETTINGCHANGE()
   END_MESSAGE_MAP()
   
   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Gets the language document</summary>
   LanguageDocument* LanguageEditView::GetDocument() const 
   {
	   return dynamic_cast<LanguageDocument*>(m_pDocument);
   }

   /// <summary>Gets the language string view</summary>
   /// <returns></returns>
   /// <exception cref="Logic::GenericException">View not found</exception>
   LanguageStringView*  LanguageEditView::GetStringView() const
   {
      // Iterate thru views
      for (POSITION pos = GetDocument()->GetFirstViewPosition(); pos != NULL; )
      {
         auto pView = dynamic_cast<LanguageStringView*>(GetDocument()->GetNextView(pos));
         if (pView != nullptr)
            return pView;
      }   

      throw GenericException(HERE, L"Cannot find language string View");
   }

   // ------------------------------ PROTECTED METHODS -----------------------------

   /// <summary>Arrange controls</summary>
   void  LanguageEditView::AdjustLayout()
   {
      // Destroyed/Minimised
	   if (RichEdit.GetSafeHwnd() == nullptr || (AfxGetMainWnd() != nullptr && AfxGetMainWnd()->IsIconic()))
         return;
         
      ClientRect view(this);

      // Anchor toolbar to top
      int barHeight = ToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	   ToolBar.SetWindowPos(nullptr, view.left, view.top, view.Width(), barHeight, SWP_NOACTIVATE | SWP_NOZORDER);

      // Stretch RichEdit over remainder
      RichEdit.SetWindowPos(nullptr, view.left, view.top+barHeight, view.Width(), view.Height()-barHeight, SWP_NOZORDER | SWP_NOACTIVATE);
   }

   /// <summary>Does the data exchange.</summary>
   /// <param name="pDX">The p dx.</param>
   void LanguageEditView::DoDataExchange(CDataExchange* pDX)
   {
      CFormView::DoDataExchange(pDX);
      DDX_Control(pDX, IDC_STRING_EDIT, RichEdit);
   }
   
   /// <summary>Creates the toolbar</summary>
   /// <param name="lpCreateStruct">The create structure.</param>
   /// <returns></returns>
   int LanguageEditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
   {
      try
      {
         // Create view
         if (CFormView::OnCreate(lpCreateStruct) == -1)
            throw Win32Exception(HERE, L"Unable to base view");

         // Create toolbar
         if (!ToolBar.Create(this, PrefsLib.LargeToolbars ? IDR_EDITOR_24 : IDR_EDITOR_16, PrefsLib.LargeToolbars ? IDB_EDITOR_24_GREY : IDB_EDITOR_16_GREY))
            throw Win32Exception(HERE, L"Unable to create toolbar");

         return 0;
      }
      catch (ExceptionBase& e) {
         Console.Log(HERE, e);
         return -1;
      }
   }

   /// <summary>Initialise control</summary>
   void LanguageEditView::OnInitialUpdate()
   {
      CFormView::OnInitialUpdate();

      // Listen for string SelectionChanged
      fnStringSelectionChanged = GetStringView()->SelectionChanged.Register(this, &LanguageEditView::onStringSelectionChanged);
      
      // Init RichEdit
      RichEdit.Initialize(MessageBackground);
   }

   /// <summary>Re-creates toolbar</summary>
   /// <param name="uFlags">The flags.</param>
   /// <param name="lpszSection">The section.</param>
   void LanguageEditView::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
   {
      CFormView::OnSettingChange(uFlags, lpszSection);

      // Re-create toolbar
      ToolBar.DestroyWindow();
      ToolBar.Create(this, PrefsLib.LargeToolbars ? IDR_EDITOR_24 : IDR_EDITOR_16, PrefsLib.LargeToolbars ? IDB_EDITOR_24_GREY : IDB_EDITOR_16_GREY);

      // Adjust layout
      AdjustLayout();
   }

   
   /// <summary>Populates the text of the currently selected string</summary>
   void LanguageEditView::onStringSelectionChanged()
   {
      // Clear text
      RichEdit.SetWindowTextW(L"");

      try 
      {
         // Re-populate
         if (LanguageString* str = GetStringView()->GetSelectedString())
         {
            string rtf; 

            // Convert string into RTF
            RtfStringWriter w(rtf);
            w.Write(str->RichText);
            w.Close();

            // Replace contents with RTF
            SETTEXTEX opt = {ST_DEFAULT, CP_ACP};
            if (!RichEdit.SendMessage(EM_SETTEXTEX, (WPARAM)&opt, (LPARAM)rtf.c_str()))
               throw Win32Exception(HERE, L"Unable to set rich-edit text");
         }
      }
      catch (ExceptionBase& e) { 
         Console.Log(HERE, e); 
         RichEdit.SetWindowTextW(GuiString(L"Error: %s", e.Message.c_str()).c_str());
      }
   }

   /// <summary>Adjusts layout</summary>
   /// <param name="nType">Type of the resize</param>
   /// <param name="cx">The new width</param>
   /// <param name="cy">The new height</param>
   void LanguageEditView::OnSize(UINT nType, int cx, int cy)
   {
      CFormView::OnSize(nType, cx, cy);
      AdjustLayout();
   }

   // ------------------------------- PRIVATE METHODS ------------------------------

   
NAMESPACE_END2(GUI,Views)


