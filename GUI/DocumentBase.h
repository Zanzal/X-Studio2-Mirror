#pragma once
#include "../Logic/MatchData.h"
#include "../Logic/TemplateFile.h"

/// <summary>User interface documents</summary>
NAMESPACE_BEGIN2(GUI,Documents)
   
   /// <summary></summary>
   enum class DocumentType { Script, Language, Mission, Project, Diff };

   /// <summary></summary>
   class DocumentBase : public CDocument
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
   protected:
      DocumentBase();
      DocumentBase(DocumentType type, bool isVirtual);
   public:
      virtual ~DocumentBase();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNCREATE(DocumentBase)
      DECLARE_MESSAGE_MAP()

   public:
      static DocumentBase* GetActive();
      static void RenameFile(Path oldPath, Path newPath);

      // --------------------- PROPERTIES ------------------------
   public:
      PROPERTY_GET_SET(Path,FullPath,GetFullPath,SetFullPath);
      PROPERTY_GET_SET(bool,Virtual,GetVirtual,SetVirtual);
      PROPERTY_GET(wstring,FileName,GetFileName);

      // ---------------------- ACCESSORS ------------------------			
   public:
      virtual bool      FindNext(UINT start, MatchData& m) const;
      virtual CHARRANGE GetSelection() const;
      wstring           GetFileName() const;
      Path              GetFullPath() const;
      DocumentType      GetType() const;
      bool              GetVirtual() const;

      // ----------------------- MUTATORS ------------------------
   public:
      void          Activate();
      virtual BOOL  CloseModified();
      BOOL          DoSave(LPCTSTR szPathName, BOOL bReplace = TRUE) override;
      void          OnCloseDocument() override;
      void          OnDocumentEvent(DocumentEvent deEvent) override;
      virtual BOOL  OnOpenTemplate(Path docPath, const TemplateFile& t);
      virtual BOOL  OnReloadDocument(LPCTSTR szPathName);
      virtual void  Rename(Path newPath);
      virtual bool  Replace(MatchData& m);
      BOOL          SaveModified() override;
      virtual void  SetFullPath(Path path);
      void          SetModifiedFlag(BOOL bModified = TRUE) override;
      void          SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE) override;
      virtual void  SetSelection(CHARRANGE rng);
      void          SetTitle(LPCTSTR title) override;
      void          SetVirtual(bool v);

   protected:
      afx_msg void  OnCommand_Close();
      afx_msg void  OnCommand_Reload();
      afx_msg void  OnCommand_Save();
      afx_msg void  OnCommand_SaveAs();
      afx_msg void  OnQueryCommand(CCmdUI* pCmdUI);
	   
      // -------------------- REPRESENTATION ---------------------
   public:
      const static UINT DOCUMENT_RELOADED = 1;     // Update flag indicating document reloaded

   protected:
      DocumentType  Type;           // Document type

   private:
      bool          IsVirtual;      // Whether document is virtual
   };

   /// <summary>List of documents</summary>
   typedef list<DocumentBase*>  DocumentList;
   

/// <summary>User interface documents</summary>
NAMESPACE_END2(GUI,Documents)

