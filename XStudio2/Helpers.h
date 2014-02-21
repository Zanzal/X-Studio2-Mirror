#pragma once
#include "stdafx.h"
#include <strsafe.h>

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

      /// <summary>Create a wrapper for an existing item</summary>
      /// <param name="node">node</param>
      /// <param name="txt">text.</param>
      /// <param name="flags">properties.</param>
      explicit TVItem(const TVITEM& item) : TVITEM(item), Text(nullptr)
      {
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
         return *this;
      }

      /// <summary>Sets the text</summary>
      /// <param name="str">The string.</param>
      void  SetText(const wstring& str)
      {
         StringCchCopy(Text.get(), cchTextMax, str.c_str());
      }

      // -------------------- REPRESENTATION ---------------------
   protected:
      CharArrayPtr  Text;
   };

NAMESPACE_END2(GUI,Controls)