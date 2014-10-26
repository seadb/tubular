#include "ast.h"
#include "tube5-parser.tab.hh"

extern void yyerror(std::string err_string);
extern void yyerror2(std::string err_string, int orig_line);


/////////////////////
//  ASTNode

void ASTNode::TransferChildren(ASTNode * from_node)
{
  // Move all of the mChildren out of the from_node
  for (int i = 0; i < from_node->GetNumChildren(); i++) {
    AddChild(from_node->GetChild(i));
  }

  // Clear the mChildren in from_node so they are not recursively deleted when it is.
  from_node->mChildren.resize(0);
}


/////////////////////
//  ASTNodeBlock

CTableEntry * ASTNodeBlock::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  // Compile the code for each sub-tree below a block.
  for (int i = 0; i < (int) mChildren.size(); i++) {
    CTableEntry * cur_entry = mChildren[i]->CompileTubeIC(table, ica);
    if (cur_entry != NULL && cur_entry->GetTemp() == true) {
      table.RemoveEntry( cur_entry );
    }
  }
  return NULL;
}


/////////////////////////
//  ASTNodeVariable

CTableEntry * ASTNodeVariable::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  return mVarEntry;   // Return the symbol-table entry associated with this variable.
}


////////////////////////
//  ASTNodeLiteral

ASTNodeLiteral::ASTNodeLiteral(int in_type, std::string in_lex)
                            : ASTNode(in_type), mLexeme(in_lex)
{
}  

ASTNodeLiteral::ASTNodeLiteral(int in_type, char * in_char)
                            : ASTNode(in_type), mCharArray(in_char)
{
  std::cout << "mCharArrayConstructor" << std::endl;
}

CTableEntry * ASTNodeLiteral::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  CTableEntry * out_var = table.AddTempEntry(mType);
  if (mType == Type::INT || mType == Type::CHAR) 
  {
    std::cout << "mLexeme" << mLexeme << std::endl;
    ica.Add("val_copy", mLexeme, out_var->GetVarID());
  }
  else if (mType == Type::INT_ARRAY || mType == Type::CHAR_ARRAY)
  {
    std::stringstream s;
    std::stringstream ss; ss << mLexeme.size(); //convert the size to a string
    ica.Add("ar_set_size", out_var->GetVarID(), ss.str());
    for(int i=0; i < mLexeme.size(); i++ )
    {
      ss.str(""); ss.clear(); ss << i; //convert the index to string
      s.str(""); s.clear(); s << "'" <<  mLexeme[i] << "'"; 
      ica.Add("ar_set_idx", out_var->GetVarID(), ss.str(), s.str());
    }
    //TODO: add intermediate code to ica
  }
  // --- Add code to deal with other literal types here! ---
  else 
  {
    std::cerr << "INTERNAL ERROR: Unknown type!" << std::endl;
  }

  return out_var;
}


//////////////////////
// ASTNodeAssign

ASTNodeAssign::ASTNodeAssign(ASTNode * lhs, ASTNode * rhs)
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
  mChildren.push_back(lhs);
  mChildren.push_back(rhs);
}

CTableEntry * ASTNodeAssign::CompileTubeIC(CSymbolTable & table,
						ICArray & ica)
{
  CTableEntry * lhs_var = mChildren[0]->CompileTubeIC(table, ica);
  CTableEntry * rhs_var = mChildren[1]->CompileTubeIC(table, ica);

  if (mType == Type::INT || mType == Type::CHAR) {
    ica.Add("val_copy", rhs_var->GetVarID(), lhs_var->GetVarID());
  }
  else if (mType == Type::INT_ARRAY || mType == Type::CHAR_ARRAY){
    //TODO: add intermediate code to ica
    //ica.Add("ar_set_idx")
  }
  else if (mType = Type::INT_ARRAY_IDX || mType == Type::CHAR_ARRAY_IDX)
  {
    CTableEntry * left = mChildren[0]->CompileTubeIC(table, ica);
    CTableEntry * right = mChildren[0]->CompileTubeIC(table,ica);
     

  }
  // --- Add code to deal with other types of assignments here! ---
  else {
    std::cerr << "Internal Compiler ERROR: Unknown type in Assign!" << std::endl;
    exit(1);
  }

  if (rhs_var->GetTemp() == true) table.RemoveEntry( rhs_var );

  return lhs_var;
}


/////////////////////
// ASTNodeMath1

ASTNodeMath1::ASTNodeMath1(ASTNode * in_child, int op)
                     : ASTNode(Type::INT), mMathOp(op)
{
  if (in_child->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_child->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }
  mChildren.push_back(in_child);
}

