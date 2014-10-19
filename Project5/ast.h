#ifndef AST_H
#define AST_H

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>

#include "symbol_table.h"
// The classes in this file hold info about the nodes that form the Abstract Syntax Tree (AST)
//
// CNode : The base class for all of the others, with useful virtual functions.
//
// CNodeBlock : Blocks of statements, including the overall program.
// CNodeVariable : Leaf node containing a variable.
// CNodeLiteral : Leaf node contiaing a literal value.
// CNodeAssign : Assignements
// CNodeMath2 : Two-input math operations (here, just '+' and '-')

class CNode {
protected:
  int mLineNum;                     // What line of the source program generated this node?
  std::vector<CNode *> mChildren;  // What sub-trees does this node have?
  std::string mLeftType;
  std::string mRightType;

public:
  CNode(int lineNum=-1) : mLineNum(lineNum) { ; }
  void yyerr(std::string errorString) {
  	std::cout << "ERROR(line " << mLineNum << "): " << errorString << std::endl;
  	exit(1);
  }
  virtual ~CNode() {
    for (int i = 0; i < (int) mChildren.size(); i++) {
      if (mChildren[i] != NULL) delete mChildren[i];
    }
  }

  // Accessors
  int GetLineNum() const { return mLineNum; }
  CNode * GetChild(int id) { return mChildren[id]; }
  unsigned int GetNumChildren() const { return mChildren.size(); }

  void SetLineNum(int lineNum) { mLineNum = lineNum; }

  CNode * RemoveChild(int id) {
    CNode * outChild = mChildren[id];
    mChildren[id] = NULL;
    return outChild;
  }

  // Add a new child to this node, at the end of the vector.
  void AddChild(CNode * child) { mChildren.push_back(child); }

  // Convert a single node to TubeIC and return information about the
  // variable where the results are saved.  Call mChildren recursively.
  virtual CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) = 0;

  // Return the name of the node being called.  This function is useful for debbing the AST.
  virtual std::string GetName() { return "CNode (base class)"; }

  // Debug function to help make sure the AST is being built correctly...
  void DebugPrint(int depth=0) {
    for (int i = 0; i < depth; i++) std::cout << ".";  // Indent appropriately
    std::cout << GetName() << std::endl;               // Print this node's name
    for (int i = 0; i < (int) mChildren.size(); i++) {  // Print all of the mChildren
      mChildren[i]->DebugPrint(depth+1);
    }
  }
};


// A temporary node used to transfer mChildren...
class CNodeTemp : public CNode {
public:
  CNodeTemp() { ; }
  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    std::cerr << "Internal Compiler Error: Trying to run CompileTubeIC on a temporary node!!" << std::endl;
    return NULL;
  }

  std::string GetName() { return "CNodeTemp (temporary container class)"; }
};

// Block...
class CNodeBlock : public CNode {
public:
  CNodeBlock() { ; }
  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    // Compile the code for each sub-tree in this block
    for (int i = 0; i < (int) mChildren.size(); i++) {
      mChildren[i]->CompileTubeIC(tables, out);
    }
    return NULL;
  }

  std::string GetName() { return "CNodeBlock (container class)"; }
};

// Leaves...
class CNodeVariable : public CNode {
private:
  CTableEntry * mVarEntry;
public:
  CNodeVariable(CTableEntry * entry) : mVarEntry(entry) {;}

  CTableEntry * GetVarEntry() { return mVarEntry; }
  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) { 
    return mVarEntry; 
  }

  std::string GetName() {
    std::string out_string = "CNodeVariable (";
    out_string += mVarEntry->GetName();
    out_string += ")";
    return out_string;
  }

};

class CNodeLiteral : public CNode {
private:
  std::string mLexeme;
  std::string mType;
public:
  CNodeLiteral(std::string in_lex, std::string in_type) : mLexeme(in_lex), mType(in_type) { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * out_var = tables.AddTempEntry();
    out << "val_copy " << mLexeme << " s" << out_var->GetVarID() << std::endl;
    out_var->SetType(mType);
    return out_var;
  }

