#include "ast.h"
#include "tube6-parser.tab.hh"

extern void yyerror(std::string err_string);
extern void yyerror2(std::string err_string, int orig_line);


/////////////////////
//  ASTNode

void ASTNode::TransferChildren(ASTNode * from_node)
{
  // Move all of the children out of the from_node
  for (int i = 0; i < from_node->GetNumChildren(); i++) {
    AddChild(from_node->GetChild(i));
  }

  // Clear the children in from_node so they are not recursively deleted when it is.
  from_node->children.resize(0);
}


/////////////////////
//  ASTNode_Block

tableEntry * ASTNode_Block::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  // Compile the code for each sub-tree below a block.
  for (int i = 0; i < (int) children.size(); i++) {
    tableEntry * cur_entry = children[i]->CompileTubeIC(table, ica);
    if (cur_entry != NULL && cur_entry->GetTemp() == true) {
      table.RemoveEntry( cur_entry );
    }
  }
  return NULL;
}


/////////////////////////
//  ASTNode_Variable

tableEntry * ASTNode_Variable::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  return var_entry;   // Return the symbol-table entry associated with this variable.
}


////////////////////////
//  ASTNode_Literal

ASTNode_Literal::ASTNode_Literal(int in_type, std::string in_lex)
  : ASTNode(in_type), lexeme(in_lex)
{
}  

tableEntry * ASTNode_Literal::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * out_var = table.AddTempEntry(type);
  if (type == Type::INT || type == Type::CHAR) {
    ica.Add("val_copy", lexeme, out_var->GetVarID());
  }
  else if (type == Type::CHAR_ARRAY) {
    // Drop the beginning and ending quotes and process escape chars.
    std::string str_value = "";
    for (int i = 1; i < (int) lexeme.size() - 1; i++) {
      if (lexeme[i] == '\\') {
        switch (lexeme[++i]) {
        case '\\': str_value += '\\'; break;
        case '"':  str_value += '"';  break;
        case 't':  str_value += '\t'; break;
        case 'n':  str_value += '\n'; break;
        };
      } else {
        str_value += lexeme[i];
      }
    }        

    std::stringstream size_str;
    size_str << str_value.size();
    ica.Add("ar_set_size", out_var->GetVarID(), size_str.str());
    for (int i = 0; i < (int) str_value.size(); i++) {
      std::stringstream set_val;
      switch (str_value[i]) {
      case '\\': set_val << "'\\\\'"; break;
      case '\t': set_val << "'\\t'"; break;
      case '\n': set_val << "'\\n'"; break;
      case '\'': set_val << "'\\''"; break;
      default: set_val << "'" << str_value[i] << "'";
      };

      std::stringstream idx_str;
      idx_str << i;
      ica.Add("ar_set_idx", out_var->GetVarID(), idx_str.str(), set_val.str());
    }
  }
  // --- Add code to deal with other literal types here! ---
  else {
    std::cerr << "INTERNAL ERROR: Unknown type!" << std::endl;
  }

  return out_var;
}


//////////////////////
// ASTNode_Assign

ASTNode_Assign::ASTNode_Assign(ASTNode * lhs, ASTNode * rhs)
  : ASTNode(lhs->GetType())
{ 
  if (lhs->GetType() != rhs->GetType()) {
    std::string err_message = "types do not match for assignment (lhs='";
    err_message += Type::AsString(lhs->GetType());
    err_message += "', rhs='";
    err_message += Type::AsString(rhs->GetType());
    err_message += "')";
    yyerror(err_message);
    exit(1);
  }
  children.push_back(lhs);
  children.push_back(rhs);
}

tableEntry * ASTNode_Assign::CompileTubeIC(symbolTable & table,
						IC_Array & ica)
{
  tableEntry * lhs_var = children[0]->CompileTubeIC(table, ica);
  tableEntry * rhs_var = children[1]->CompileTubeIC(table, ica);

  if (type == Type::INT || type == Type::CHAR) {
    // Determine if the lhs is an index into an array
    if (lhs_var->IsArrayIndex()) {
      ica.Add("ar_set_idx", lhs_var->GetArrayID(), lhs_var->GetIndexID(), rhs_var->GetVarID());
    } else {
      ica.Add("val_copy", rhs_var->GetVarID(), lhs_var->GetVarID());
    }
  }
  else if (type == Type::INT_ARRAY || type == Type::CHAR_ARRAY) {
    ica.Add("ar_copy", rhs_var->GetVarID(), lhs_var->GetVarID());
  }

  else {
    std::cerr << "Internal Compiler ERROR: Unknown type in Assign!" << std::endl;
    exit(1);
  }

  if (rhs_var->GetTemp() == true) table.RemoveEntry( rhs_var );

  return lhs_var;
}


/////////////////////
// ASTNode_Math1

