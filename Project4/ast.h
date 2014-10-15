#ifndef AST_H
#define AST_H

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#include "symbol_table.h"

// The classes in this file hold info about the nodes that form the Abstract Syntax Tree (AST)
//
// ASTNode : The base class for all of the others, with useful virtual functions.
//
// ASTNode_Block : Blocks of statements, including the overall program.
// ASTNode_Variable : Leaf node containing a variable.
// ASTNode_Literal : Leaf node contiaing a literal value.
// ASTNode_Assign : Assignements
// ASTNode_Math2 : Two-input math operations (here, just '+' and '-')

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
  virtual std::string GetType() = 0;
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
  virtual tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) = 0;

  // Return the name of the node being called.  This function is useful for debbing the AST.
  virtual std::string GetName() { return "ASTNode (base class)"; }

  // Debug function to help make sure the AST is being built correctly...
  void DebugPrint(int depth=0) {
    for (int i = 0; i < depth; i++) std::cout << ".";  // Indent appropriately
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
  std::string GetType() { ; }
  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    std::cerr << "Internal Compiler Error: Trying to run CompileTubeIC on a temporary node!!" << std::endl;
    return NULL;
  }

  std::string GetName() { return "ASTNode_Temp (temporary container class)"; }
};

// Block...
class ASTNode_Block : public ASTNode {
public:
  ASTNode_Block() { ; }
  std::string GetType() { ; }
  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    // Compile the code for each sub-tree in this block
    for (int i = 0; i < (int) children.size(); i++) {
      children[i]->CompileTubeIC(tables, out);
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
  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) { return var_entry; }

  std::string GetName() {
    std::string out_string = "ASTNode_Variable (";
    out_string += var_entry->GetName();
    out_string += ")";
    return out_string;
  }

  std::string GetType() {
    return var_entry->GetType();
  }
};

class ASTNode_Literal : public ASTNode {
private:
  std::string lexeme;
  std::string type;
public:
  ASTNode_Literal(std::string in_lex, std::string in_type) : lexeme(in_lex), type(in_type) { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * out_var = tables.AddTempEntry();
    out << "val_copy " << lexeme << " s" << out_var->GetVarID() << std::endl;
    return out_var;
  }

  std::string GetType() {
	return type; 
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

  std::string GetType() { ; }
  ~ASTNode_Assign() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * lhs_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * rhs_var = children[1]->CompileTubeIC(tables, out);

    	out << "val_copy s" <<  rhs_var->GetVarID() << " s" << lhs_var->GetVarID() << std::endl;
    	return lhs_var;
    }

  std::string GetName() { return "ASTNode_Assign (operator=)"; }
};

class ASTNode_MathAssign : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_MathAssign(ASTNode * lhs, ASTNode * rhs, int op) {
    children.push_back(lhs);
    children.push_back(rhs);
    math_op = op;
  }
  std::string GetType() { ; }

  ~ASTNode_MathAssign() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * lhs_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * rhs_var = children[1]->CompileTubeIC(tables, out);

    const int l = lhs_var->GetVarID();
    const int r = rhs_var->GetVarID();

    // Determine the correct operation...
    if (math_op == '+') {
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
    }

    return lhs_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_MathAssign (operator";
    out_string += (char) math_op;
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Math2 : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op) : math_op(op) {
    children.push_back(in1);
    children.push_back(in2);
  }
  std::string GetType() { ; }
  ~ASTNode_Math2() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();

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

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Math2 (operator";
    out_string += (char) math_op;
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Negation : public ASTNode {
public:
  ASTNode_Negation(ASTNode * in) {
    children.push_back(in);
  }
  std::string GetType() { ; }
  ~ASTNode_Negation() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i = in_var->GetVarID();
    const int o = out_var->GetVarID();

    out << "val_copy -1 s" << o << std::endl;
    out << "mult s" << o << " s" << i << " s" << o << std::endl;

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Negation";
  }
};

class ASTNode_Not : public ASTNode {
public:
  ASTNode_Not(ASTNode * in) {
    children.push_back(in);
  }
  std::string GetType() { ; }
  ~ASTNode_Not() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i = in_var->GetVarID();
    const int o = out_var->GetVarID();

    out << "test_equ s" << i << " 0 s" << o << std::endl;

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Not";
  }
};