  std::string GetName() {
    std::string out_string = "CNodeLiteral (";
    out_string += mLexeme;
    out_string += ")";
    return out_string;
  }

};

// Math...

class CNodeAssign : public CNode {
public:
  CNodeAssign(CNode * lhs, CNode * rhs, int  lineNum) {
    mLineNum = lineNum;
    mChildren.push_back(lhs);
    mChildren.push_back(rhs);
  }

  ~CNodeAssign() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * lhs_var = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * rhs_var = mChildren[1]->CompileTubeIC(tables, out);
    mLeftType = lhs_var->GetType();
    mRightType = rhs_var->GetType();
    if (mLeftType != mRightType) {
	std::string e;
	e += "types do not match for assignment (lhs='";
	e += mLeftType; 
	e += "', rhs='";
	e += mRightType;
	e += "')";
	yyerr(e);
    }
    out << "val_copy s" <<  rhs_var->GetVarID() << " s" << lhs_var->GetVarID() << std::endl;
    lhs_var->SetType(mLeftType);
    return lhs_var;
    }

  std::string GetName() { return "CNodeAssign (operator=)"; }
};

class CNodeMathAssign : public CNode {
protected:
  int mMathOp;
public:
  CNodeMathAssign(CNode * lhs, CNode * rhs, int op, int lineNum) {
    mChildren.push_back(lhs);
    mChildren.push_back(rhs);
    mMathOp = op;
    mLineNum = lineNum;
  }

  ~CNodeMathAssign() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * lhs_var = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * rhs_var = mChildren[1]->CompileTubeIC(tables, out);
    mLeftType = lhs_var->GetType();
    mRightType = rhs_var->GetType();
    if (mLeftType == "char" || mRightType == "char") {
	std::string e;
	e += "cannot use type 'char' in mathematical expressions";
	yyerr(e);
    } else if (mRightType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mRightType;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int l = lhs_var->GetVarID();
    const int r = rhs_var->GetVarID();

    // Determine the correct operation...
    switch(mMathOp){
      case '+':
        out << "add s" << l << " s" << r << " s" << l << std::endl;
        break;
      case '-':
        out << "sub s" << l << " s" << r << " s" << l << std::endl;
        break;
      case '*':
        out << "mult s" << l << " s" << r << " s" << l << std::endl;
        break;
      case '/':
        out << "div s" << l << " s" << r << " s" << l << std::endl;
        break;
      case '%':
        out << "mod s" << l << " s" << r << " s" << l << std::endl;
        break;
      default:
        std::cerr << "INTERNAL COMPILER ERROR: Unknown MathAssign operator '"
                << mMathOp << "'" << std::endl;
        break;
    }

    /*
    if (mMathOp == '+') {
      out << "add s" << l << " s" << r << " s" << l << std::endl;
    } else if (math_op == '-') {
      out << "sub s" << l << " s" << r << " s" << l << std::endl;
    } else if (math_op == '*') {
      out << "mult s" << l << " s" << r << " s" << l << std::endl;
    } else if (math_op == '/') {
      out << "div s" << l << " s" << r << " s" << l << std::endl;
    } else if (math_op == '%') {
      out << "mod s" << l << " s" << r << " s" << l << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown MathAssign operator '"
                << math_op << "'" << std::endl;
    }*/

    lhs_var->SetType("int");

    return lhs_var;
  }

  std::string GetName() {
    std::string out_string = "CNodeMathAssign (operator";
    out_string += (char) mMathOp;
    out_string += ")";
    return out_string;
  }
};

