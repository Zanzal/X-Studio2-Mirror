#include "stdafx.h"
#include "ScriptValidator.h"
#include "../Logic/SyntaxLibrary.h"

namespace Testing
{
   namespace Scripts
   {
   
      // -------------------------------- CONSTRUCTION --------------------------------

      /// <summary>Creates source value read from an input stream</summary>
      /// <param name="src">The input stream</param>
      /// <exception cref="Logic::ArgumentException">Stream is not readable</exception>
      /// <exception cref="Logic::ArgumentNullException">Stream is null</exception>
      /// <exception cref="Logic::ComException">COM Error</exception>
      /// <exception cref="Logic::FileFormatException">File format is corrupt</exception>
      /// <exception cref="Logic::IOException">An I/O error occurred</exception>
      ScriptCodeValidator::ScriptCodeValidator(StreamPtr orig, StreamPtr copy)
         : In(orig), Out(copy)
      {
      }

      // ------------------------------- STATIC METHODS -------------------------------
      
      /// <summary>Create text mismatch exception</summary>
      /// <param name="src">throw source.</param>
      /// <param name="prop">name of property that mismatches</param>
      /// <param name="a">original</param>
      /// <param name="b">copy</param>
      /// <returns></returns>
      ValidationException  ScriptCodeValidator::CodeMismatch(const GuiString& src, const GuiString& prop, const GuiString& a, const GuiString& b)
      {
         return ValidationException(src, VString(L"code mismatch: %s", prop.c_str()), a, b);
      }

      /// <summary>Create text mismatch exception</summary>
      /// <param name="src">throw source.</param>
      /// <param name="prop">name of property that mismatches</param>
      /// <param name="a">original</param>
      /// <param name="b">copy</param>
      /// <returns></returns>
      ValidationException  ScriptCodeValidator::CodeMismatch(const GuiString& src, const GuiString& prop, const ParameterValue& a, const ParameterValue& b)
      {
         GuiString v1 = (a.Type == ValueType::String ? a.String : VString(L"0x%08X  (%d)", a.Int, a.Int));
         GuiString v2 = (b.Type == ValueType::String ? b.String : VString(L"0x%08X  (%d)", b.Int, b.Int));

         return CodeMismatch(src, prop, v1, v2);
      }
      
      /// <summary>Create text mismatch exception</summary>
      /// <param name="src">throw source.</param>
      /// <param name="prop">name of property that mismatches</param>
      /// <param name="a">original</param>
      /// <param name="b">copy</param>
      /// <returns></returns>
      ValidationException  ScriptCodeValidator::CodeMismatch(const GuiString& src, const GuiString& prop, int a, int b)
      {
         auto v1 = VString(L"0x%08X  (%d)", a, a);
         auto v2 = VString(L"0x%08X  (%d)", b, b);

         return CodeMismatch(src, prop, v1, v2);
      }

      // ------------------------------- PUBLIC METHODS -------------------------------
      
      /// <summary>Perform comparison of commands generated by compiler between script and it's validation copy</summary>
      bool  ScriptCodeValidator::Compare()
      {
         // Properties
         Compare(In.CodeArray, Out.CodeArray, 0, L"script name");
         //Compare(In.CodeArray, Out.CodeArray, 1, L"script engine version");
         Compare(In.CodeArray, Out.CodeArray, 2, L"script description");
         Compare(In.CodeArray, Out.CodeArray, 3, L"script version");
         Compare(In.CodeArray, Out.CodeArray, 4, L"script live data flag");
         Compare(In.CodeArray, Out.CodeArray, 9, L"script command ID");

         // Identify script version
         Version = EngineVersionConverter::ToGame( In.ReadInt(In.CodeArray, 1, L"script engine version") );

         // Vars/Args
         CompareVariables();
         CompareArguments();

         // Std/Aux commands
         CompareCommands(CommandType::Standard);
         CompareCommands(CommandType::Auxiliary);
         return true;
      }

