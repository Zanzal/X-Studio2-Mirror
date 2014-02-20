#pragma once


/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Utils)

   // ------------------------------------ ENUMS -----------------------------------

   // ----------------------------- EVENTS AND DELEGATES ---------------------------

   // ----------------------------------- CLASSES ----------------------------------

   /// <summary>Defines a source of Properties</summary>
   class PropertySource
   {
   public:
      /// <summary>Called to populate properties.</summary>
      /// <param name="p">The p.</param>
      virtual void  OnDisplayProperties(CMFCPropertyGridCtrl& grid) PURE;

      /// <summary>Called when property updated.</summary>
      /// <param name="p">The p.</param>
      virtual bool  OnPropertyUpdated(CMFCPropertyGridProperty* p)
      {
         return true;
      }

      /// <summary>Called when property needs validating.</summary>
      /// <param name="p">The p.</param>
      /// <returns></returns>
      virtual bool  OnValidateProperty(CMFCPropertyGridProperty* p)
      {
         return true;
      }
   };

   
   /// <summary>Property grid item with validation</summary>
   class ValidatingProperty : public CMFCPropertyGridProperty
   {
      // --------------------- CONSTRUCTION ----------------------
   public:
	   ValidatingProperty(const CString& strGroupName, DWORD_PTR dwData = 0, BOOL bIsValueList = FALSE)
         : CMFCPropertyGridProperty(strGroupName, dwData, bIsValueList)
      {}

	   ValidatingProperty(const CString& strName, const COleVariant& varValue, LPCTSTR lpszDescr = NULL, DWORD_PTR dwData = 0,
		   LPCTSTR lpszEditMask = NULL, LPCTSTR lpszEditTemplate = NULL, LPCTSTR lpszValidChars = NULL)
         : CMFCPropertyGridProperty(strName, varValue, lpszDescr, dwData, lpszEditMask, lpszEditTemplate, lpszValidChars)
      {}

      // ---------------------- ACCESSORS ------------------------	
   protected:
      /// <summary>Query whether an option exists (case insensitive)</summary>
      /// <param name="value">item text.</param>
      /// <returns></returns>
      bool Contains(const wstring& value) const
      {
         auto val = value.c_str();
         
         // Search options
         for (auto pos = m_lstOptions.GetHeadPosition(); pos; )
            if (!StrCmpI(m_lstOptions.GetNext(pos), val))
               return true;

         return false;
      }

      /// <summary>Finds the index of an option from it's text  (case insensitive)</summary>
      /// <param name="value">item text</param>
      /// <returns>Index if found, otherwise -1</returns>
      int Find(const wstring& value) const
      {
         auto val = value.c_str();

         // Search options
         int i = 0;
         for (auto pos = m_lstOptions.GetHeadPosition(); pos; ++i)
            if (!StrCmpI(m_lstOptions.GetNext(pos), val))
               return i;

         // Not found
         return -1;
      }

      /// <summary>Tries to find an option item (case insensitive).</summary>
      /// <param name="value">item text.</param>
      /// <param name="index">The index if found, otherwise -1.</param>
      /// <returns></returns>
      bool  TryFind(const wstring& value, int& index) const
      {
         index = Find(value);
         return index != -1;
      }

      // ----------------------- MUTATORS ------------------------
   public:
      /// <summary>Called when value is being updated</summary>
      /// <returns></returns>
      BOOL OnUpdateValue() override
      {
         CString strText;

         // Get editing text
	      m_pWndInPlace->GetWindowText(strText);
         GuiString value((const wchar*)strText);
         
         try
         {
            // Validate/Modify value
            if (!OnValidateValue(value))
               return FALSE;

            // Replace input
            m_pWndInPlace->SetWindowTextW(value.c_str());

            // Success: Update property
            CMFCPropertyGridProperty::OnUpdateValue();
            OnValueChanged(value);
            return TRUE;
         }
         catch (ExceptionBase& e) {
            theApp.ShowError(HERE, e, GuiString(L"Unable to update %s property with value '%s'", GetName(), (const wchar*)strText));
            return FALSE;
         }
      }

      /// <summary>Validates and optionally modifies the value being input</summary>
      /// <param name="value">The value.</param>
      /// <returns>True to accept, false to reject</returns>
      virtual bool OnValidateValue(GuiString& value)
      {
         return true;
      }

      /// <summary>Called when value changed.</summary>
      /// <param name="value">value text.</param>
      virtual void OnValueChanged(GuiString value)
      {
      }

      // -------------------- REPRESENTATION ---------------------
   };



NAMESPACE_END2(GUI,Utils)

