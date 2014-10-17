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
  std::string lhs_type;
  std::string rhs_type;

public:
  ASTNode(int in_line_num=-1) : line_num(in_line_num) { ; }
  void yyerr(std::string err_string) {
  	std::cout << "ERROR(line " << line_num << "): " << err_string << std::endl;
  	exit(1);
  }
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
  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) { 
    return var_entry; 
  }

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
  std::string type;
public:
  ASTNode_Literal(std::string in_lex, std::string in_type) : lexeme(in_lex), type(in_type) { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * out_var = tables.AddTempEntry();
    out << "val_copy " << lexeme << " s" << out_var->GetVarID() << std::endl;
    out_var->SetType(type);
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
  ASTNode_Assign(ASTNode * lhs, ASTNode * rhs, int  in_line) {
    line_num = in_line;
    children.push_back(lhs);
    children.push_back(rhs);
  }

  ~ASTNode_Assign() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * lhs_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * rhs_var = children[1]->CompileTubeIC(tables, out);
    lhs_type = lhs_var->GetType();
    rhs_type = rhs_var->GetType();
    if (lhs_type != rhs_type) {
	std::string e;
	e += "types do not match for assignment (lhs='";
	e += lhs_type; 
	e += "', rhs='";
	e += rhs_type;
	e += "')";
	yyerr(e);
    }
    out << "val_copy s" <<  rhs_var->GetVarID() << " s" << lhs_var->GetVarID() << std::endl;
    lhs_var->SetType(lhs_type);
    return lhs_var;
    }

  std::string GetName() { return "ASTNode_Assign (operator=)"; }
};

class ASTNode_MathAssign : public ASTNode {
protected:
  int math_op;
public:
  ASTNode_MathAssign(ASTNode * lhs, ASTNode * rhs, int op, int in_line) {
    children.push_back(lhs);
    children.push_back(rhs);
    math_op = op;
    line_num = in_line;
  }

  ~ASTNode_MathAssign() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * lhs_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * rhs_var = children[1]->CompileTubeIC(tables, out);
    lhs_type = lhs_var->GetType();
    rhs_type = rhs_var->GetType();
    if (lhs_type == "char") {
	std::string e;
	e += "cannot use type '";
	e += lhs_type;
	e += "' in mathematical expressions";
	yyerr(e);
    } else if (rhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += rhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
    }
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

    lhs_var->SetType("int");

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
  ASTNode_Math2(ASTNode * in1, ASTNode * in2, int op, int in_line) : math_op(op) {
    children.push_back(in1);
    children.push_back(in2);
    line_num = in_line;
  }
  ~ASTNode_Math2() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();
    lhs_type = in_var1->GetType();
    rhs_type = in_var2->GetType();
    if (lhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += lhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
    } else if (rhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += rhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
    }
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
    out_var->SetType("int");
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
  ASTNode_Negation(ASTNode * in, int in_line) {
    children.push_back(in);
    line_num = in_line;
  }
  ~ASTNode_Negation() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();
    lhs_type = in_var->GetType();
    if (lhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += lhs_type;
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
    return "ASTNode_Negation";
  }
};

class ASTNode_Not : public ASTNode {
public:
  ASTNode_Not(ASTNode * in, int in_line) {
    children.push_back(in);
    line_num = in_line;
  }
  ~ASTNode_Not() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();
    lhs_type = in_var->GetType();
    if (lhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += lhs_type;
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
    return "ASTNode_Not";
  }
};

class ASTNode_Comparison : public ASTNode {
protected:
  std::string comp_op;
public:
  ASTNode_Comparison(ASTNode * in1, ASTNode * in2, std::string op, int in_line) : comp_op(op) {
    children.push_back(in1);
    children.push_back(in2);
    line_num = in_line;
  }
  ~ASTNode_Comparison() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    lhs_type = in_var1->GetType();
    rhs_type = in_var2->GetType();
    if (lhs_type != rhs_type) {
	std::string e;
	e += "types do not match for relationship operator (lhs = '";
	e += lhs_type;
	e += "', rhs = '";
	e += rhs_type;
	e += "')";
	yyerr(e);
    }
    out_var->SetType("int");
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
  ASTNode_Logical(ASTNode * in1, ASTNode * in2, std::string op, int in_line) : log_op(op) {
    children.push_back(in1);
    children.push_back(in2);
    line_num = in_line;
  }
  ~ASTNode_Logical() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();
    lhs_type = in_var1->GetType();
    if (lhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += lhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
    }
    const int i1 = in_var1->GetVarID();
    const int o3 = out_var->GetVarID();

