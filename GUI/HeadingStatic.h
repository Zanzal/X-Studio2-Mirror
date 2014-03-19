#pragma once

/// <summary>User interface</summary>
NAMESPACE_BEGIN2(GUI,Controls)
   
   /// <summary></summary>
   class HeadingStatic : public CStatic
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
   protected:
      HeadingStatic();    // Protected constructor used by dynamic creation
   public:
      virtual ~HeadingStatic();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNCREATE(HeadingStatic)
      DECLARE_MESSAGE_MAP()
	  
      // --------------------- PROPERTIES ------------------------
	  
      // ---------------------- ACCESSORS ------------------------			
   public:
      // ----------------------- MUTATORS ------------------------
   protected:
      void AdjustLayout();
      void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;

	   afx_msg void OnSize(UINT nType, int cx, int cy);
	  
      // -------------------- REPRESENTATION ---------------------
      
   private:
   };
   

NAMESPACE_END2(GUI,Controls)
