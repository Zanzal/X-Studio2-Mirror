#pragma once
//

namespace Logic
{
   /// <summary>Represents a file or folder path</summary>
   class UtilExport Path 
   {
      // --------------------- CONSTRUCTION ----------------------
   public:
      Path();
      Path(const WCHAR* path);
      Path(const wstring& path);
      Path(const Path& r);
      Path(Path&& r);
      virtual ~Path();

      // ------------------------ STATIC -------------------------

   private:
      static CharArrayPtr  Init(const WCHAR*  path);

      // --------------------- PROPERTIES ------------------------
		
   public:
      PROPERTY_GET(wstring,Extension,GetExtension);
      PROPERTY_GET(wstring,FileName,GetFileName);
      PROPERTY_GET(Path,Folder,GetFolder);
      PROPERTY_GET(UINT,Length,GetLength);

		// ---------------------- ACCESSORS ------------------------
   public:
      Path     AppendBackslash() const;
      Path     Append(wstring  path) const;
      bool     Empty() const;
      bool     Exists() const;
      wstring  GetExtension() const;
      wstring  GetFileName() const;
      Path     GetFolder() const;
      UINT     GetLength() const;
      bool     HasExtension(const WCHAR* ext) const;
      bool     HasExtension(wstring ext) const;
      bool     IsDirectory() const;
      Path     RemoveBackslash() const;
      Path     RemoveExtension() const;
      Path     RenameExtension(wstring ext) const;
      Path     RenameFileName(wstring name) const;

      // Operators
      Path& operator=(const WCHAR* path)     { return Assign(path);               }
      Path& operator=(const wstring& path)   { return Assign(path.c_str());       }
      Path& operator=(const Path& path)      { return Assign(path.Buffer.get());  }
      Path& operator=(Path&& r);

      Path operator+(const WCHAR* path) const      { return Append(path);               }
      Path operator+(const wstring& path) const    { return Append(path);               }
      Path operator+(const Path& path) const       { return Append(path.c_str());       }

      bool operator==(const WCHAR* path) const     { return Compare(path) == 0;               }
      bool operator==(const wstring& path) const   { return Compare(path.c_str()) == 0;       }
      bool operator==(const Path& path) const      { return Compare(path.Buffer.get()) == 0;  }

      bool operator!=(const WCHAR* path) const     { return Compare(path) != 0;               }
      bool operator!=(const wstring& path) const   { return Compare(path.c_str()) != 0;       }
      bool operator!=(const Path& path) const      { return Compare(path.Buffer.get()) != 0;  }

      bool operator<(const Path& path) const       { return Compare(path.Buffer.get()) < 0;   }
      bool operator>(const Path& path) const       { return Compare(path.Buffer.get()) > 0;   }

      // Access
      const WCHAR* c_str() const                   { return Buffer.get(); }
      explicit operator WCHAR*()                   { return Buffer.get(); }
      explicit operator std::wstring() const       { return Buffer.get(); }
      wstring ToString() const                     { return Buffer.get(); }

   private:
      CharArrayPtr  Copy() const;
      int  Compare(const WCHAR* path) const;
      
		// ----------------------- MUTATORS ------------------------
   public:

   protected:
      Path&  Assign(const WCHAR*  text);
      
		// -------------------- REPRESENTATION ---------------------

   protected:
      CharArrayPtr  Buffer;
   };

   /// <summary>Represents the path of a file/folder in the application folder</summary>
   class UtilExport AppPath : public Path
   {
   public:
      AppPath();
      AppPath(const wstring& filename);
   };

   /// <summary>Represents the path of a temporary file</summary>
   class UtilExport TempPath : public Path
   {
   public:
      TempPath(const wchar* prefix = L"tmp");
   };

}