class ASTNode_Comparison : public ASTNode {
protected:
  std::string comp_op;
public:
  ASTNode_Comparison(ASTNode * in1, ASTNode * in2, std::string op) : comp_op(op) {
    children.push_back(in1);
    children.push_back(in2);
  }
  std::string GetType() { ; }
  ~ASTNode_Comparison() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int i2 = in_var2->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    if (comp_op == "<") {
      out << "test_less s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (comp_op == ">") {
      out << "test_gtr s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (comp_op == "<=") {
      out << "test_lte s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (comp_op == ">=") {
      out << "test_gte s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (comp_op == "==") {
      out << "test_equ s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    } else if (comp_op == "!=") {
      out << "test_nequ s" << i1 << " s" <<  i2 << " s" << o3 << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Comparison operator '"
                << comp_op << "'" << std::endl;
    }

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Comparison (operator";
    out_string += comp_op;
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Logical : public ASTNode {
protected:
  std::string log_op;
public:
  ASTNode_Logical(ASTNode * in1, ASTNode * in2, std::string op) : log_op(op) {
    children.push_back(in1);
    children.push_back(in2);
  }
  std::string GetType() { ; }
  ~ASTNode_Logical() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

// Added tableEntry in_var2 and const int i2 into the code below so short circuiting works

    const int i1 = in_var1->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    if (log_op == "&&") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_0 s" << o3 << " and" << o3 << std::endl;
      tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "and" << o3 << ":" << std::endl;
    } else if (log_op == "||") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_n0 s" << o3 << " or" << o3 << std::endl;
      tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "or" << o3 << ":" << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Logical operator '"
                << log_op << "'" << std::endl;
    }

    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Logical (operator";
    out_string += log_op;
    out_string += ")";
    return out_string;
  }
};

class ASTNode_Conditional : public ASTNode {
public:
  ASTNode_Conditional(ASTNode * in1, ASTNode * in2, ASTNode * in3) {
    children.push_back(in1);
    children.push_back(in2);
    children.push_back(in3);
  }
  std::string GetType() { ; }
  ~ASTNode_Conditional() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " cond_true" << o4 << std::endl;

    // False
    tableEntry * in_var3 = children[2]->CompileTubeIC(tables, out);
    const int i3 = in_var3->GetVarID();
    out << "val_copy s" << i3 << " s" << o4 << std::endl;
    out << "jump cond_end" << o4 << std::endl;

    // True
    out << "cond_true" << o4 << ":" << std::endl;
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    const int i2 = in_var2->GetVarID();
    out << "val_copy s" << i2 << " s" << o4 << std::endl;

    out << "cond_end" << o4 << ":" << std::endl;

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_If : public ASTNode {
public:
  ASTNode_If(ASTNode * in1, ASTNode * in2) {
    children.push_back(in1);
    children.push_back(in2);
  }
  std::string GetType() { ; }
  ~ASTNode_If() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " if_true" << o4 << std::endl;

    // False
    out << "jump if_end" << o4 << std::endl;

    // True
    out << "if_true" << o4 << ":" << std::endl;
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);

    out << "if_end" << o4 << ":" << std::endl;

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_While : public ASTNode {
public:
  ASTNode_While(ASTNode * in1, ASTNode * in2) {
    children.push_back(in1);
    children.push_back(in2);
  }
  std::string GetType() { ; }
  ~ASTNode_While() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * out_var = tables.AddTempEntry();
    const int o4 = out_var->GetVarID();

    out << "while_begin" << o4 << ":" << endl;

    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    const int i1 = in_var1->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_0 s" << o4 << " while_end" << o4 << std::endl;

    stringstream ss;
    ss << "while_end" << o4;
    tables.breaks.push_back(ss.str());

    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);

    if(tables.breaks.size() > 0)
    tables.breaks.pop_back();

    out << "jump while_begin" << o4 << endl;

    out << "while_end" << o4 << ":" << std::endl;

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_Break : public ASTNode {
public:
  ASTNode_Break() {
  }
  std::string GetType() { ; }
  ~ASTNode_Break() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    if(tables.breaks.size() > 0) {
      out << "jump " << tables.breaks.back() << endl;
      //breaks.pop_back();
    }
    else {
      cerr << "INTERNAL COMPILER ERROR: bad usage of break" << endl;
    }

    return NULL;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_Print : public ASTNode {
public:
  ASTNode_Print() { ; }
  virtual ~ASTNode_Print() { ; }
  std::string GetType() { ; }

  virtual tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out)
  {
    for (int i = 0; i < (int) children.size(); i++) {
      tableEntry * in_var = children[i]->CompileTubeIC(tables, out);
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

class ASTNode_Random : public ASTNode {
public:
  ASTNode_Random(ASTNode * in) {
    children.push_back(in);
  }
  std::string GetType() { ; }

  ~ASTNode_Random() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    out << "random s" <<  in_var->GetVarID() << " s" << out_var->GetVarID() << std::endl;

    return out_var;
  }
};

#endif
