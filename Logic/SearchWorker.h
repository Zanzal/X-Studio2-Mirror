#pragma once


#include "BackgroundWorker.h"
#include "MatchData.h"

namespace Logic
{
   namespace Threads
   {
      /// <summary>Forward declaration</summary>
      class SearchWorker;

      /// <summary>Type of search being performed</summary>
      enum class SearchCommand { Find, Replace, FindAll, ReplaceAll };

      /// <summary>Get search target name</summary>
      LogicExport wstring  GetString(SearchTarget t);

      

      /// <summary>Data for find and replace worker thread</summary>
      class LogicExport SearchWorkerData : public WorkerData
      {
         friend class SearchWorker;
         // ------------------------ TYPES --------------------------
      private:

         // --------------------- CONSTRUCTION ----------------------
      public:
         SearchWorkerData(Threads::Operation op, SearchTarget targ, const MatchData& search, Path folder, GameVersion ver) 
            : WorkerData(op), 
              Folder(folder), 
              Version(ver), 
              Target(targ), 
              Match(search), 
              Initialized(false),
              Command(SearchCommand::Find)
         {}
         virtual ~SearchWorkerData()
         {}

         // --------------------- PROPERTIES ------------------------

         // ---------------------- ACCESSORS ------------------------	

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Feedbacks the error.</summary>
         /// <param name="msg">The MSG.</param>
         void  FeedbackError(wstring msg)
         {
            SendFeedback(ProgressType::Error, 1, msg);
         }

         /// <summary>Feedbacks finish notification.</summary>
         void  FeedbackFinish(bool cancelled)
         {
            if (!cancelled)
               SendFeedback(Cons::UserAction, ProgressType::Succcess, 0, L"No more matches");
            else
               SendFeedback(Cons::UserAction, ProgressType::Failure, 0, L"Operation cancelled");
         }

         /// <summary>Feedbacks the match.</summary>
         void  FeedbackMatch()
         {
            SendFeedback(ProgressType::Info, 1, VString(L"%s (%d) : %s", Match.FullPath.FileName.c_str(), 
                                                                           Match.LineNumber, 
                                                                           Match.LineText.c_str()));
         }

         /// <summary>Feedbacks start notification.</summary>
         void  FeedbackStart()
         {
            // Feedback
            auto txt = (Match.UseRegEx ? L"Searching for '%s' in %s using regular expressions" : L"Searching for '%s' in %s");
            SendFeedback(ProgressType::Operation, 0, VString(txt, Match.SearchTerm.c_str(), GetString(Target).c_str()) );
         }

         // -------------------- REPRESENTATION ---------------------
      public:
         Path        Folder;
         MatchData       Match;
         SearchTarget    Target;
         GameVersion     Version;
         SearchCommand   Command;
         
      private:
         bool            Initialized;
         list<Path>  Files;
      };

      /// <summary>Find and replace worker thread</summary>
      class LogicExport SearchWorker : public BackgroundWorker
      {
         // ------------------------ TYPES --------------------------
      private:

         // --------------------- CONSTRUCTION ----------------------
      public:
         SearchWorker();
         virtual ~SearchWorker();

         DEFAULT_COPY(SearchWorker);	// Default copy semantics
         DEFAULT_MOVE(SearchWorker);	// Default move semantics

         // ------------------------ STATIC -------------------------
      protected:
         static void  BuildFileList(SearchWorkerData* data);
         static DWORD WINAPI ThreadMain(SearchWorkerData* data);

         // --------------------- PROPERTIES ------------------------
      
         // ---------------------- ACCESSORS ------------------------			

         // ----------------------- MUTATORS ------------------------
      public:
         /// <summary>Starts the thread.</summary>
         /// <param name="param">operation data.</param>
         /// <exception cref="Logic::ArgumentNullException">param is null</exception>
         /// <exception cref="Logic::InvalidOperationException">Thread already running</exception>
         /// <exception cref="Logic::Win32Exception">Failed to start Thread</exception>
         void  Start(SearchCommand cmd, SearchWorkerData* param)
         {
            REQUIRED(param);

            // Set command + Start.
            param->Command = cmd;
            BackgroundWorker::Start(param);
         }

         // -------------------- REPRESENTATION ---------------------

      private:
      };

   }
}

using namespace Logic::Threads;
