#pragma once
#include "stdafx.h"
#include "../Logic/RichString.h"
#include <strsafe.h>
#include <Richole.h>

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Controls)
   
   /// <summary>Listview group helper</summary>
   class LVGroup : public LVGROUP
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
      
   public:
      LVGroup(UINT id, const wstring& txt, UINT flags = LVGF_HEADER | LVGF_GROUPID) : Header(txt)
      {
         ZeroMemory((LVGROUP*)this, sizeof(LVGROUP));

         cbSize    = sizeof(LVGROUP);
         mask      = flags;
         iGroupId  = id;
         pszHeader = (WCHAR*)Header.c_str();
      }
      
      // ------------------------ STATIC -------------------------
      
      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
      
      // ----------------------- MUTATORS ------------------------
      
      // -------------------- REPRESENTATION ---------------------
      
   private:
      wstring Header;
   };
   
   /// <summary>Listview item helper</summary>
   class LVItem : public LVITEM
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
      
   public:
      /// <summary>Create empty item for retrieving text and properties</summary>
      /// <param name="item">Item</param>
      /// <param name="subItem">SubItem</param>
      /// <param name="bufLen">buffer length</param>
      /// <param name="flags">desired properties</param>
      LVItem(UINT item, UINT subItem, UINT bufLen, UINT flags = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM | LVIF_GROUPID)
      {
         ZeroMemory((LVITEM*)this, sizeof(LVITEM));

         // Allocate text buffer
         if (bufLen > 0)
         {
            Text = CharArrayPtr(new wchar[bufLen]);
            Text.get()[0] = '\0';
         }

         mask       = flags;
         pszText    = Text.get();
         cchTextMax = bufLen;
         iItem      = item;
         iSubItem   = subItem;
      }

      /// <summary>Create a populated item for setting text and properties</summary>
      /// <param name="item">item number</param>
      /// <param name="txt">text.</param>
      /// <param name="group">group.</param>
      /// <param name="flags">properties.</param>
      LVItem(UINT item, const wstring& txt, UINT group, UINT flags = LVIF_TEXT | LVIF_GROUPID) : Text(CopyText(txt))
      {
         ZeroMemory((LVITEM*)this, sizeof(LVITEM));

         mask     = flags;
         iItem    = item;
         iGroupId = group;
         pszText  = Text.get();
      }
      
      // ------------------------ STATIC -------------------------
   private:
      /// <summary>Duplicates the input string.</summary>
      /// <param name="str">The string.</param>
      /// <returns>new wide char array</returns>
      static wchar* CopyText(const wstring& str)
      {
         wchar* buf = new wchar[str.length()+1];
         StringCchCopy(buf, str.length()+1, str.c_str());
         return buf;
      }

      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
      
      // ----------------------- MUTATORS ------------------------
      
      // -------------------- REPRESENTATION ---------------------
   protected:
      CharArrayPtr  Text;
   };


   /// <summary>TreeView item helper</summary>
   class TVItem : public TVITEM
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
      
   public:
      /// <summary>Create empty item for retrieving text and properties</summary>
      /// <param name="node">node</param>
      /// <param name="bufLen">buffer length</param>
      /// <param name="flags">desired properties</param>
      TVItem(HTREEITEM node, UINT bufLen, UINT flags = TVIF_TEXT | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM)
      {
         ZeroMemory(dynamic_cast<TVITEM*>(this), sizeof(TVITEM));

         // Allocate text buffer
         Text = CharArrayPtr(new wchar[bufLen+1]);
         Text.get()[0] = L'\0';

         // Set properties
         mask       = flags;
         hItem      = node;
         pszText    = Text.get();
         cchTextMax = bufLen;
      }

      /// <summary>Create a wrapper for an existing item, duplicating the text into internal buffer</summary>
      /// <param name="item">item</param>
      explicit TVItem(const TVITEM& item) : TVITEM(item), Text(new wchar[256])
      {
         cchTextMax = 256;
         Assign(item);
      }
      
      // ------------------------ STATIC -------------------------
   private:

      // --------------------- PROPERTIES ------------------------
	   
      // ---------------------- ACCESSORS ------------------------			
   public:
      
      // ----------------------- MUTATORS ------------------------
   public:
      /// <summary>Copy data from another item</summary>
      /// <param name="r">The r.</param>
      /// <returns></returns>
      TVItem& operator=(const TVITEM& r)
      {
         Assign(r);
         return *this;
      }

      /// <summary>Sets the text</summary>
      /// <param name="str">The string.</param>
      void  SetText(const wstring& str)
      {
         StringCchCopy(pszText=Text.get(), cchTextMax, str.c_str());
      }

   private:
      /// <summary>Copy data from another item</summary>
      /// <param name="r">The r.</param>
      /// <returns></returns>
      void  Assign(const TVITEM& r)
      {
         mask = r.mask;

         // Copy handle/data
         hItem  = (r.mask & TVIF_HANDLE ? r.hItem  : 0);
         lParam = (r.mask & TVIF_PARAM  ? r.lParam : 0);

         // Copy/Erase text
         SetText((r.mask & TVIF_TEXT) && r.pszText ? r.pszText : L"");

         // Copy remaining properties
         iSelectedImage = (r.mask & TVIF_SELECTEDIMAGE ? r.iSelectedImage : 0);
         iImage    = (r.mask & TVIF_IMAGE ? r.iImage    : 0);
         state     = (r.mask & TVIF_STATE ? r.state     : 0);
         stateMask = (r.mask & TVIF_STATE ? r.stateMask : 0);
      }

      // -------------------- REPRESENTATION ---------------------
   protected:
      CharArrayPtr  Text;
   };


   
   /// <summary>Character format helper</summary>
   struct CHARFORMAT3 : public CHARFORMAT2
   {
   public:
      BYTE bUnderlineColor;   // Undocumented index into RTF colour table
   };

   /// <summary>Character format helper</summary>
   class CharFormat : public CHARFORMAT3
   {
   public:
      /// <summary>Create empty.</summary>
      CharFormat() { Clear(); }

      /// <summary>Set initial mask and effects</summary>
      /// <param name="mask">The mask.</param>
      /// <param name="effects">The effects.</param>
      CharFormat(DWORD mask, DWORD effects) : CharFormat()
      {
         dwMask = mask;
         dwEffects = effects;
      }

      /// <summary>Set initial mask, effects and text colour.</summary>
      /// <param name="mask">The mask.</param>
      /// <param name="effects">The effects.</param>
      /// <param name="col">text colour.</param>
      CharFormat(DWORD mask, DWORD effects, COLORREF col) : CharFormat(mask, effects)
      {
         crTextColor = col;
      }

      /// <summary>Create from logFont</summary>
      /// <param name="lf">The lf.</param>
      /// <param name="wnd">window to use for font height conversion.</param>
      CharFormat(LOGFONT lf, CWnd* wnd) : CharFormat(CFM_FACE | CFM_CHARSET | CFM_WEIGHT | CFM_WEIGHT | CFM_ITALIC | CFM_UNDERLINE | CFM_SIZE, 0)
      {
         // Font
         StringCchCopy(szFaceName, LF_FACESIZE, lf.lfFaceName);
         bPitchAndFamily = lf.lfPitchAndFamily;
         bCharSet = lf.lfCharSet;

         // Height (Points -> twips)
         if (lf.lfHeight >= 0)
            yHeight = lf.lfHeight * 20;   
         else
         {  // Height (Pixels -> twips)
            auto dc = wnd->GetDC();
            yHeight = -lf.lfHeight * 1440 / dc->GetDeviceCaps(LOGPIXELSY);  // == 20 * MulDiv(-lf.lfHeight, 72, dc->GetDeviceCaps(LOGPIXELSY));
            wnd->ReleaseDC(dc);
         }
         
         // Formatting
         wWeight = (WORD)lf.lfWeight;
         if (lf.lfItalic)
            dwEffects |= CFE_ITALIC;
         if (lf.lfUnderline)
            dwEffects |= CFE_UNDERLINE;
      }

      /// <summary>Clear formatting</summary>
      void  Clear()
      {
         ZeroMemory((CHARFORMAT3*)this, sizeof(CHARFORMAT2));
         cbSize = sizeof(CHARFORMAT2);
      }
   };

   /// <summary>Character format helper</summary>
   class ParaFormat : public PARAFORMAT2
   {
   public:
      /// <summary>Create empty.</summary>
      ParaFormat() { Clear(); }

      /// <summary>Set initial mask.</summary>
      /// <param name="mask">The mask.</param>
      ParaFormat(DWORD mask) : ParaFormat()
      {
         dwMask = mask;
      }
      /// <summary>Set mask and alignment</summary>
      /// <param name="mask">The mask.</param>
      /// <param name="align">The align.</param>
      ParaFormat(DWORD mask, Alignment align) : ParaFormat(mask)
      {
         wAlignment = (UINT)align;
      }

      /// <summary>Clear formatting</summary>
      void  Clear()
      {
         ZeroMemory((PARAFORMAT2*)this, sizeof(PARAFORMAT2));
         cbSize = sizeof(PARAFORMAT2);
      }
   };

   /// <summary>RichEdit object helper</summary>
   class ReObject : public REOBJECT
   {
   public:
      ReObject() { Clear(); }

      /// <summary>Clear</summary>
      void  Clear()
      {
         ZeroMemory((REOBJECT*)this, sizeof(REOBJECT));
         cbStruct = sizeof(REOBJECT);
      }
   };


   /// <summary>Character range helper</summary>
   class CharRange : public CHARRANGE
   {
   public:
      CharRange(long from, long to) : CHARRANGE({from,to})
      {}
   };


NAMESPACE_END2(GUI,Controls)