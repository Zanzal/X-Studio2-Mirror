#include "stdafx.h"
#include "ScriptObjectWnd.h"
#include "MainWnd.h"
#include "../Logic/ScriptObjectLibrary.h"
#include <strsafe.h>
#include "Helpers.h"

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Windows)

   // --------------------------------- APP WIZARD ---------------------------------
  
   BEGIN_MESSAGE_MAP(CScriptObjectWnd, CGameDataWnd)
   END_MESSAGE_MAP()

   // -------------------------------- CONSTRUCTION --------------------------------

   CScriptObjectWnd::CScriptObjectWnd() 
   {
   }


   CScriptObjectWnd::~CScriptObjectWnd()
   {
   }

   // ------------------------------- STATIC METHODS -------------------------------

   // ------------------------------- PUBLIC METHODS -------------------------------

   /// <summary>Creates the window.</summary>
   /// <param name="parent">The parent.</param>
   /// <exception cref="Logic::Win32Exception">Unable to create window</exception>
   void CScriptObjectWnd::Create(CWnd* parent)
   {
      __super::Create(parent, L"Script Objects", IDR_SCRIPT_OBJECTS, IDR_SCRIPT_OBJECTS);
   }

   // ------------------------------ PROTECTED METHODS -----------------------------
   
   /// <summary>Gets the item text.</summary>
   /// <param name="index">The index.</param>
   /// <returns></returns>
   wstring  CScriptObjectWnd::GetItemText(UINT index)
   {
      auto obj = reinterpret_cast<const ScriptObject*>(ListView.GetItemData(index));

      // Format item
      return obj ? obj->GetDisplayText() : L"";
   }
   
   /// <summary>Supply tooltip data.</summary>
   /// <param name="data">data.</param>
   void CScriptObjectWnd::OnRequestTooltip(CustomTooltip::TooltipData* data)
   {
      int index = GetHotItemIndex();

      // Verify index
      if (index != -1)
      {
         // Lookup current items
         auto Content = ScriptObjectLib.Query(GetSearchTerm());

         // Provide item
         if (index < (int)Content.size())
         {
            data->ResetTo(ScriptObjectTooltipData(*Content[index]));
            return;
         }
      }

      // No item: Cancel
      data->Cancel();
   }

   /// <summary>Populates the group combo.</summary>
   void  CScriptObjectWnd::PopulateGroupCombo()
   {
      Groups.AddString(L"All Groups");

      // Populate group names
      for (ScriptObjectGroup g = ScriptObjectGroup::Constant; g <= ScriptObjectGroup::TransportClass; g++)
         Groups.AddString(GetString(g).c_str());

      // Select 'unfiltered'
      Groups.SetCurSel(0);
   }

   /// <summary>Populates the items</summary>
   /// <param name="searchTerm">The search term.</param>
   /// <param name="selectedGroup">The selected group.</param>
   void CScriptObjectWnd::PopulateItems(const wstring& searchTerm, UINT selectedGroup)
   {
      // Lookup matches
      auto Content = ScriptObjectLib.Query(searchTerm);
      ListView.SetItemCount(Content.size());
      
      // Redefine groups
      for (ScriptObjectGroup g = ScriptObjectGroup::Constant; g <= ScriptObjectGroup::TransportClass; g++)
      {
         LVGroup grp((UINT)g, GetString(g));
         
         // Insert group
         if (ListView.InsertGroup(grp.iGroupId, (LVGROUP*)&grp) != grp.iGroupId)
            throw Win32Exception(HERE, GuiString(L"Unable to insert script object group ") + GetString(g));
      }

      // Generate/insert display text for each command
      UINT i = 0;
      for (auto& obj : Content)
      {
         // Define item
         LVItem item(i++, obj->Text, (UINT)obj->Group, LVIF_TEXT|LVIF_GROUPID|LVIF_IMAGE|LVIF_PARAM);
         item.iImage = 3 + GameVersionIndex(obj->Version).Index;
         item.lParam = (LPARAM)obj;

         // Insert item
         if (ListView.InsertItem((LVITEM*)&item) == -1)
            throw Win32Exception(HERE, VString(L"Unable to insert %s '%s' (item %d)", GetString(obj->Group).c_str(), item.pszText, i-1));
      }
   }
   // ------------------------------- PRIVATE METHODS ------------------------------
   
   
NAMESPACE_END2(GUI,Windows)