      // ------------------------------ PROTECTED METHODS -----------------------------

      // ------------------------------- PRIVATE METHODS ------------------------------
      
      /// <summary>Compare type and value of two nodes</summary>
      bool  ScriptCodeValidator::Compare(XmlNodePtr parent_in, XmlNodePtr parent_out, UINT index, GuiString help)
      {
         // Read nodes
         auto v1 = In.ReadValue(parent_in, index, help.c_str());
         auto v2 = Out.ReadValue(parent_out, index, help.c_str());

         // Compare content
         if (v1 != v2)
            throw CodeMismatch(HERE, help, v1, v2);

         return true;
      }

      /// <summary>Compare argument branches</summary>
      void  ScriptCodeValidator::CompareArguments()
      {
         // Branch size
         CompareSize(In.CodeArray, Out.CodeArray, 7, L"arguments branch size");

         // Arguments
         auto in_args = In.GetChild(In.CodeArray, 7, L"arguments branch");
         auto out_args = Out.GetChild(Out.CodeArray, 7, L"arguments branch");

         for (int i = 0; i < in_args->childNodes->length; i++)
         {
            // Property count
            CompareSize(in_args, out_args, i, VString(L"argument %d sub-branch size", i+1));
            
            // Get sub-branch
            auto in_arg = In.GetChild(in_args, i, VString(L"argument %d sub-branch", i+1).c_str());
            auto out_arg = Out.GetChild(out_args, i, VString(L"argument %d sub-branch", i+1).c_str());
            
            // argument type/description
            Compare(in_arg, out_arg, 0, VString(L"argument %d type", i+1));
            Compare(in_arg, out_arg, 1, VString(L"argument %d description", i+1));
         }
      }