class CNodeMath2 : public CNode {
protected:
  int mMathOp;
public:
  CNodeMath2(CNode * in1, CNode * in2, int op, int lineNum) : mMathOp(op) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mLineNum = lineNum;
  }
  ~CNodeMath2() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();
    mLeftType = in_var1->GetType();
    mRightType = in_var2->GetType();
    if (mLeftType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mLeftType;
        e += "' in mathematical expressions";
        yyerr(e);
    } else if (mRightType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mRightType;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();
    

    switch(mMathOp){
      case '+':
        out << "add s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '-':
        out << "sub s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '*':
        out << "mult s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '/':
        out << "div s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '%':
        out << "mod s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      default:
        std::cerr << "INTERNAL COMPILER ERROR: Unknown Math2 operator '"
                << mMathOp << "'" << std::endl;
        break;
    }

    out_var->SetType("int");
    return out_var;
        /*
    // Determine the correct operation...
    if (math_op == '+') {
      out << "add s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (math_op == '-') {
      out << "sub s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (math_op == '*') {
      out << "mult s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (math_op == '/') {
      out << "div s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (math_op == '%') {
      out << "mod s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Math2 operator '"
                << math_op << "'" << std::endl;
    }
    */
  }

  std::string GetName() {
    std::string out_string = "CNodeMath2 (operator";
    out_string += (char) mMathOp;
    out_string += ")";
    return out_string;
  }
};

class CNodeNegation : public CNode {
public:
  CNodeNegation(CNode * in, int lineNum) {
    mChildren.push_back(in);
    mLineNum = lineNum;
  }
  ~CNodeNegation() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();
    mLeftType = in_var->GetType();
    if (mLeftType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mLeftType;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int i = in_var->GetVarID();
    const int o = out_var->GetVarID();

    out << "val_copy -1 s" << o << std::endl;
    out << "mult s" << o << " s" << i << " s" << o << std::endl;
    out_var->SetType("int");
    return out_var;
  }

  std::string GetName() {
    return "CNodeNegation";
  }
};

class CNodeNot : public CNode {
public:
  CNodeNot(CNode * in, int lineNum) {
    mChildren.push_back(in);
    mLineNum = lineNum;
  }
  ~CNodeNot() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();
    mLeftType = in_var->GetType();
    if (mLeftType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mLeftType;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int i = in_var->GetVarID();
    const int o = out_var->GetVarID();

    out << "test_equ s" << i << " 0 s" << o << std::endl;
    out_var->SetType("int");
    return out_var;
  }

  std::string GetName() {
    return "CNodeNot";
  }
};

class CNodeComparison : public CNode {
protected:
  std::string mCompOp;
public:
  CNodeComparison(CNode * in1, CNode * in2, std::string op, int lineNum) : mCompOp(op) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mLineNum = lineNum;
  }
  ~CNodeComparison() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();

    mLeftType = in_var1->GetType();
    mRightType = in_var2->GetType();
    if (mLeftType != mRightType) {
	std::string e;
	e += "types do not match for relationship operator (lhs = '";
	e += mLeftType;
	e += "', rhs = '";
	e += mRightType;
	e += "')";
	yyerr(e);
    }
    out_var->SetType("int");
    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();

    if (mCompOp == "<") {
      out << "test_less s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (mCompOp == ">") {
      out << "test_gtr s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (mCompOp == "<=") {
      out << "test_lte s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (mCompOp == ">=") {
      out << "test_gte s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (mCompOp == "==") {
      out << "test_equ s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (mCompOp == "!=") {
      out << "test_nequ s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Comparison operator '"
                << mCompOp << "'" << std::endl;
    }

        return out_var;
  }

  std::string GetName() {
    std::string out_string = "CNodeComparison (operator";
    out_string += mCompOp;
    out_string += ")";
    return out_string;
  }
};

