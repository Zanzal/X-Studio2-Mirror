#include "stdafx.h"
#include "DocumentBase.h"
#include "MainWnd.h"

/// <summary>User interface documents</summary>
NAMESPACE_BEGIN2(GUI,Documents)

   // --------------------------------- APP WIZARD ---------------------------------
  
   IMPLEMENT_DYNCREATE(DocumentBase, CDocument)

   BEGIN_MESSAGE_MAP(DocumentBase, CDocument)
   END_MESSAGE_MAP()
   
   // -------------------------------- CONSTRUCTION --------------------------------

   DocumentBase::DocumentBase()
   {
   }

   DocumentBase::~DocumentBase()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   DocumentBase*  DocumentBase::GetActive()
   {
      CMDIChildWnd* pChild = theApp.GetMainWindow()->MDIGetActive();

      if (!pChild)
         return nullptr;

      return (DocumentBase*)pChild->GetActiveDocument();
   }

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Activates this document</summary>
   void  DocumentBase::Activate()
   {
      auto pos = GetFirstViewPosition();
      GetNextView(pos)->GetParentFrame()->ActivateFrame();
   }

   /// <summary>Finds the next.</summary>
   /// <param name="src">The source.</param>
   /// <returns></returns>
   bool  DocumentBase::FindNext(MatchData& src)
   {
      return false;
   }

   /// <summary>Get the full document path.</summary>
   /// <returns></returns>
   IO::Path  DocumentBase::GetFullPath() const
   {
      return IO::Path((const wchar*)GetPathName());
   }

   /// <summary>Get document type.</summary>
   /// <returns></returns>
   DocumentType  DocumentBase::GetType() const
   {
      return Type;
   }

   /// <summary>Sets the selection.</summary>
   /// <param name="rng">char range.</param>
   void  DocumentBase::SetSelection(CHARRANGE rng)
   {
   }
   
   // ------------------------------ PROTECTED METHODS -----------------------------
   
   
   // ------------------------------- PRIVATE METHODS ------------------------------
   
   
/// <summary>User interface documents</summary>
NAMESPACE_END2(GUI,Documents)
