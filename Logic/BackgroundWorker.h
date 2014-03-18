#pragma once
#include "WorkerData.h"


namespace Logic
{
   namespace Threads
   {

      /// <summary>Background worker pattern</summary>
      class LogicExport BackgroundWorker
      {
         // ------------------------ TYPES --------------------------
      private:
	  
         // --------------------- CONSTRUCTION ----------------------
      protected:
         /// <summary>Create background worker.</summary>
         /// <param name="pfn">worker function.</param>
         /// <exception cref="Logic::ArgumentNullException">function is nullptr</exception>
         BackgroundWorker(ThreadProc pfn) : Proc(pfn), Thread(nullptr)
         {
            REQUIRED(pfn);
         }
      public:
         /// <summary>Stops the background worker without throwing</summary>
         virtual ~BackgroundWorker()
         {
            // Stop if running
            Stop();
         }
      
         // ------------------------ STATIC -------------------------
      
         // --------------------- PROPERTIES ------------------------
	  
         // ---------------------- ACCESSORS ------------------------			
      public:
         /// <summary>Determines whether thread is running.</summary>
         /// <returns></returns>
         bool  IsRunning() const
         {
            DWORD code(0);

            // Check if closed
            if (Thread == nullptr)
               return false;

            // Query thread state
            if (!GetExitCodeThread(Thread, &code))
               throw Win32Exception(HERE, L"Unable to query thread state");

            // return true if active
            return code == STILL_ACTIVE;
         }

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Closes the thread handle.</summary>
         void  Close()
         {
            // Close handle
            CloseHandle(Thread);
            Thread = nullptr;
         }

         /// <summary>Sets the 'abort' flag and closes the thread handle</summary>
         void  Stop()
         {
            // Ensure running
            if (IsRunning())
               Data->Abort();

            // Close handle
            Close();
         }

      protected:
         /// <summary>Starts the thread.</summary>
         /// <param name="param">operation data.</param>
         /// <exception cref="Logic::ArgumentNullException">param is nullptr -or- parent window is nullptr</exception>
         /// <exception cref="Logic::InvalidOperationException">Thread already running</exception>
         /// <exception cref="Logic::Win32Exception">Failed to start Thread</exception>
         void  Start(WorkerData* param)
         {
            // Ensure data valid
            REQUIRED(param);
            REQUIRED(param->GetParent());
               
            // Ensure not started
            if (Thread)
               throw InvalidOperationException(HERE, L"Thread already running");

            // Launch thread
            Thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Proc, (void*)(Data=param), NULL, NULL);

            if (!Thread)
               throw Win32Exception(HERE, L"Unable to start thread");
         }

         // -------------------- REPRESENTATION ---------------------
      private:
         ThreadProc   Proc;
         WorkerData*  Data;
         HANDLE       Thread;
      };
   

   }
}

using namespace Logic::Threads;