class CNodeLogical : public CNode {
protected:
  std::string mLogOp;
public:
  CNodeLogical(CNode * in1, CNode * in2, std::string op, int lineNum) : mLogOp(op) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mLineNum = lineNum;
  }
  ~CNodeLogical() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();
    mLeftType = in_var1->GetType();
    if (mLeftType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mLeftType;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int i1 = in_var1->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    if (mLogOp == "&&") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_0 s" << o3 << " and" << o3 << std::endl;
      CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
      mRightType = in_var2->GetType();
      if (mRightType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mRightType;
        e += "' in mathematical expressions";
        yyerr(e);
      }
      //std::cout << "Logical type 2: " << mRightType << std::endl;
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "and" << o3 << ":" << std::endl;
    } 
    
    else if (mLogOp == "||") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_n0 s" << o3 << " or" << o3 << std::endl;
      CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
      mRightType = in_var2->GetType();
      if (mRightType == "char") {
        std::string e;
        e += "cannot use type '";
        e += mRightType;
        e += "' in mathematical expressions";
        yyerr(e);
      }
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "or" << o3 << ":" << std::endl;
    }

    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Logical operator '"
                << mLogOp << "'" << std::endl;
    }
    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "CNodeLogical (operator";
    out_string += mLogOp;
    out_string += ")";
    return out_string;
  }
};

class CNodeConditional : public CNode {
public:
  CNodeConditional(CNode * in1, CNode * in2, CNode * in3, int lineNum) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mChildren.push_back(in3);
    mLineNum = lineNum;
  }
  ~CNodeConditional() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " cond_true" << o4 << std::endl;

    // False
    CTableEntry * in_var3 = mChildren[2]->CompileTubeIC(tables, out);
    const int i3 = in_var3->GetVarID();
    out << "val_copy s" << i3 << " s" << o4 << std::endl;
    out << "jump cond_end" << o4 << std::endl;

    // True
    out << "cond_true" << o4 << ":" << std::endl;
    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
    const int i2 = in_var2->GetVarID();
    out << "val_copy s" << i2 << " s" << o4 << std::endl;

    out << "cond_end" << o4 << ":" << std::endl;

    out_var->SetType("int");

    return out_var;
  }

  std::string GetName() {
    return "CNodeConditional";
  }
};

class CNodeIf : public CNode {
public:
  CNodeIf(CNode * in1, CNode * in2, int lineNum) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mLineNum = lineNum;
  }
  ~CNodeIf() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    mLeftType = in_var1->GetType();
    if (mLeftType != "int") {
	std::string e;
	e += "condition for if statements must evaluate to type int";
	yyerr(e);
    }
    CTableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " if_true" << o4 << std::endl;

    // False
    out << "jump if_end" << o4 << std::endl;

    // True
    out << "if_true" << o4 << ":" << std::endl;
    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);

    out << "if_end" << o4 << ":" << std::endl;

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "CNodeConditional";
  }
};

class CNodeWhile : public CNode {
public:
  CNodeWhile(CNode * in1, CNode * in2, int lineNum) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mLineNum = lineNum;
  }
  ~CNodeWhile() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * out_var = tables.AddTempEntry();
    const int o4 = out_var->GetVarID();

    out << "while_begin" << o4 << ":" << std::endl;

    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    mLeftType = in_var1->GetType();
    if (mLeftType != "int") {
	std::string e;
	e += "condition for while statements must evaluate to type int";
	yyerr(e);
    }
    const int i1 = in_var1->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_0 s" << o4 << " while_end" << o4 << std::endl;

    std::stringstream ss;
    ss << "while_end" << o4;
    tables.breaks.push_back(ss.str());

    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);

    if(tables.breaks.size() > 0) tables.breaks.pop_back();

    out << "jump while_begin" << o4 << std::endl;

    out << "while_end" << o4 << ":" << std::endl;

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "CNodeConditional";
  }
};

