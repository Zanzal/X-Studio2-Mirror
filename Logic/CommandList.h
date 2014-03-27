#pragma once
#include "ScriptCommand.h"

namespace Logic
{
   namespace Scripts
   {

      /// <summary>List of script commands</summary>
      class LogicExport CommandList : public list<ScriptCommand>
      {
         // --------------------- CONSTRUCTION ----------------------

         // ------------------------ STATIC -------------------------

         // --------------------- PROPERTIES ------------------------
			
		   // ---------------------- ACCESSORS ------------------------	
      public:
         /// <summary>Determines if 'define label' command is a label definition or subroutine</summary>
         /// <param name="cmd">The command.</param>
         /// <returns></returns>
         bool IsSubRoutine(iterator cmd) const
         {
            if (!cmd->Is(CMD_DEFINE_LABEL))
               return false;

            // Linear search following commands
            for (auto p = ++cmd; p != end(); ++p)
               // Label: Not a subroutine
               if (p->Is(CMD_DEFINE_LABEL))
                  return false;
               // EndSub: Subroutine
               else if (p->Is(CMD_END_SUB))
                  return true;

            // EOF: Not a subroutine
            return false;
         }

         /// <summary>Find command by index.</summary>
         /// <param name="index">The index.</param>
         /// <returns></returns>
         /// <exception cref="Logic::IndexOutOfRangeException">Invalid index</exception>
         ScriptCommand& operator[](UINT index)
         {
            UINT i = 0;

            // Linear search
            for (auto pos = begin(); pos != end(); ++pos)
               if (index == i++)
                  return *pos;

            // Invalid index
            throw IndexOutOfRangeException(HERE, index, size());
         }

         // ----------------------- MUTATORS ------------------------

         // -------------------- REPRESENTATION ---------------------
      };

      
      /// <summary>Script Command list iterator</summary>
      typedef CommandList::iterator  CommandIterator;

   }
}

using namespace Logic::Scripts;