      /// <summary>Compare command branches</summary>
      /// <param name="cmdType">branch type</param>
      void  ScriptCodeValidator::CompareCommands(CommandType cmdType)
      {
         int codeArrayIndex  = (cmdType == CommandType::Standard ? 6 : 8);
         const wchar* branch = (cmdType == CommandType::Standard ? L"std" : L"aux");
         UINT initialIndex   = (cmdType == CommandType::Standard ? 0 : 1);

         // Verify branch size
         CompareSize(In.CodeArray, Out.CodeArray, codeArrayIndex, VString(L"%s commands branch size", branch).c_str());

         // Get branch
         auto in_cmds = In.GetChild(In.CodeArray, codeArrayIndex, VString(L"%s commands branch", branch).c_str());
         auto out_cmds = Out.GetChild(Out.CodeArray, codeArrayIndex, VString(L"%s commands branch", branch).c_str());

         // Verify each command
         for (int i = 0; i < in_cmds->childNodes->length; i++)
         {
            const CommandSyntax* syntax = nullptr;
            UINT nodeIndex = initialIndex;

            // Build location description
            auto line = VString(L"(%s %d) : ", branch, i);
            bool errors = false;

            // Get command branch
            auto in_cmd = In.GetChild(in_cmds, i, (line+L"sub-branch").c_str());
            auto out_cmd = Out.GetChild(out_cmds, i, (line+L"sub-branch").c_str());

            // [LABEL DEF] variously encoded as NOP/CmdComment, depending on the author.  All mine are encoded as CmdComment.
            if (cmdType == CommandType::Auxiliary && IsCommentedLabel(in_cmd, out_cmd, line))
               continue;

            // Get command ID
            Compare(in_cmd, out_cmd, nodeIndex, line + L"command ID");
            syntax = &SyntaxLib.Find(In.ReadInt(in_cmd, nodeIndex++, (line+L"command ID").c_str()), Version);

            // [AUX] Verify refIndex
            if (cmdType == CommandType::Auxiliary)
               // JMP is appended as child of break/continue, so my RefIndex associates with JMP, not the following Std command.
               //  This means that break/continue refIndex cannot be validated
               if (!syntax->Is(CMD_BREAK) && !syntax->Is(CMD_CONTINUE))
                  Compare(in_cmd, out_cmd, 0, line + L"RefIndex");

            // [CMD COMMENT] Get true command ID
            if (syntax->Is(CMD_COMMAND_COMMENT))
            {
               Compare(in_cmd, out_cmd, nodeIndex, line + L"commented command ID");
               syntax = &SyntaxLib.Find(In.ReadInt(in_cmd, nodeIndex++, (line+L"commented command ID").c_str()), Version);
            }

            // Improve location description
            line = VString(L"(%s %d) '%s' : ", branch, i, syntax->Text.c_str());

            // Node count:  (Skip for commands with known 'hidden' parameters)
            if (!syntax->Is(CMD_START_DELAYED_COMMAND) && !syntax->Is(CMD_SET_WING_COMMAND) && !syntax->Is(CMD_GET_OBJECT_NAME_ARRAY))
               CompareSize(in_cmds, out_cmds, i, line+L"node count");
            
            // parameters
            UINT paramIndex = 1;
            for (ParameterSyntax p : syntax->Parameters)
            {
               GuiString paramId(line + VString(L" param %d of %d : ", paramIndex++, syntax->ParameterCount) + GetString(p.Type));
               DataType  dt = DataType::UNKNOWN;

               try
               {
                  switch (p.Type)
                  {
                  // Single node
                  case ParameterType::COMMENT:        
                  case ParameterType::SCRIPT_NAME:    
                  case ParameterType::LABEL_NAME:     
                  case ParameterType::LABEL_NUMBER: 
                  case ParameterType::VARIABLE:          // Old 'var' parameter
                  case ParameterType::RETURN_VALUE:      
                  case ParameterType::RETURN_VALUE_IF:
                  case ParameterType::RETURN_VALUE_IF_START:
                  case ParameterType::INTERRUPT_RETURN_VALUE_IF: 
                     Compare(in_cmd, out_cmd, nodeIndex, paramId);
                     break;
                        
                  // Parameter as {Type,Value} pair
                  default:
                     dt = (DataType)In.ReadInt(in_cmd, nodeIndex, (paramId+L" DataType").c_str());

                     Compare(in_cmd, out_cmd, nodeIndex, line+paramId+GetString(p.Type)+L" (type)");
                     Compare(in_cmd, out_cmd, nodeIndex, line+paramId+GetString(p.Type)+L" (value)");
                        
                     ++nodeIndex;
                     break;
                  }
               }
               // Comparison failed: Print details of value
               catch (ValidationException& e) 
               {
                  Console.Log(HERE, e);
                  errors = true;

                  // Read values
                  int in_val  = In.ReadInt(in_cmd, nodeIndex, (paramId+L" Value").c_str()),
                      out_val = In.ReadInt(out_cmd, nodeIndex, (paramId+L" Value").c_str());
                     
                  // Print Variable names
                  switch (p.Type)
                  {
                  case ParameterType::COMMENT:        
                  case ParameterType::SCRIPT_NAME:    
                  case ParameterType::LABEL_NAME:     
                  case ParameterType::LABEL_NUMBER: 
                     break;

                  // Var/RetVar
                  case ParameterType::VARIABLE:          // Old 'var' parameter
                  case ParameterType::RETURN_VALUE:      
                  case ParameterType::RETURN_VALUE_IF:
                  case ParameterType::RETURN_VALUE_IF_START:
                  case ParameterType::INTERRUPT_RETURN_VALUE_IF: 
                     // Return Value: Print components
                     if (in_val < 0 || out_val < 0)  
                     {
                        Console << L"  Original RetVal: " << ReturnValue(in_val) << ENDL;
                        Console << L"  Copy RetVal: " << ReturnValue(out_val) << ENDL;
                        break;
                     }
                     else // RetVar: Print names
                        dt = DataType::VARIABLE;
                        // Fall thru...

                  // Parameter as {Type,Value} pair
                  default:
                     // Variable: Print names
                     if (dt == DataType::VARIABLE && in_val >= 0 && out_val >= 0)
                     {
                        Console << L"  Original var: " << InVars[in_val] << ENDL;
                        Console << L"  Copy var: " << OutVars[out_val] << ENDL;
                     }

                     ++nodeIndex;  // Re-align node index
                     break;
                  }
               }

               ++nodeIndex;
            }

            // Errors: Abort after printing all invalid parameters
            if (errors)
               throw GenericException(HERE, L"Stopping validation due to errors");
         }
      }


