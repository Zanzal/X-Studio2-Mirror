#pragma once
#include "DocumentBase.h"
#include "Logic/ProjectFile.h"
#include "Logic/Event.h"

/// <summary>User interface documents</summary>
NAMESPACE_BEGIN2(GUI,Documents)
   
   /// <summary>MSCI Script document template</summary>
   class ProjectDocTemplate : public CSingleDocTemplate
   {
      // --------------------- CONSTRUCTION ----------------------
   public:
      ProjectDocTemplate();

      // ------------------------ STATIC -------------------------

      DECLARE_DYNAMIC(ProjectDocTemplate)

      // ---------------------- ACCESSORS ------------------------	

      // ----------------------- MUTATORS ------------------------
   public:
      Confidence MatchDocType(LPCTSTR lpszPathName, CDocument*& rpDocMatch) override;
      CFrameWnd* CreateNewFrame(CDocument* pDoc, CFrameWnd* pOther) override;
      CDocument* OpenDocumentFile(LPCTSTR lpszPathName, BOOL bAddToMRU, BOOL bMakeVisible) override;
   };

   /// <summary>Project item added/removed events</summary>
   /// <typeparam name="item">Item added or removed</typeparam>
   /// <typeparam name="parent">Parent of item (may be null)</typeparam>
   typedef Event<ProjectItem*, ProjectItem*> ProjectChangedEvent;

   /// <summary>Project item added/removed event handler</summary>
   typedef ProjectChangedEvent::DelegatePtr  ProjectChangedHandler;

   /// <summary></summary>
   class ProjectDocument : public DocumentBase
   {
      // ------------------------ TYPES --------------------------
   private:
	  
      // --------------------- CONSTRUCTION ----------------------
   protected:
      ProjectDocument();    // Protected constructor used by dynamic creation
   public:
      virtual ~ProjectDocument();
       
      // ------------------------ STATIC -------------------------
      DECLARE_DYNCREATE(ProjectDocument)
      DECLARE_MESSAGE_MAP()

   public:
      static SimpleEvent          Loaded;
      static ProjectChangedEvent  ItemAdded,
                                  ItemRemoved;

      static ProjectDocument*  GetActive();
	  
      // --------------------- PROPERTIES ------------------------
   public:

      // ---------------------- ACCESSORS ------------------------			
   public:

      // ----------------------- MUTATORS ------------------------
   public:
      bool  AddFile(IO::Path path, ProjectFolderItem* parent);
      void  AddFolder(const wstring& name, ProjectFolderItem* parent);
      bool  Contains(IO::Path path) const;
      void  MoveItem(ProjectItem* item, ProjectFolderItem* folder);
      ProjectItemPtr  RemoveItem(ProjectItem* item);
      void  OnDocumentEvent(DocumentEvent deEvent) override;
      BOOL  OnNewDocument() override;
      BOOL  OnOpenDocument(LPCTSTR lpszPathName) override;
      BOOL  OnSaveDocument(LPCTSTR lpszPathName) override;

      // -------------------- REPRESENTATION ---------------------
   public:
      ProjectFile  Project;

   protected:
   
   };

   

/// <summary>User interface documents</summary>
NAMESPACE_END2(GUI,Documents)

