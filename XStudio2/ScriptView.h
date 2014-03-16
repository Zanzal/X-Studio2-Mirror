#pragma once

#include "afxcmn.h"
#include "..Utils/Utils.h"
#include "ScriptDocument.h"
#include "ScriptEdit.h"


/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Views)

   // ------------------ EVENTS AND DELEGATES -----------------

   typedef Event<POINT>                  CaretMovedEvent;
   typedef CaretMovedEvent::DelegatePtr  CaretMovedHandler;

   // ------------------------ CLASSES ------------------------

   /// <summary>Script view</summary>
   class ScriptView : public CFormView
   {
      // ------------------------ TYPES --------------------------
   public:
	   enum{ IDD = IDR_SCRIPTVIEW };

      // --------------------- CONSTRUCTION ----------------------
   protected: 
	   ScriptView();
	public:
      virtual ~ScriptView();   
	  
      // ------------------------ STATIC -------------------------
      DECLARE_DYNCREATE(ScriptView)
      DECLARE_MESSAGE_MAP()

   public:
      static CaretMovedEvent   CaretMoved;
   
      // --------------------- PROPERTIES ------------------------
			
      // ---------------------- ACCESSORS ------------------------			
   public:
      bool            FindNext(UINT start, MatchData& m) const;
	   ScriptDocument* GetDocument() const;
      ScriptFile&     GetScript() const;
      CHARRANGE       GetSelection() const;

      // ----------------------- MUTATORS ------------------------
   public:
      bool  Replace(MatchData& m);
      void  Replace(const wstring& txt);
      void  SetSelection(CHARRANGE rng);
      BOOL  PreTranslateMessage(MSG* pMsg) override;

   protected:
      void AdjustLayout();
      void ScrollTo(const ScriptLabel& l);
      void PopulateVariables();
      void PopulateScope();
      virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
      virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
      void UpdateScope();
      
      virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
      afx_msg void OnDestroy();
      afx_msg void OnEditComment()       { OnPerformCommand(ID_EDIT_COMMENT);     }
      afx_msg void OnEditIndent()        { OnPerformCommand(ID_EDIT_INDENT);      }
      afx_msg void OnEditOutdent()       { OnPerformCommand(ID_EDIT_OUTDENT);     }
      afx_msg void OnEditFormatDoc()     { OnPerformCommand(ID_EDIT_FORMAT_DOC);  }
      afx_msg void OnEditFormatSel()     { OnPerformCommand(ID_EDIT_FORMAT_SEL);  }
      afx_msg void OnEditRefactor()      { OnPerformCommand(ID_EDIT_REFACTOR);    }
      afx_msg void OnEditGotoLabel();  
      afx_msg void OnEditOpenScript(); 
      afx_msg void OnEditViewString(); 
      afx_msg void OnEditUndo()          { OnPerformCommand(ID_EDIT_UNDO);        }
      afx_msg void OnEditRedo()          { OnPerformCommand(ID_EDIT_REDO);        }
      afx_msg void OnClipboardCopy()     { OnPerformCommand(ID_EDIT_COPY);        }
      afx_msg void OnClipboardCut()      { OnPerformCommand(ID_EDIT_CUT);         }
      afx_msg void OnClipboardPaste()    { OnPerformCommand(ID_EDIT_PASTE);       }
      afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
      handler void OnCompileComplete();
	   virtual void OnInitialUpdate(); 
      afx_msg void OnPerformCommand(UINT nID);
      afx_msg void OnQueryCommand(CCmdUI *pCmdUI);
      afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
      afx_msg void OnScopeSelectionChange();
      afx_msg void OnSetFocus(CWnd* pOldWnd);
      afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
      afx_msg void OnSize(UINT nType, int cx, int cy);	  
      afx_msg void OnTextSelectionChange(NMHDR* pNMHDR, LRESULT* result);
      handler void OnTextChanged();

      // -------------------- REPRESENTATION ---------------------
   public:
      ScriptEdit RichEdit;
      CComboBox  ScopeCombo;
      CComboBox  VariablesCombo;

   protected:
      EventHandler fnCompileComplete,
                   fnTextChanged;

   };


   
NAMESPACE_END2(GUI,Views)