CTableEntry * ASTNodeMath1::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  CTableEntry * in_var = mChildren[0]->CompileTubeIC(table, ica);
  CTableEntry * out_var = table.AddTempEntry(mType);

  switch (mMathOp) {
  case '-':
    ica.Add("mult", in_var->GetVarID(), "-1", out_var->GetVarID());
    break;
  case '!':
    ica.Add("test_equ", in_var->GetVarID(), "0", out_var->GetVarID());
    break;
  default:
    std::cerr << "Internal compiler error: unknown Math1 operation '" << mMathOp << "'." << std::endl;
    exit(1);
  };

  if (in_var->GetTemp() == true) table.RemoveEntry( in_var );

  return out_var;
}


/////////////////////
// ASTNodeMath2

ASTNodeMath2::ASTNodeMath2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), mMathOp(op)
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

  mChildren.push_back(in1);
  mChildren.push_back(in2);
}


CTableEntry * ASTNodeMath2::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(table, ica);
  CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(table, ica);
  CTableEntry * out_var = table.AddTempEntry(mType);

  int i1 = in_var1->GetVarID();
  int i2 = in_var2->GetVarID();
  int o3 = out_var->GetVarID();

  // Determine the correct operation...  
  if (mMathOp == '+') { ica.Add("add", i1, i2, o3); }
  else if (mMathOp == '-') { ica.Add("sub",  i1, i2, o3); }
  else if (mMathOp == '*') { ica.Add("mult", i1, i2, o3); }
  else if (mMathOp == '/') { ica.Add("div",  i1, i2, o3); }
  else if (mMathOp == '%') { ica.Add("mod",  i1, i2, o3); }
  else if (mMathOp == COMP_EQU)  { ica.Add("test_equ",  i1, i2, o3); }
  else if (mMathOp == COMP_NEQU) { ica.Add("test_nequ", i1, i2, o3); }
  else if (mMathOp == COMP_GTR)  { ica.Add("test_gtr",  i1, i2, o3); }
  else if (mMathOp == COMP_GTE)  { ica.Add("test_gte",  i1, i2, o3); }
  else if (mMathOp == COMP_LESS) { ica.Add("test_less", i1, i2, o3); }
  else if (mMathOp == COMP_LTE)  { ica.Add("test_lte",  i1, i2, o3); }
  else {
    std::cerr << "INTERNAL ERROR: Unknown Math2 type '" << mMathOp << "'" << std::endl;
  }

  // Cleanup symbol table.
  if (in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  if (in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );

  return out_var;
}


/////////////////////
// ASTNodeBool2

ASTNodeBool2::ASTNodeBool2(ASTNode * in1, ASTNode * in2, int op)
  : ASTNode(Type::INT), mBoolOp(op)
{
  ASTNode * in_test = (in1->GetType() != Type::INT) ? in1 : in2;
  if (in_test->GetType() != Type::INT) {
    std::string err_message = "cannot use type '";
    err_message += Type::AsString(in_test->GetType());
    err_message += "' in mathematical expressions";
    yyerror(err_message);
    exit(1);
  }

  mChildren.push_back(in1);
  mChildren.push_back(in2);
}


CTableEntry * ASTNodeBool2::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(table, ica);
  CTableEntry * out_var = table.AddTempEntry(mType);
  std::string end_label = table.NextLabelID("end_bool_");

  // Convert the first answer to a 0 or 1 and put it in out_var.
  ica.Add("test_nequ", in_var1->GetVarID(), "0", out_var->GetVarID());

  // Determine the correct operation for short-circuiting...  
  if (mBoolOp == '&') {
    ica.Add("jump_if_0", out_var->GetVarID(), end_label, -1, "AND!");
  }
  else if (mBoolOp == '|') {
    ica.Add("jump_if_n0", out_var->GetVarID(), end_label, -1, "OR!");
  }
  else { std::cerr << "INTERNAL ERROR: Unknown Bool2 type '" << mBoolOp << "'" << std::endl; }

  // The output code should only get here if the first part didn't short-circuit...
  CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(table, ica);

  // Convert the second answer to a 0 or 1 and put it in out_var.
  ica.Add("test_nequ", in_var2->GetVarID(), "0", out_var->GetVarID());

  // Leave the output label to jump to.
  ica.AddLabel(end_label);

  // Cleanup symbol table.
  if (in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  if (in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );

  return out_var;
}


/////////////////////
// ASTNodeIf

ASTNodeIf::ASTNodeIf(ASTNode * in1, ASTNode * in2, ASTNode * in3)
  : ASTNode(Type::VOID)
{
  if (in1->GetType() != Type::INT) {
    yyerror("condition for if statements must evaluate to type int");
    exit(1);
  }

  mChildren.push_back(in1);
  mChildren.push_back(in2);
  mChildren.push_back(in3);
}