    // Determine the correct operation...
    if (log_op == "&&") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_0 s" << o3 << " and" << o3 << std::endl;
      tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
      rhs_type = in_var2->GetType();
      if (rhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += rhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
      }
      //std::cout << "Logical type 2: " << rhs_type << std::endl;
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "and" << o3 << ":" << std::endl;
    } else if (log_op == "||") {
      out << "test_nequ s" << i1 << " 0 s" << o3 << std::endl;
      out << "jump_if_n0 s" << o3 << " or" << o3 << std::endl;
      tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
      rhs_type = in_var2->GetType();
      if (rhs_type == "char") {
        std::string e;
        e += "cannot use type '";
        e += rhs_type;
        e += "' in mathematical expressions";
        yyerr(e);
      }
      const int i2 = in_var2->GetVarID();
      out << "test_nequ s" << i2 << " 0 s" << o3 << std::endl;
      out << "or" << o3 << ":" << std::endl;
    }
    else {
      std::cerr << "INTERNAL COMPILER ERROR: Unknown Logical operator '"
                << log_op << "'" << std::endl;
    }
    out_var->SetType(in_var1->GetType());

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
  ASTNode_Conditional(ASTNode * in1, ASTNode * in2, ASTNode * in3, int in_line) {
    children.push_back(in1);
    children.push_back(in2);
    children.push_back(in3);
    line_num = in_line;
  }
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

    out_var->SetType("int");

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_If : public ASTNode {
public:
  ASTNode_If(ASTNode * in1, ASTNode * in2, int in_line) {
    children.push_back(in1);
    children.push_back(in2);
    line_num = in_line;
  }
  ~ASTNode_If() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    lhs_type = in_var1->GetType();
    if (lhs_type != "int") {
	std::string e;
	e += "condition for if statements must evaluate to type int";
	yyerr(e);
    }
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

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_While : public ASTNode {
public:
  ASTNode_While(ASTNode * in1, ASTNode * in2, int in_line) {
    children.push_back(in1);
    children.push_back(in2);
    line_num = in_line;
  }
  ~ASTNode_While() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * out_var = tables.AddTempEntry();
    const int o4 = out_var->GetVarID();

    out << "while_begin" << o4 << ":" << endl;

    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    lhs_type = in_var1->GetType();
    if (lhs_type != "int") {
	std::string e;
	e += "condition for while statements must evaluate to type int";
	yyerr(e);
    }
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

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_Break : public ASTNode {
public:
  ASTNode_Break(int in_line) {
    line_num = in_line;
  }
  ~ASTNode_Break() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    if(tables.breaks.size() > 0) {
      out << "jump " << tables.breaks.back() << endl;
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
    return "ASTNode_Conditional";
  }
};

class ASTNode_Print : public ASTNode {
public:
  ASTNode_Print() { ; }
  virtual ~ASTNode_Print() { ; }

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
  ASTNode_Random(ASTNode * in, int in_line) {
    children.push_back(in);
    line_num = in_line;
  }

  ~ASTNode_Random() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();
    lhs_type = in_var->GetType();
    if (lhs_type != "int") {
	std::string e;
	e += "cannot use type '";
	e += lhs_type;
	e += "' as an argument to random";
	yyerr(e);
    }
    out << "random s" <<  in_var->GetVarID() << " s" << out_var->GetVarID() << std::endl;

    out_var->SetType(in_var->GetType());

    return out_var;
  }
};

class ASTNode_Else : public ASTNode {
public:
  ASTNode_Else(ASTNode * in1, ASTNode * in2, ASTNode * in3, int in_line) {
    children.push_back(in1);
    children.push_back(in2);
    children.push_back(in3);
    line_num = in_line;
  }
  ~ASTNode_Else() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * in_var1 = children[0]->CompileTubeIC(tables, out);
    tableEntry * out_var = tables.AddTempEntry();

    const int i1 = in_var1->GetVarID();
    const int o4 = out_var->GetVarID();

    out << "test_nequ s" << i1 << " 0 s" << o4 << std::endl;
    out << "jump_if_n0 s" << o4 << " if_true" << o4 << std::endl;

    // False
    out << "jump else_label" << o4 << std::endl;

    // True
    out << "if_true" << o4 << ":" << std::endl;
    tableEntry * in_var2 = children[1]->CompileTubeIC(tables, out);
    out << "jump end_label" << o4 << std::endl;

    //Else
    out << "else_label" << o4 << ":" << std::endl;
    tableEntry * in_var3 = children[2]->CompileTubeIC(tables, out);

    out << "end_label" << o4 << ":" << std::endl;

    out_var->SetType(in_var1->GetType());

    return out_var;
  }

  std::string GetName() {
    return "ASTNode_Conditional";
  }
};

class ASTNode_Blank : public ASTNode {
public:
  ASTNode_Blank() { ; }

  tableEntry * CompileTubeIC(symbolTables & tables, std::ostream & out) {
    tableEntry * out_var = tables.AddTempEntry();
    return out_var;
  }

  std::string GetName() {
    std::string out_string = "ASTNode_Blank";
    return out_string;
  }

};

#endif