      /// <summary>Compare size of two array nodes</summary>
      bool  ScriptCodeValidator::CompareSize(XmlNodePtr parent_in, XmlNodePtr parent_out, UINT index, GuiString help)
      {
         // Read sizes
         auto v1 = In.ReadArray(parent_in, index, help.c_str());
         auto v2 = Out.ReadArray(parent_out, index, help.c_str());

         // Compare size
         if (v1 != v2)
            throw CodeMismatch(HERE, help, v1, v2);

         return true;
      }

      
      /// <summary>Compare variables branches</summary>
      void  ScriptCodeValidator::CompareVariables()
      {
         // Verify branch size
         CompareSize(In.CodeArray, Out.CodeArray, 5, L"variables branch size");

         // Get branches
         auto in_vars = In.GetChild(In.CodeArray, 5, L"variables branch");
         auto out_vars = Out.GetChild(Out.CodeArray, 5, L"variables branch");

         // Compare names/order
         for (int i = 0; i < in_vars->childNodes->length; i++)
         {
            Compare(in_vars, out_vars, i, VString(L"variable %d of %d", i+1, in_vars->childNodes->length));

            // Store for name resolution
            InVars.push_back( ScriptVariable(In.ReadString(in_vars, i, L"variable name"), i) );
            OutVars.push_back( ScriptVariable(Out.ReadString(out_vars, i, L"variable name"), i) );
         }
         
      }
      
      /// <summary>Check for a label definition encoded as a comment in the egosoft script, and an commented command by my compiler</summary>
      /// <param name="in_cmd">input command</param>
      /// <param name="out_cmd">output command</param>
      /// <param name="line">The line.</param>
      /// <returns>True if same label name but encoded different ways, otherwise false</returns>
      /// <exception cref="Testing::Scripts::ValidationException">Label names don't match</exception>
      bool  ScriptCodeValidator::IsCommentedLabel(XmlNodePtr in_cmd, XmlNodePtr out_cmd, const GuiString& line)
      {
         // Get command IDs
         UINT in_ID  = In.ReadInt(in_cmd, 1, (line+L"command ID").c_str()),
              out_ID = Out.ReadInt(out_cmd, 1, (line+L"command ID").c_str());

         // Check for Comment vs CommandComment
         if (in_ID == CMD_COMMENT && out_ID == CMD_COMMAND_COMMENT)
         {
            // Verify underlying ID is label definition
            if (Out.ReadInt(out_cmd, 2, (line+L"commented command ID").c_str()) != CMD_DEFINE_LABEL)
               return false;

            // [COMMENT vs COMMENTED DEFINE_LABEL] Read names from different indicies
            auto v1 = In.ReadString(in_cmd, 2, (line+L"label defintion").c_str());
            auto v2 = Out.ReadString(out_cmd, 3, (line+L"commented label definition").c_str());

            // Strip trailing colon + whitespace from comment version
            v1 = GuiString(v1).TrimRight(L": ");

            // Compare names
            if (v1 != v2)
               throw CodeMismatch(HERE, (line+L"commented label definition"), v1, v2);

            return true;
         }

         // Not a mismatched label
         return false;
      }

   }
}