class CNodeFor : public CNode {
public:
  CNodeFor(CNode * in1, CNode * in2, CNode *in3,
                CNode *in4, int lineNum) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mChildren.push_back(in3);
    mChildren.push_back(in4);
    mLineNum = lineNum;
  }
  ~CNodeFor() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * out_var = tables.AddTempEntry();
    const int o4 = out_var->GetVarID();

    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);

    out << "for_begin" << o4 << ":" << std::endl;

    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
    mLeftType = in_var2->GetType();

    if (mLeftType != "int") {
	  std::string e;
	  e += "condition for for statements must evaluate to type int";
	  yyerr(e);
    }
    const int i2 = in_var2->GetVarID();

    out << "test_nequ s" << i2 << " 0 s" << o4 << std::endl;
    out << "jump_if_0 s" << o4 << " for_end" << o4 << std::endl;

    std::stringstream ss;
    ss << "for_end" << o4;
    tables.breaks.push_back(ss.str());

    CTableEntry * in_var4 = mChildren[3]->CompileTubeIC(tables, out);

    if(tables.breaks.size() > 0)
      tables.breaks.pop_back();

    CTableEntry * in_var3 = mChildren[2]->CompileTubeIC(tables, out);

    out << "jump for_begin" << o4 << std::endl;

    out << "for_end" << o4 << ":" << std::endl;

    out_var->SetType(in_var1->GetType());
    return out_var;
  }

  std::string GetName() {
    return "CNodeFor";
  }
};

class CNodeBreak : public CNode {
public:
  CNodeBreak(int lineNum) {
    mLineNum = lineNum;
  }
  ~CNodeBreak() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    if(tables.breaks.size() > 0) {
      out << "jump " << tables.breaks.back() << std::endl;
      //breaks.pop_back();
    }
    else {
	std::string e;
	e += "'break' command used outside of any loop";
	yyerr(e);
    }

    return NULL;
  }

  std::string GetName() {
    return "CNodeConditional";
  }
};

class CNodePrint : public CNode {
public:
  CNodePrint() { ; }
  virtual ~CNodePrint() { ; }

  virtual CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out)
  {
    for (int i = 0; i < (int) mChildren.size(); i++) {
      CTableEntry * in_var = mChildren[i]->CompileTubeIC(tables, out);
      if (in_var->GetType() == "int") {
      	out << "out_int s" << in_var->GetVarID() << std::endl;
      } else if (in_var->GetType() == "char") {
	out << "out_char s" << in_var->GetVarID() << std::endl;
      }
    }
    out << "out_char '\\n'" << std::endl;

    return NULL;
  }
};

class CNodeRandom : public CNode {
public:
  CNodeRandom(CNode * in, int lineNum) {
    mChildren.push_back(in);
    mLineNum = lineNum;
  }

  ~CNodeRandom() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();
    mLeftType = in_var->GetType();
    if (mLeftType != "int") {
	std::string e;
	e += "cannot use type '";
	e += mLeftType;
	e += "' as an argument to random";
	yyerr(e);
    }
    out << "random s" <<  in_var->GetVarID() << " s" << out_var->GetVarID() << std::endl;

    out_var->SetType(in_var->GetType());

    return out_var;
  }
};

class CNodeElse : public CNode {
public:
  CNodeElse(CNode * in1, CNode * in2, CNode * in3, int lineNum) {
    mChildren.push_back(in1);
    mChildren.push_back(in2);
    mChildren.push_back(in3);
    mLineNum = lineNum;
  }
  ~CNodeElse() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * in_var1 = mChildren[0]->CompileTubeIC(tables, out);
    CTableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " if_true" << o4 << std::endl;

    // False
    out << "jump else_label" << o4 << std::endl;

    // True
    out << "if_true" << o4 << ":" << std::endl;
    CTableEntry * in_var2 = mChildren[1]->CompileTubeIC(tables, out);
    out << "jump end_label" << o4 << std::endl;

    //Else
    out << "else_label" << o4 << ":" << std::endl;
    CTableEntry * in_var3 = mChildren[2]->CompileTubeIC(tables, out);

    out << "end_label" << o4 << ":" << std::endl;

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "CNodeConditional";
  }
};

class CNodeBlank : public CNode {
public:
  CNodeBlank() { ; }

  CTableEntry * CompileTubeIC(CSymbolTables & tables, std::ostream & out) {
    CTableEntry * out_var = tables.AddTempEntry();
    out_var->SetType("int");
    return out_var;
  }

  std::string GetName() {
    std::string out_string = "CNodeBlank";
    return out_string;
  }

};

#endif
