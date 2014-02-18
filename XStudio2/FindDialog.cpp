// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FindDialog.h"
#include "MainWnd.h"
#include "afxdialogex.h"
#include "FindProgressDialog.h"

/// <summary>User interface windows</summary>
NAMESPACE_BEGIN2(GUI,Windows)

      // -------------------------------- CONSTRUCTION --------------------------------

      FindDialog::FindDialog(CWnd* pParent /*=NULL*/)
	      : CDialogEx(FindDialog::IDD, pParent)
         , UseRegEx(FALSE)
         , MatchCase(FALSE)
         , MatchWholeWord(FALSE)
         , Expanded(true)
      {
      }

      FindDialog::~FindDialog()
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------
      
      IMPLEMENT_DYNAMIC(FindDialog, CDialogEx)
      
      BEGIN_MESSAGE_MAP(FindDialog, CDialogEx)
         ON_BN_CLICKED(IDC_FIND, &FindDialog::OnFind_Click)
         ON_BN_CLICKED(IDC_REPLACE, &FindDialog::OnReplace_Click)
         ON_BN_CLICKED(IDC_FIND_ALL, &FindDialog::OnFindAll_Click)
         ON_BN_CLICKED(IDC_REPLACE_ALL, &FindDialog::OnReplaceAll_Click)
         ON_BN_CLICKED(IDC_OPTIONS, &FindDialog::OnOptions_Click)
         ON_CBN_EDITCHANGE(IDC_FIND_COMBO, &FindDialog::OnFind_TextChanged)
         ON_CBN_EDITCHANGE(IDC_REPLACE_COMBO, &FindDialog::OnReplace_TextChanged)
         ON_CBN_EDITCHANGE(IDC_TARGET_COMBO, &FindDialog::OnTarget_TextChanged)
         ON_WM_SHOWWINDOW()
      END_MESSAGE_MAP()

      // ------------------------------- PUBLIC METHODS -------------------------------

      // ------------------------------ PROTECTED METHODS -----------------------------
      
      void FindDialog::DoDataExchange(CDataExchange* pDX)
      {
         CDialogEx::DoDataExchange(pDX);
         DDX_Check(pDX, IDC_REGEX_CHECK, UseRegEx);
         DDX_Check(pDX, IDC_CASE_CHECK, MatchCase);
         DDX_Check(pDX, IDC_WORD_CHECK, MatchWholeWord);
         DDX_Control(pDX, IDC_FIND, FindButton);
         DDX_Control(pDX, IDC_FIND_ALL, FindAllButton);
         DDX_Control(pDX, IDC_FIND_COMBO, FindCombo);
         DDX_Control(pDX, IDC_OPTIONS, OptionsButton);
         DDX_Control(pDX, IDC_REPLACE, ReplaceButton);
         DDX_Control(pDX, IDC_REPLACE_ALL, ReplaceAllButton);
         DDX_Control(pDX, IDC_REPLACE_COMBO, ReplaceCombo);
         DDX_Control(pDX, IDC_TARGET_COMBO, TargetCombo);
      }

      /// <summary>Shrinks or Expands the dialog.</summary>
      /// <param name="expand">True to expand, false to shrink</param>
      void FindDialog::Expand(bool expand)
      {
         vector<int> Buttons  = { IDC_FIND, IDC_REPLACE, IDC_OPTIONS, IDC_FIND_ALL, IDC_REPLACE_ALL };
         vector<int> Controls = { IDC_REGEX_CHECK, IDC_CASE_CHECK, IDC_WORD_CHECK, IDC_RESULTS1_RADIO, IDC_RESULTS2_RADIO, IDC_FIND_GROUPBOX };

         // Show/Hide controls
         for (int id : Controls)
            GetDlgItem(id)->ShowWindow(expand ? SW_SHOW : SW_HIDE);

         // Calculate amount to resize
         CRect resizeRect;
         GetDlgItem(IDC_FIND_RESIZE)->GetWindowRect(resizeRect);
         int resize = (expand ? 1 : -1) * resizeRect.Height();

         // Resize window
         CRect wnd;
         GetWindowRect(wnd);
         SetWindowPos(nullptr, 0, 0, wnd.Width(), wnd.Height()+resize, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
         
         // Move buttons
         for (int id : Buttons)
         {
            CRect rc;
            GetDlgItem(id)->GetWindowRect(rc);
            ScreenToClient(rc);
            rc.OffsetRect(0, resize);
            GetDlgItem(id)->SetWindowPos(nullptr, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
         }

         // Set options text
         OptionsButton.SetWindowTextW(expand ? L"Hide Options" : L"Show Options");
         
         // Update state
         Expanded = expand;
      }

      /// <summary>Gets the search target.</summary>
      /// <returns></returns>
      SearchTarget  FindDialog::GetSearchTarget() const
      {
         return (SearchTarget)TargetCombo.GetCurSel();
      }

      /// <summary>Gets the search term.</summary>
      /// <returns></returns>
      wstring  FindDialog::GetSearchTerm() const
      {
         CString str;
         FindCombo.GetWindowText(str);
         return (const wchar*)str;
      }
      
      /// <summary>Gets the replacement term.</summary>
      /// <returns></returns>
      wstring  FindDialog::GetReplaceTerm() const
      {
         CString str;
         ReplaceCombo.GetWindowText(str);
         return (const wchar*)str;
      }
      
      /// <summary>One-time initialization</summary>
      /// <returns></returns>
      BOOL FindDialog::OnInitDialog()
      {
         CDialogEx::OnInitDialog();

         // Populate targets
         TargetCombo.AddString(L"Selection");
         TargetCombo.AddString(L"Current Document");
         TargetCombo.AddString(L"All Open Documents");
         TargetCombo.AddString(L"All Project Files");
         TargetCombo.AddString(L"Scripts Folder");
         TargetCombo.SetCurSel(0);
         
         // Set output pane
         this->CheckRadioButton(IDC_RESULTS1_RADIO, IDC_RESULTS2_RADIO, IDC_RESULTS1_RADIO);
         return TRUE;  // return TRUE unless you set the focus to a control
      }

      /// <summary>Finds the next match, if any</summary>
      void FindDialog::OnFind_Click()
      {
         try
         {
            // Feedback
            Console << Cons::UserAction << (Search ? "Find Next: " : "Find First: ") 
                    << Cons::Yellow << GetSearchTerm() << Cons::White << " in " 
                    << Cons::Yellow << GetString(GetSearchTarget()) << ENDL;

            // Init: Create operation
            if (!Search)
               NewSearch(Operation::Find);

            // FindNext:
            if (!Search->FindNext())
            {
               // Complete
               Reset();
               AfxMessageBox(L"Search complete");
            }
         }
         catch (ExceptionBase& e) {
            theApp.ShowError(HERE, e);
         }
      }

      
      void FindDialog::OnFind_TextChanged()
      {
         Reset();
      }
      
      void FindDialog::OnFindAll_Click()
      {
         try
         {
            // Feedback
            Console << Cons::UserAction << "Find All: "
                    << Cons::Yellow << GetSearchTerm() << Cons::White << " in " 
                    << Cons::Yellow << GetString(GetSearchTarget()) << ENDL;

            // Find All
            NewSearch(Operation::FindAll);
            Search->FindAll();
            Reset();
         }
         catch (ExceptionBase& e) {
            theApp.ShowError(HERE, e);
         }
      }
      
      void FindDialog::OnOptions_Click()
      {
         // Toggle window size
         Expand(!Expanded);
      }

      void FindDialog::OnReplace_Click()
      {
         try
         {
            // Init: Create operation
            if (!Search)
               NewSearch(Operation::Replace);

            // Replace:
            if (!Search->Replace(GetReplaceTerm()))
            {
               // Complete
               Reset();
               AfxMessageBox(L"Search complete");
            }
         }
         catch (ExceptionBase& e) {
            theApp.ShowError(HERE, e);
         }
      }

      void FindDialog::OnReplace_TextChanged()
      {
         Reset();
      }
      

      void FindDialog::OnReplaceAll_Click()
      {
         try
         {
            NewSearch(Operation::ReplaceAll);
            Search->ReplaceAll(GetReplaceTerm());
            Reset();
         }
         catch (ExceptionBase& e) {
            theApp.ShowError(HERE, e);
         }
      }
      
      void FindDialog::OnShowWindow(BOOL bShow, UINT nStatus)
      {
         CDialogEx::OnShowWindow(bShow, nStatus);

         // Start anew
         Reset();
      }
      
      void FindDialog::OnTarget_TextChanged()
      {
         Reset();
      }
      

      // ------------------------------- PRIVATE METHODS ------------------------------

      void  FindDialog::NewSearch(Operation operation)
      {
         // Display 'Find Next'
         FindButton.SetWindowTextW(L"Find Next");

         // Create new search
         Search.reset(new SearchOperation(operation,
                                          GetSearchTarget(), 
                                          GetSearchTerm(), 
                                          GetReplaceTerm(), 
                                          MatchCase != FALSE, 
                                          MatchWholeWord != FALSE, 
                                          UseRegEx != FALSE));
      }

      void  FindDialog::Reset()
      {
         // Display 'Find'
         FindButton.SetWindowTextW(L"Find");
         Search.reset(nullptr);
      }

/// <summary>User interface windows</summary>
NAMESPACE_END2(GUI,Windows)