ASTNode_Math1::ASTNode_Math1(ASTNode * in_child, int op)
  : ASTNode(Type::INT), math_op(op)
{
  if (in_child->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_child->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }
  children.push_back(in_child);
}

tableEntry * ASTNode_Math1::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * in_var = children[0]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);

  switch (math_op) {
  case '-':
    ica.Add("mult", in_var->GetVarID(), "-1", out_var->GetVarID());
    break;
  case '!':
    ica.Add("test_equ", in_var->GetVarID(), "0", out_var->GetVarID());
    break;
  default:
    std::cerr << "Internal compiler error: unknown Math1 operation '" << math_op << "'." << std::endl;
    exit(1);
  };

  if (in_var->GetTemp() == true) table.RemoveEntry( in_var );

  return out_var;
}


/////////////////////
// ASTNode_Math2

ASTNode_Math2::ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), math_op(op)
{
  bool rel_op = (op==COMP_EQU) || (op==COMP_NEQU) || (op==COMP_LESS) || (op==COMP_LTE) ||
    (op==COMP_GTR) || (op==COMP_GTE);

  ASTNode * in_test = (in1->GetType() != Type::INT) ? in1 : in2;
  if (in_test->GetType() != Type::INT) {
    if (!rel_op || in_test->GetType() != Type::CHAR) {
      std::string err_message = "cannot use type '";
      err_message += Type::AsString(in_test->GetType());
      err_message += "' in mathematical expressions";
      yyerror(err_message);
      exit(1);
    } else if (rel_op && (in1->GetType() != Type::CHAR || in2->GetType() != Type::CHAR)) {
      std::string err_message = "types do not match for relationship operator (lhs='";
      err_message += Type::AsString(in1->GetType());
      err_message += "', rhs='";
      err_message += Type::AsString(in2->GetType());
      err_message += "')";
      yyerror(err_message);
      exit(1);
    }
  }

  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Math2::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * in_var1 = children[0]->CompileTubeIC(table, ica);
  tableEntry * in_var2 = children[1]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);

  int i1 = in_var1->GetVarID();
  int i2 = in_var2->GetVarID();
  int o3 = out_var->GetVarID();

  // Determine the correct operation...  
  if (math_op == '+') { ica.Add("add", i1, i2, o3); }
  else if (math_op == '-') { ica.Add("sub",  i1, i2, o3); }
  else if (math_op == '*') { ica.Add("mult", i1, i2, o3); }
  else if (math_op == '/') { ica.Add("div",  i1, i2, o3); }
  else if (math_op == '%') { ica.Add("mod",  i1, i2, o3); }
  else if (math_op == COMP_EQU)  { ica.Add("test_equ",  i1, i2, o3); }
  else if (math_op == COMP_NEQU) { ica.Add("test_nequ", i1, i2, o3); }
  else if (math_op == COMP_GTR)  { ica.Add("test_gtr",  i1, i2, o3); }
  else if (math_op == COMP_GTE)  { ica.Add("test_gte",  i1, i2, o3); }
  else if (math_op == COMP_LESS) { ica.Add("test_less", i1, i2, o3); }
  else if (math_op == COMP_LTE)  { ica.Add("test_lte",  i1, i2, o3); }
  else {
    std::cerr << "INTERNAL ERROR: Unknown Math2 type '" << math_op << "'" << std::endl;
  }

  // Cleanup symbol table.
  if (in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  if (in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );

  return out_var;
}


/////////////////////
// ASTNode_Bool2

ASTNode_Bool2::ASTNode_Bool2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), bool_op(op)
{
  ASTNode * in_test = (in1->GetType() != Type::INT) ? in1 : in2;
  if (in_test->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_test->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }

  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_Bool2::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * in_var1 = children[0]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);
  std::string end_label = table.NextLabelID("end_bool_");

  // Convert the first answer to a 0 or 1 and put it in out_var.
  ica.Add("test_nequ", in_var1->GetVarID(), "0", out_var->GetVarID());

  // Determine the correct operation for short-circuiting...  
  if (bool_op == '&') {
    ica.Add("jump_if_0", out_var->GetVarID(), end_label, -1, "AND!");
  }
  else if (bool_op == '|') {
    ica.Add("jump_if_n0", out_var->GetVarID(), end_label, -1, "OR!");
  }
  else { std::cerr << "INTERNAL ERROR: Unknown Bool2 type '" << bool_op << "'" << std::endl; }

  // The output code should only get here if the first part didn't short-circuit...
  tableEntry * in_var2 = children[1]->CompileTubeIC(table, ica);

  // Convert the second answer to a 0 or 1 and put it in out_var.
  ica.Add("test_nequ", in_var2->GetVarID(), "0", out_var->GetVarID());

  // Leave the output label to jump to.
  ica.AddLabel(end_label);

  // Cleanup symbol table.
  if (in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  if (in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );

  return out_var;
}


