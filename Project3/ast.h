#ifndef AST_H
#define AST_H

#include <fstream>
#include <string>
#include <vector>

#include "symbol_table.h"

// The classes in this file hold info about the nodes that form the Abstract Syntax Tree (AST)
//
// ASTNode : The base class for all of the others, with useful virtual functions.
//
// ASTNode_Block : Blocks of statements, including the overall program.
// ASTNode_Variable : Leaf node containing a variable.
// ASTNode_Literal : Leaf node contiaing a literal value.
// ASTNode_Assign : Assignments
// ASTNode_Math2 : Two-input math operations (here, just '+' and '-')
// ASTNode_Compare : Compare two expressions
class ASTNode {
protected:
  int line_num;                     // What line of the source program generated this node?
  std::vector<ASTNode *> children;  // What sub-trees does this node have?

public:
  ASTNode(int in_line_num=-1) : line_num(in_line_num) { ; }
  virtual ~ASTNode() {
    for (int i = 0; i < (int) children.size(); i++) {
      if (children[i] != NULL) delete children[i];
    }
  }

  // Accessors
  int GetLineNum() const { return line_num; }
  ASTNode * GetChild(int id) { return children[id]; }
  unsigned int GetNumChildren() const { return children.size(); }

  void SetLineNum(int _in) { line_num = _in; }
  ASTNode * RemoveChild(int id) {
    ASTNode * out_child = children[id];
    children[id] = NULL;
    return out_child;
  }

  // Add a new child to this node, at the end of the vector.
  void AddChild(ASTNode * in_child) { children.push_back(in_child); }

  // Convert a single node to TubeIC and return information about the
  // variable where the results are saved.  Call children recursively.
  virtual tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) = 0;

  // Return the name of the node being called.  This function is useful for debbing the AST.
  virtual std::string GetName() { return "ASTNode (base class)"; }

  // Debug function to help make sure the AST is being built correctly...
  void DebugPrint(int depth=0) {
    for (int i = 0; i < depth; i++) std::cout << " ";  // Indent appropriately
    std::cout << GetName() << std::endl;               // Print this node's name
    for (int i = 0; i < (int) children.size(); i++) {  // Print all of the children
      children[i]->DebugPrint(depth+1);
    }
  }
};


// A temporary node used to transfer children...
class ASTNode_Temp : public ASTNode {
public:
  ASTNode_Temp() { ; }
  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    std::cerr << "Internal Compiler Error: Trying to run CompileTubeIC on a temporary node!!" << std::endl;
    return NULL;
  }

  std::string GetName() { return "ASTNode_Temp (temporary container class)"; }
};

// Block...
class ASTNode_Block : public ASTNode {
public:
  ASTNode_Block() { ; }
  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    // Compile the code for each sub-tree in this block
    for (int i = 0; i < (int) children.size(); i++) {
      children[i]->CompileTubeIC(table, out);
    }
    return NULL;
  }

  std::string GetName() { return "ASTNode_Block (container class)"; }
};

// Leaves...
class ASTNode_Variable : public ASTNode {
private:
  tableEntry * var_entry;
public:
  ASTNode_Variable(tableEntry * in_entry) : var_entry(in_entry) {;}

  tableEntry * GetVarEntry() { return var_entry; }
  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) { return var_entry; }

  std::string GetName() {
    std::string out_string = "ASTNode_Variable (";
    out_string += var_entry->GetName();
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Literal : public ASTNode {
private:
  std::string lexeme;
public:
  ASTNode_Literal(std::string in_lex) : lexeme(in_lex) { ; }

  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    tableEntry * out_var = table.AddTempEntry();
    out << "val_copy " << lexeme << " s" << out_var->GetVarID() << std::endl;
    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Literal (";
    out_string += lexeme;
    out_string += ")";
    return out_string;
  }
};

// Math...

class ASTNode_Assign : public ASTNode {
public:
  ASTNode_Assign(ASTNode * lhs, ASTNode * rhs) {
    children.push_back(lhs);
    children.push_back(rhs);
  }

  ~ASTNode_Assign() { ; }

  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    tableEntry * lhs_var = children[0]->CompileTubeIC(table, out);
    tableEntry * rhs_var = children[1]->CompileTubeIC(table, out);

    out << "val_copy s" <<  rhs_var->GetVarID() << " s" << lhs_var->GetVarID() << std::endl;

    return lhs_var;
  }

  std::string GetName() { return "ASTNode_Assign (operator=)"; }
};

class ASTNode_Math2 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op) : math_op(op) {
    children.push_back(in1);
    children.push_back(in2);
  }
  ~ASTNode_Math2() { ; }

  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(table, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(table, out);
    tableEntry * out_var = table.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    switch( math_op ){
      case '+':
        out << "add s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '-':
        out << "sub s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '*':
        out << "mult s" << i1 << " s" << i2 << " s" << o3 << std::endl;
        break;
      case '/':
        out << "div s" << i1 << " s" << i2 << " s" << o3 << std::endl;
        break;
      default:
        std::cerr << "INTERNAL COMPILER ERROR: Unknown Math2 operator '"
                << math_op << "'" << std::endl;
    }

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Math2 (operator";
    out_string += (char) math_op;
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Print : public ASTNode {
public:
  ASTNode_Print() { ; }
  virtual ~ASTNode_Print() { ; }

  virtual tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out)
  {
    for (int i = 0; i < (int) children.size(); i++) {
      tableEntry * in_var = children[i]->CompileTubeIC(table, out);
      out << "out_int s" << in_var->GetVarID() << std::endl;
    }
    out << "out_char '\\n'" << std::endl;

    return NULL;
  }
};


class ASTNode_Random : public ASTNode {
public:
  ASTNode_Random() { ; }
  virtual ~ASTNode_Random() { ; }

  virtual tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out)
  {
    for (int i = 0; i < (int) children.size(); i++) {
      tableEntry * in_var = children[i]->CompileTubeIC(table, out);
      out << "out_int s" << in_var->GetVarID() << std::endl;
    }
    out << "out_char '\\n'" << std::endl;

    return NULL;
  }
};


class ASTNode_Compare : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Compare(ASTNode * in1, ASTNode * in2, int op) : math_op(op) {
    children.push_back(in1);
    children.push_back(in2);
  }
  ~ASTNode_Compare() { ; }

  tableEntry * CompileTubeIC(symbolTable & table, std::ofstream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(table, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(table, out);
    tableEntry * out_var = table.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    switch( math_op ){
      case '>':
        out << "test_gtr s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '<':
        out << "test_less s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
        break;
      case '!=':
        out << "test_nequ s" << i1 << " s" << i2 << " s" << o3 << std::endl;
        break;
      case '==':
        out << "test_equ s" << i1 << " s" << i2 << " s" << o3 << std::endl;
        break;
      case '>=':
        out << "test_gte s" << i1 << " s" << i2 << " s" << o3 << std::endl;
        break;
      case '<=':
        out << "test_lte s" << i1 << " s" << i2 << " s" << o3 ;
        break;
      default:
        std::cerr << "INTERNAL COMPILER ERROR: Unknown Math2 operator '"
                << math_op << "'" << std::endl;
    }

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Math2 (operator";
    out_string += (char) math_op;
    out_string += ")";
    return out_string;
  }
};


#endif
