#ifndef AST_H
#define AST_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The classes in this file hold info about the nodes that form the Abstract Syntax Tree (AST)
//
//  ASTNode : The base class for all of the others, with useful virtual functions.
//
//  ASTNodeTempNode : AST Node that will be replaced (used for argument lists).
//  ASTNodeBlock : Blocks of statements, including the overall program.
//  ASTNodeVariable : Leaf node containing a variable.
//  ASTNodeLiteral : Leaf node contiaing a literal value.
//  ASTNodeAssign : Assignements
//  ASTNodeMath1 : One-input math operations (unary '-' and '!')
//  ASTNodeMath2 : Two-input math operations ('+', '-', '*', '/', '%', and comparisons)
//  ASTNodeBool2 : Two-input bool operations ('&&' and '||')
//  ASTNodeIf : If-conditional node.
//  ASTNodeWhile : While-loop node.
//  ASTNodeBreak : Break node
//  ASTNodePrint : Print command
//

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "ic.h"
#include "type_info.h"
#include "symbol_table.h"

class ASTNode {
protected:
  int mType;                         // What type should this node pass up?
  int mLineNum;                     // What line of the source program generated this node?
  std::vector<ASTNode *> mChildren;  // What sub-trees does this node have?
  bool mDebug;

  void SetType(int new_type) { mType = new_type; } // Use inside constructor only!
public:
  ASTNode(int in_type) : mType(in_type), mLineNum(-1), mDebug(false) { ; }
  virtual ~ASTNode() {
    for (int i = 0; i < (int) mChildren.size(); i++) delete mChildren[i];
  }

  int GetType()              { return mType; }
  int GetLineNum()           { return mLineNum; }
  int GetDebug()             { return mDebug; }
  ASTNode * GetChild(int id) { return mChildren[id]; }
  int GetNumChildren()       { return mChildren.size(); }

  void SetLineNum(int _in)                 { mLineNum = _in; }
  void SetDebug(bool inDebug)              { mDebug = inDebug; }
  void SetChild(int id, ASTNode * in_node) { mChildren[id] = in_node; }
  void AddChild(ASTNode * in_child)        { mChildren.push_back(in_child); }
  void TransferChildren(ASTNode * in_node);

  // Convert a single node to TubeIC and return information about the
  // variable where the results are saved.  Call mChildren recursively.
  virtual CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica) = 0;
};


// A placeholder node in the AST.
class ASTNodeTempNode : public ASTNode {
public:
  ASTNodeTempNode(int in_type) : ASTNode(in_type) { ; }
  ~ASTNodeTempNode() { ; }
  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica) { return NULL; }
};

// Block...
class ASTNodeBlock : public ASTNode {
public:
  ASTNodeBlock() : ASTNode(Type::VOID) { ; }
  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

// Leaves...
class ASTNodeVariable : public ASTNode {
private:
  CTableEntry * mVarEntry;
public:
  ASTNodeVariable(CTableEntry * in_entry)
    : ASTNode(in_entry->GetType()), mVarEntry(in_entry) {;}

  CTableEntry * GetVarEntry() { return mVarEntry; }
  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeIndex : public ASTNode {
  private:
    CTableEntry * mArray;
    ASTNode * mIndex;
  public:
    ASTNodeIndex(CTableEntry * entry, ASTNode * index, bool debug);
    ASTNode * GetIndex() { return mIndex; }
    CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);

};

class ASTNodeSize : public ASTNode {
  private:
    CTableEntry * mArray;
  public:
    ASTNodeSize(CTableEntry * entry);
    CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeResize : public ASTNode {
  private:
    CTableEntry * mArray;
    ASTNode * mSize;
  public:
    ASTNodeResize(CTableEntry * entry, ASTNode *size);
    CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeLiteral : public ASTNode {
private:
 std::string mLexeme;     // When we print, how should this node look?
 char * mCharArray;
 int * mIntArray;
public:
  ASTNodeLiteral(int in_type, std::string in_lex);
  ASTNodeLiteral(int in_type, char * in_char);
  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};
/*
class ASTNodeArray : public ASTNode {
  public:
    ASTNodeArray(std::string in_array);
    ASTNodeArray(int * in_array);
    CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
}*/
// Math...

class ASTNodeAssign : public ASTNode {
public:
  ASTNodeAssign(ASTNode * lhs, ASTNode * rhs);
  ~ASTNodeAssign() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeMath1 : public ASTNode {
protected:
  int mMathOp;
public:
  ASTNodeMath1(ASTNode * in_child, int op);
  virtual ~ASTNodeMath1() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeMath2 : public ASTNode {
protected:
  int mMathOp;
public:
  ASTNodeMath2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNodeMath2() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeBool2 : public ASTNode {
protected:
  int mBoolOp;
public:
  ASTNodeBool2(ASTNode * in1, ASTNode * in2, int op);
  virtual ~ASTNodeBool2() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeIf : public ASTNode {
public:
  ASTNodeIf(ASTNode * in1, ASTNode * in2, ASTNode * in3);
  virtual ~ASTNodeIf() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeWhile : public ASTNode {
public:
  ASTNodeWhile(ASTNode * in1, ASTNode * in2);
  virtual ~ASTNodeWhile() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodeBreak : public ASTNode {
public:
  ASTNodeBreak();
  virtual ~ASTNodeBreak() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};

class ASTNodePrint : public ASTNode {
public:
  ASTNodePrint(ASTNode * out_child);
  virtual ~ASTNodePrint() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);
};


class ASTNodeRandom : public ASTNode {
public:
  ASTNodeRandom(ASTNode * in);
  virtual ~ASTNodeRandom() { ; }

  CTableEntry * CompileTubeIC(CSymbolTable & table, ICArray & ica);

};
#endif