CTableEntry * ASTNodeIf::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  std::string else_label = table.NextLabelID("if_else_");
  std::string end_label = table.NextLabelID("if_end_");

  CTableEntry * in_var0 = mChildren[0]->CompileTubeIC(table, ica);

  // If the condition is false, jump to else.  Otherwise continue through if.
  ica.Add("jump_if_0", in_var0->GetVarID(), else_label);

  if (mChildren[1]) {
    CTableEntry * in_var1 = mChildren[1]->CompileTubeIC(table, ica);
    if (in_var1 && in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  }

  // Now that we are done with "if", jump to the end; also start the else here.
  ica.Add("jump", end_label);
  ica.AddLabel(else_label);

  if (mChildren[2]) {
    CTableEntry * in_var2 = mChildren[2]->CompileTubeIC(table, ica);
    if (in_var2 && in_var2->GetTemp() == true) table.RemoveEntry( in_var2 );
  }

  // Close off the code with the end label.
  ica.AddLabel(end_label);

  return NULL;
}


/////////////////////
// ASTNodeWhile

ASTNodeWhile::ASTNodeWhile(ASTNode * in1, ASTNode * in2)
  : ASTNode(Type::VOID)
{
  mChildren.push_back(in1);
  mChildren.push_back(in2);

  if (in1->GetType() != Type::INT) {
    yyerror("condition for while statements must evaluate to type int");
    exit(1);
  }
}


CTableEntry * ASTNodeWhile::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  std::string start_label = table.NextLabelID("while_start_");
  std::string end_label = table.NextLabelID("while_end_");

  table.PushWhileEndLabel(end_label);

  ica.AddLabel(start_label);

  CTableEntry * in_var0 = mChildren[0]->CompileTubeIC(table, ica);

  // If the condition is false, jump to end.  Otherwise continue through body.
  ica.Add("jump_if_0", in_var0->GetVarID(), end_label);

  if (mChildren[1]) {
    CTableEntry * in_var1 = mChildren[1]->CompileTubeIC(table, ica);
    if (in_var1 && in_var1->GetTemp() == true) table.RemoveEntry( in_var1 );
  }

  // Now that we are done with the while body, jump back to the start.
  ica.Add("jump", start_label);
  ica.AddLabel(end_label);

  table.PopWhileEndLabel();

  return NULL;
}


/////////////////////
// ASTNodeBreak

ASTNodeBreak::ASTNodeBreak()
  : ASTNode(Type::VOID)
{
}


CTableEntry * ASTNodeBreak::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  if (table.GetWhileDepth() == 0) {
    yyerror2("'break' command used outside of any loop", mLineNum);
    //yyerror("'break' command used outside of any loop");
    exit(1);
  }

  ica.Add("jump", table.GetWhileEndLabel());

  return NULL;
}


/////////////////////
// ASTNodePrint

ASTNodePrint::ASTNodePrint(ASTNode * out_child)
  : ASTNode(Type::VOID)
{
  // Save the child...
  if (out_child != NULL) AddChild(out_child);
}


CTableEntry * ASTNodePrint::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{
  // Collect the output arguments as they are calculated...
  for (int i = 0; i < (int) mChildren.size(); i++) {
    CTableEntry * cur_var = mChildren[i]->CompileTubeIC(table, ica);
    switch (cur_var->GetType()) {
    case Type::INT:
      ica.Add("out_int", cur_var->GetVarID());
      break;
    case Type::CHAR:
      ica.Add("out_char", cur_var->GetVarID());
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

/////////////////////////
// ASTNodeIndex

ASTNodeIndex::ASTNodeIndex(CTableEntry * array, ASTNode * index) 
    : ASTNode(array->GetType()), mArray(array) 
{
  
  int indexType = index->GetType();
  if( indexType != Type::INT  )
    {
      std::string errString = "array indices must be of type int";
      yyerror(errString);
      exit(1);
    }
  mIndex = index; 

  int idType = array->GetType();           
  if (idType != Type::INT_ARRAY && idType != Type::CHAR_ARRAY) 
  {
    std::string errString = "cannot index into a non-array type";
    yyerror(errString);
    exit(1);
  }
  else if( idType == Type::INT_ARRAY)
  {
    mType = Type::INT;

  }
  else
  {
    mType = Type::CHAR;
  } 
}

CTableEntry * ASTNodeIndex::CompileTubeIC(CSymbolTable & table, ICArray & ica)
{ 
  CTableEntry * index = mIndex->CompileTubeIC(table, ica);
  CTableEntry * out_var = table.AddTempEntry(mType);
  
  int i2 = mArray->GetVarID();
  int o3 = out_var->GetVarID();

  
  std::stringstream ss;
  ss << index->GetVarID(); 
  mArray->SetIndex(ss.str());

  ica.Add("ar_get_idx", mArray->GetVarID(), ss.str(), o3);
  
  return mArray; 

}

