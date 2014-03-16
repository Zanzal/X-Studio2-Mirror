#pragma once



namespace Logic
{
   namespace FileSystem
   {
      /// <summary>Supported file types</summary>
      enum class FileType : UINT { Unknown, Script, Language, Mission,  Project, Universe };

      /// <summary>Get file-type name</summary>
      LogicExport wstring GetString(FileType t);

      /// <summary></summary>
      class LogicExport FileIdentifier
      {
		   // ------------------------ TYPES --------------------------
      private:
	  
         // --------------------- CONSTRUCTION ----------------------

      public:
         FileIdentifier();
         virtual ~FileIdentifier();
		 
		   DEFAULT_COPY(FileIdentifier);	// Default copy semantics
		   DEFAULT_MOVE(FileIdentifier);	// Default move semantics

         // ------------------------ STATIC -------------------------

         static FileType  Identify(Path  path);

         // --------------------- PROPERTIES ------------------------
			
         // ---------------------- ACCESSORS ------------------------			

         // ----------------------- MUTATORS ------------------------

         // -------------------- REPRESENTATION ---------------------

      private:
      };
   }
}


using namespace Logic::FileSystem;