///////////////////////////
// ASTNode_ArrayAccess

ASTNode_ArrayAccess::ASTNode_ArrayAccess(ASTNode * in1, ASTNode * in2)
  : ASTNode(Type::InternalType(in1->GetType()))
{
  if (Type::IsArray(in1->GetType()) == false) {
    std::string err_message = "cannot index into a non-array type '";
    err_message += Type::AsString(in1->GetType());
    err_message += "'.";
    yyerror(err_message);
    exit(1);
  }

  if (in2->GetType() != Type::INT) {
    yyerror ("array indices must be of type int");
    exit(1);
  }

  children.push_back(in1);
  children.push_back(in2);
}


tableEntry * ASTNode_ArrayAccess::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * in_var0 = children[0]->CompileTubeIC(table, ica);
  tableEntry * in_var1 = children[1]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);
    
  out_var->SetArrayIndex(in_var0, in_var1);
  ica.Add("ar_get_idx", in_var0->GetVarID(), in_var1->GetVarID(), out_var->GetVarID());

  // NOTE: We are not removing in_var0 and in_var1 yet since we may need them again.

  return out_var;
}


//////////////////////
// ASTNode_MethodCall

ASTNode_MethodCall::ASTNode_MethodCall(ASTNode * in_base, const std::string & in_name)
  : ASTNode(Type::VOID), name(in_name)
{
  children.push_back(in_base);

  if (name == "size") SetType(Type::INT);
  else if (name == "resize") SetType(Type::VOID);
  else {
    std::string err_message = "unknown method '";
    err_message += name;
    err_message += "'";
    yyerror(err_message);
    exit(1);
  }

  // The base of the method call must be an array (for now)
  if (!Type::IsArray(in_base->GetType())) {
    std::string err_message = "array methods cannot be run on type '";
    err_message += Type::AsString(in_base->GetType());
    err_message += "'";
    yyerror(err_message);
    exit(1);
  }
}


tableEntry * ASTNode_MethodCall::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * array_var = children[0]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);

  if (name == "size") {
    ica.Add("ar_get_size", array_var->GetVarID(), out_var->GetVarID());
  }
  else if (name == "resize") {
    tableEntry * size_var = children[1]->CompileTubeIC(table, ica);
    ica.Add("ar_set_size", array_var->GetVarID(), size_var->GetVarID());
    if (size_var->GetTemp() == true) table.RemoveEntry( size_var );
  }
  else {
    std::cerr << "Internal Compiler ERROR: Unknown method in ASTNode_MethodCall" << std::endl;
    exit(1);
  }

  // Cleanup symbol table.
  if (array_var->GetTemp() == true) table.RemoveEntry( array_var );

  return out_var;
}

void ASTNode_MethodCall::TypeCheckArgs()
{
  if (name == "size") {
    if (children.size() != 1) {
      yyerror("array size() method does not take any arguments.");
      exit(1);
    }
  }
  else if (name == "resize") {
    if (children.size() != 2) {
      yyerror("array resize() method takes exactly one (int) argument.");
      exit(1);
    }
    if (children[1]->GetType() != Type::INT) {
      yyerror("array resize() method argument must be of type int.");
      exit(1);
    }
  }
}


/////////////////////
// ASTNode_If

ASTNode_If::ASTNode_If(ASTNode * in1, ASTNode * in2, ASTNode * in3)
  : ASTNode(Type::VOID)
{
  if (in1->GetType() != Type::INT) {
    yyerror("condition for if statements must evaluate to type int");
    exit(1);
  }

  children.push_back(in1);
  children.push_back(in2);
  children.push_back(in3);
}


tableEntry * ASTNode_If::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  std::string else_label = table.NextLabelID("if_else_");
  std::string end_label = table.NextLabelID("if_end_");

  tableEntry * in_var0 = children[0]->CompileTubeIC(table, ica);

  // If the condition is false, jump to else.  Otherwise continue through if.
  ica.Add("jump_if_0", in_var0->GetVarID(), else_label);

  if (children[1]) {
    tableEntry * in_var1 = children[1]->CompileTubeIC(table, ica);
    if (in_var1 && in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  }

  // Now that we are done with "if", jump to the end; also start the else here.
  ica.Add("jump", end_label);
  ica.AddLabel(else_label);

  if (children[2]) {
    tableEntry * in_var2 = children[2]->CompileTubeIC(table, ica);
    if (in_var2 && in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );
  }

  // Close off the code with the end label.
  ica.AddLabel(end_label);

  return NULL;
}


/////////////////////
// ASTNode_While

