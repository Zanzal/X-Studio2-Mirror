#include "stdafx.h"
#include "EncryptedX2Stream.h"

namespace Logic
{
   namespace IO
   {
      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Creates an encrypted file stream using another stream as input</summary>
      /// <param name="src">The input stream.</param>
      /// <exception cref="Logic::ArgumentNullException">Stream is null</exception>
      EncryptedX2Stream::EncryptedX2Stream(StreamPtr  src) : StreamDecorator(src)
      {
      }

      /// <summary>Creates an encrypted file stream using a file as input</summary>
      /// <param name="path">The full path.</param>
      /// <param name="mode">The creation mode.</param>
      /// <param name="access">The file access.</param>
      /// <param name="share">The sharing permitted.</param>
      /// <exception cref="Logic::FileNotFoundException">File not found</exception>
      /// <exception cref="Logic::IOException">An I/O error occurred</exception>
      EncryptedX2Stream::EncryptedX2Stream(Path path, FileMode mode, FileAccess access, FileShare share)
         : StreamDecorator( StreamPtr(new FileStream(path, mode, access, share)) )
      {
      }

      /// <summary>Closes the stream without throwing</summary>
      EncryptedX2Stream::~EncryptedX2Stream()
      {
         StreamDecorator::SafeClose();
      }

      // ------------------------------- STATIC METHODS -------------------------------
      
      /// <summary>Determines whether a stream uses X2 file encryption.</summary>
      /// <param name="s">stream.</param>
      /// <returns></returns>
      bool  EncryptedX2Stream::IsEncrypted(StreamPtr s)
      {
         // Verify length
         if (s->GetLength() < 3)
            return false;
         
         // Prepare
         WORD  header;
         DWORD origin = s->GetPosition();

         // Read 2-byte header and reset position
         s->Seek(1, SeekOrigin::Begin);
         s->Read(reinterpret_cast<BYTE*>(&header), 2);
         s->Seek(origin, SeekOrigin::Begin);

         // Check for encrypted GZip header 
         return (header ^ 0x7e7e) == 0x8b1f;
      }

      // ------------------------------- PUBLIC METHODS -------------------------------

      /// <summary>Reads/decodes from the stream into the specified buffer.</summary>
      /// <param name="buffer">The destination buffer</param>
      /// <param name="length">The length of the buffer</param>
      /// <returns>Number of bytes read</returns>
      /// <exception cref="Logic::ArgumentNullException">Buffer is null</exception>
      /// <exception cref="Logic::NotSupportedException">Stream is not readable</exception>
      /// <exception cref="Logic::IOException">An I/O error occurred</exception>
      DWORD  EncryptedX2Stream::Read(BYTE* buffer, DWORD length)
      {
         REQUIRED(buffer);

         // Skip first byte
         if (StreamDecorator::GetPosition() == 0)
            StreamDecorator::Seek(1, SeekOrigin::Begin);

         // Read+decode buffer
         DWORD bytesRead = StreamDecorator::Read(buffer, length);
         Encode(buffer, bytesRead);

         // Return length
         return bytesRead;
      }

      /// <summary>Writes/encodes the specified buffer to the stream</summary>
      /// <param name="buffer">The buffer.</param>
      /// <param name="length">The length of the buffer.</param>
      /// <returns>Number of bytes written</returns>
      /// <exception cref="Logic::ArgumentNullException">Buffer is null</exception>
      /// <exception cref="Logic::NotSupportedException">Stream is not writeable</exception>
      /// <exception cref="Logic::IOException">An I/O error occurred</exception>
      DWORD  EncryptedX2Stream::Write(const BYTE* buffer, DWORD length)
      {
         REQUIRED(buffer);

         // Copy buffer so we can encode it
         ByteArrayPtr copy(new BYTE[length]);
         memcpy(copy.get(), buffer, length);

         // Encode + Write
         Encode(copy.get(), length);
         return StreamDecorator::Write(copy.get(), length);
      }

      // ------------------------------ PROTECTED METHODS -----------------------------

		// ------------------------------- PRIVATE METHODS ------------------------------

      /// <summary>Encodes a byte array</summary>
      /// <param name="buffer">Buffer to encode</param>
      /// <param name="length">Length of buffer</param>
      void  EncryptedX2Stream::Encode(byte* buffer, DWORD length)
      {
         // Encode buffer
         for (DWORD i = 0; i < length; i++)
            buffer[i] ^= DECRYPT_KEY;
      }

   }
}