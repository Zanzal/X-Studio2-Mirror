#include "stdafx.h"
#include "OwnerDrawImpl.h"

namespace GUI
{
   namespace Utils
   {
   
      // -------------------------------- CONSTRUCTION --------------------------------

      OwnerDrawImpl::OwnerDrawImpl(DRAWITEMSTRUCT& d, CFont* f)
         : DC(d.hDC), 
           Disabled((d.itemState & ODS_DISABLED) != 0), 
           Focused((d.itemState & ODS_FOCUS) != 0), 
           Selected((d.itemState & ODS_SELECTED) != 0), 
           PrevFont(nullptr)
      {
         SetFont(f);
      }

      OwnerDrawImpl::~OwnerDrawImpl()
      {
         // Restore original font
         DC.SelectObject(PrevFont);
      }

      // ------------------------------- STATIC METHODS -------------------------------

      // ------------------------------- PUBLIC METHODS -------------------------------

      // ------------------------------ PROTECTED METHODS -----------------------------
      
      /// <summary>Draws the background, using the system highlight colour if appropriate</summary>
      /// <param name="dc">dc.</param>
      /// <param name="rc">drawing rectangle.</param>
      /// <param name="col">default (not-selected) colour.</param>
      void  OwnerDrawImpl::DrawBackground(CRect& rc, UINT col /*= COLOR_WINDOW*/)
      {
         // Fill with correct system colour
         DC.FillSolidRect(rc, GetSysColor(!Selected ? col : COLOR_HIGHLIGHT));
      }

      /// <summary>Draws the focus rectangle</summary>
      /// <param name="dc">device context.</param>
      /// <param name="rc">drawing rectangle.</param>
      void  OwnerDrawImpl::DrawFocus(CRect& rc)
      {
         if (Focused)
            DC.DrawFocusRect(rc);
      }

      /// <summary>Draws text using the system highlight colour if appropriate</summary>
      /// <param name="dc">device context.</param>
      /// <param name="sz">string.</param>
      /// <param name="rc">drawing rectangle.</param>
      /// <param name="flags">drawing flags.</param>
      /// <param name="col">default (not-selected) colour.</param>
      void  OwnerDrawImpl::DrawText(const wstring& sz, CRect& rc, UINT flags /*= DT_LEFT*/, UINT col /*= COLOR_WINDOWTEXT*/)
      {
         // Set appropriate system colour
         auto col = DC.SetTextColor(GetSysColor(!Selected ? col : COLOR_HIGHLIGHTTEXT));
         auto bk = DC.SetBkMode(TRANSPARENT);
         
         // Draw Text 
         DC.DrawText(sz.c_str(), sz.length(), rc, flags);

         // Cleanup
         DC.SetTextColor(col);
         DC.SetBkMode(bk);
      }

      void  OwnerDrawImpl::SetFont(CFont* f)
      {
         REQUIRED(f);
         
         // Change font
         CFont* prev = DC.SelectObject(f);

         // Only preserve the initial font
         if (!PrevFont)
            PrevFont = prev;
      }

      // ------------------------------- PRIVATE METHODS ------------------------------
   
   }
}