ASTNode_While::ASTNode_While(ASTNode * in1, ASTNode * in2)
  : ASTNode(Type::VOID)
{
  children.push_back(in1);
  children.push_back(in2);

  if (in1->GetType() != Type::INT) {
    yyerror("condition for while statements must evaluate to type int");
    exit(1);
  }
}


tableEntry * ASTNode_While::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  std::string start_label = table.NextLabelID("while_start_");
  std::string end_label = table.NextLabelID("while_end_");

  table.PushWhileEndLabel(end_label);

  ica.AddLabel(start_label);

  tableEntry * in_var0 = children[0]->CompileTubeIC(table, ica);

  // If the condition is false, jump to end.  Otherwise continue through body.
  ica.Add("jump_if_0", in_var0->GetVarID(), end_label);

  if (children[1]) {
    tableEntry * in_var1 = children[1]->CompileTubeIC(table, ica);
    if (in_var1 && in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  }

  // Now that we are done with the while body, jump back to the start.
  ica.Add("jump", start_label);
  ica.AddLabel(end_label);

  table.PopWhileEndLabel();

  return NULL;
}


/////////////////////
// ASTNode_Break

ASTNode_Break::ASTNode_Break()
  : ASTNode(Type::VOID)
{
}


tableEntry * ASTNode_Break::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  if (table.GetWhileDepth() == 0) {
    yyerror2("'break' command used outside of any loop", line_num);
    //yyerror("'break' command used outside of any loop");
    exit(1);
  }

  ica.Add("jump", table.GetWhileEndLabel());

  return NULL;
}


/////////////////////
// ASTNode_Print

ASTNode_Print::ASTNode_Print(ASTNode * out_child)
  : ASTNode(Type::VOID)
{
  // Save the child...
  if (out_child != NULL) AddChild(out_child);
}


tableEntry * ASTNode_Print::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  // Collect the output arguments as they are calculated...
  for (int i = 0; i < (int) children.size(); i++) {
    tableEntry * cur_var = children[i]->CompileTubeIC(table, ica);
    switch (cur_var->GetType()) {
    case Type::INT:
      ica.Add("out_int", cur_var->GetVarID());
      break;
    case Type::CHAR:
      ica.Add("out_char", cur_var->GetVarID());
      break;
    case Type::CHAR_ARRAY:
    case Type::INT_ARRAY:
      {
        int size_id = table.GetTempVarID();
        int index_id = table.GetTempVarID();
        int entry_id = table.GetTempVarID();        
        std::string start_label = table.NextLabelID("print_array_start_");
        std::string end_label = table.NextLabelID("print_array_end_");
        
        ica.Add("val_copy", "0", index_id, -1, "Init loop variable for printing array.");
        ica.Add("ar_get_size", cur_var->GetVarID(), size_id, -1, "Save size of array into variable.");
        ica.AddLabel(start_label);

        ica.Add("test_gte", index_id, size_id, entry_id, "Test if we are finished yet...");
        ica.Add("jump_if_n0", entry_id, end_label, -1, " ...and jump to end if so.");

        ica.Add("ar_get_idx", cur_var->GetVarID(), index_id, entry_id,
                "Collect the value at the next index.");

        if (cur_var->GetType() == Type::CHAR_ARRAY) ica.Add("out_char", entry_id, -1, -1,
                                                            "Print this entry!");
        else if (cur_var->GetType() == Type::INT_ARRAY) ica.Add("out_int", entry_id, -1, -1,
                                                                "Print this entry!");

        ica.Add("add", index_id, "1", index_id, "Increment to the next index.");
        
        ica.Add("jump", start_label);
        ica.AddLabel(end_label);

        table.FreeTempVarID(size_id);
        table.FreeTempVarID(index_id);
        table.FreeTempVarID(entry_id);
      }
      break;
    default:
      std::cerr << "Internal Compiler ERROR: Unknown Type in Write::CompilerTubeIC" << std::endl;
      exit(1);
    };

    if (cur_var->GetTemp() == true) table.RemoveEntry( cur_var );
  }
  ica.Add("out_char", "'\\n'", -1, -1, "End print statements with a newline.");
  
  return NULL;
}


//////////////////////
//  ASTNode_Random

ASTNode_Random::ASTNode_Random(ASTNode * in_child)
  : ASTNode(Type::INT)
{
  if (in_child->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_child->GetType());
    err_message += "' as an argument to random";
    yyerror(err_message);
    exit(1);
  }
  children.push_back(in_child);
}

tableEntry * ASTNode_Random::CompileTubeIC(symbolTable & table, IC_Array & ica)
{
  tableEntry * in_var = children[0]->CompileTubeIC(table, ica);
  tableEntry * out_var = table.AddTempEntry(type);

  ica.Add("random", in_var->GetVarID(), out_var->GetVarID());

  if (in_var->GetTemp() == true) table.RemoveEntry( in_var );

  return out_var;
}

