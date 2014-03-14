#pragma once
#include "Common.h"


namespace Logic
{
   namespace IO
   {
      /// <summary>Defines how a file is opened or created</summary>
      enum class FileMode 
      { 
         CreateNew = CREATE_NEW,                // Create new, fail if exists
         CreateAlways = CREATE_ALWAYS,          // Create new, overwrite if exists
         OpenAlways = OPEN_ALWAYS,              // Open file, create if does not exist
         OpenExisting = OPEN_EXISTING,          // Open file, fail if does not exist
         TruncateExisting = TRUNCATE_EXISTING   // Truncate existing, fail if does not exist
      };

      /// <summary>Defines read/write access on a stream</summary>
      enum class FileAccess : UINT 
      { 
         Read = GENERIC_READ,                      // Read access
         Write = GENERIC_WRITE,                    // Write access
         ReadWrite = GENERIC_READ|GENERIC_WRITE    // Read/Write access
      };

      /// <summary>Defines how a stream is shared</summary>
      enum class FileShare : UINT 
      { 
         AllowRead = FILE_SHARE_READ,        // Permit others to read
         AllowWrite = FILE_SHARE_WRITE,      // Permit others to write
         AllowDelete = FILE_SHARE_DELETE,    // Permit others to delete
         AllowNone = 0                       // Permit nothing
      };

      /// <summary>Defines the origin of a stream seeking operation</summary>
      enum class SeekOrigin 
      { 
         Begin = FILE_BEGIN,           // Beginning of file
         Current = FILE_CURRENT,       // Current position
         End = FILE_END                // End of file
      };

      /// <summary>Represents any stream</summary>
      class Stream
      {
         // --------------------- CONSTRUCTION ----------------------
      protected:
         Stream() {}
      public:
         virtual ~Stream() {  }

         // Prevent copying/moving
         NO_MOVE(Stream);
         NO_COPY(Stream);

         // --------------------- PROPERTIES ------------------------
			
			// ---------------------- ACCESSORS ------------------------
      public:
         virtual bool  CanRead() const PURE;
         virtual bool  CanSeek() const PURE;
         virtual bool  CanWrite() const PURE;

         virtual DWORD GetLength()  PURE;
         virtual DWORD GetPosition() const PURE;

         // ----------------------- MUTATORS ------------------------
      public:
         virtual void  Close() PURE;
         virtual void  Flush() PURE;
         virtual void  SafeClose() PURE;
         virtual void  Seek(LONG  offset, SeekOrigin  mode) PURE;
         virtual void  SetLength(DWORD  length) PURE;

         virtual DWORD Read(BYTE* buffer, DWORD length) PURE;
         virtual DWORD Write(const BYTE* buffer, DWORD length) PURE;
      
         /// <summary>Reads all remaining bytes from a stream into a buffer</summary>
         /// <returns>New byte array</returns>
         /// <exception cref="Logic::NotSupportedException">Stream is not readable</exception>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         virtual ByteArrayPtr ReadAllBytes()
         {
            DWORD        count = 0,
                         length = GetLength() - GetPosition();
            ByteArrayPtr buffer( new BYTE[length+1] );
            
            // Ensure null terminated
            buffer.get()[length] = '\0';

            // Empty:
            if (length == 0)
               return buffer;

            do
            {  // Read all bytes until EOF
               count += Read(&buffer.get()[count], length - count);
            } 
            while (count < length);

            // return buffer
            return buffer;
         }

         // -------------------- REPRESENTATION ---------------------
      };

      /// <summary>A shared pointer to any stream</summary>
      typedef shared_ptr<Stream>  StreamPtr;

      /// <summary>Allows for subclassing a stream using the Facade pattern</summary>
      class StreamFacade : public Stream
      {
         // --------------------- CONSTRUCTION ----------------------
      protected:
         /// <summary>Creates the stream using another stream as input</summary>
         /// <param name="src">The input stream.</param>
         /// <exception cref="Logic::ArgumentNullException">Stream is null</exception>
         StreamFacade(StreamPtr src) : Source(src)
         { 
            REQUIRED(src); 
         }
      public:
         /// <summary>Closes the stream without throwing</summary>
         virtual ~StreamFacade() 
         { 
            SafeClose();
         }

         // Prevent copying/moving
         NO_MOVE(StreamFacade);
         NO_COPY(StreamFacade);

			// --------------------- PROPERTIES ------------------------
			
			// ---------------------- ACCESSORS ------------------------
      public:
         /// <summary>Queries whether the stream is readable</summary>
         bool  CanRead() const                         { return Source->CanRead();  }

         /// <summary>Queries whether the stream is seekable</summary>
         bool  CanSeek() const                         { return Source->CanSeek();  }

         /// <summary>Queries whether the stream is writeable</summary>
         bool  CanWrite() const                        { return Source->CanWrite(); }

         /// <summary>Gets the file length</summary>
         /// <returns></returns>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         DWORD GetLength()                             { return Source->GetLength(); }

         /// <summary>Gets the current seek position.</summary>
         /// <returns></returns>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         DWORD GetPosition() const                     { return Source->GetPosition(); }

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Closes the stream.</summary>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         void  Close()                                 { return Source->Close(); }

         /// <summary>Flushes any data to the stream.</summary>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         void  Flush()                                 { return Source->Flush(); }

         /// <summary>Closes the stream without throwing.</summary>
         void  SafeClose()                             { Source->SafeClose();    }

         /// <summary>Seeks to the specified offset.</summary>
         /// <param name="offset">The offset in bytes</param>
         /// <param name="mode">The seek origin</param>
         /// <exception cref="Logic::NotSupportedException">Stream is not seekable</exception>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         void  Seek(LONG  offset, SeekOrigin  mode)    { return Source->Seek(offset, mode); }

         /// <summary>Sets the length of the stream</summary>
         /// <param name="length">The length in bytes</param>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         void  SetLength(DWORD  length)                { return Source->SetLength(length);  }

         /// <summary>Reads from the stream into the specified buffer.</summary>
         /// <param name="buffer">The destination buffer</param>
         /// <param name="length">The length of the buffer</param>
         /// <returns>Number of bytes read</returns>
         /// <exception cref="Logic::ArgumentNullException">Buffer is null</exception>
         /// <exception cref="Logic::NotSupportedException">Stream is not readable</exception>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         DWORD Read(BYTE* buffer, DWORD length)        { return Source->Read(buffer, length); }

         /// <summary>Writes the specified buffer to the stream</summary>
         /// <param name="buffer">The buffer.</param>
         /// <param name="length">The length of the buffer.</param>
         /// <returns>Number of bytes written</returns>
         /// <exception cref="Logic::ArgumentNullException">Buffer is null</exception>
         /// <exception cref="Logic::NotSupportedException">Stream is not writeable</exception>
         /// <exception cref="Logic::IOException">An I/O error occurred</exception>
         DWORD Write(const BYTE* buffer, DWORD length) { return Source->Write(buffer, length); }

         // -------------------- REPRESENTATION ---------------------
      private:
         StreamPtr  Source;
      };
      
   }

}

using namespace Logic::IO;
