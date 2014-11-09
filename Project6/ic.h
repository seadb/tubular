#ifndef ICH
#define ICH

////////////////////////////////////////////////////////////////////////////////////////////////
//
//  The classes in this file hold information about the intermediate code (IC) and how it is stored
//  in memory before the file is printed out.
//
//  The ICEntry class holds information about a single mInstruction.
//
//  The ICArray class holds an array of ICEntries that make up the full intermediate code program.
//
//  The ICArg_* classes are private within ICEntry and hold information about the arguments
//  associated with each instruction
//    ICArg_Base is the base class that all of the others are derived from.
//    ICArg_VarScalar holds info about scalar variables used as arguments (eg, s27).
//    ICArg_VarConst holds info about literal numbers or chars used as arguments (eg 10 or 'Q').
//    ICArg_VarArray holds info about array variables used as arguments (eg, a5).
//

#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cctype>
#include <cstring>

class ICArray ;

class ICEntry {
private:
  // Variables for converting to assembly

  class ICArg_Base {
  public:
    ICArg_Base() { ; }
    virtual ~ICArg_Base() { ; }

    virtual void AssemblyRead(std::ostream & ofs, int lit, char reg) { }
    virtual void AssemblyWrite(std::ostream & ofs, int lit, char reg) { }

    virtual std::string AsString() = 0;
    virtual std::string AsAssemblyString() = 0;
    virtual std::string GetReg() = 0;
    virtual int GetID() { return -1; }

    virtual bool IsScalar() { return false; }
    virtual bool IsConst() { return false; }
    //virtual bool IsArray() { return false; }
  };

  class ICArg_VarScalar : public ICArg_Base {
  private:
    int mVarID;
    std::string mReg;
  public:
    ICArg_VarScalar(int _id) : mVarID(_id), mReg(""){ ; }
    ~ICArg_VarScalar() { ; }

    void AssemblyRead(std::ostream & ofs, int lit, char reg) {
      mReg = "reg" + As_String(reg);
      ofs << "  load " << lit << " reg" << As_String(reg) << std::endl;
    }
    void AssemblyWrite(std::ostream & ofs, int lit, char reg) {
      ofs << "  store reg" << As_String(reg) << " " << lit << std::endl;
    }

    std::string GetReg() {
      return mReg;
    }

    std::string AsString() {
      std::stringstream out_str;
      out_str << "s" << mVarID;
      return out_str.str();
    }
    std::string AsAssemblyString() {
      return mReg;
    }
    int GetID() { return mVarID; }

    bool IsScalar() { return true; }
  };

  // All constant values: int, char, or label.
  class ICArg_Const : public ICArg_Base {
  private:
    std::string mValue;
  public:
    ICArg_Const(std::string val) : mValue(val) { ; }
    ~ICArg_Const() { ; }

    void AssemblyRead(std::ostream & ofs, int lit, char reg) { }
    void AssemblyWrite(std::ostream & ofs, int lit, char reg) {  }

    std::string AsString() { return mValue; }
    std::string AsAssemblyString() {
      return mValue;
    }
    std::string GetReg() { return mValue; }

    bool IsConst() { return true; }
  };

  // Hint: This is only useful starting in project 5!
  class ICArg_VarArray : public ICArg_Base {
  private:
    int mVarID;
  public:
    ICArg_VarArray(int _id) : mVarID(_id) { ; }
    ~ICArg_VarArray() { ; }

    void AssemblyRead(std::ostream & ofs, int lit, char reg){}
    void AssemblyWrite(std::ostream & ofs, int lit, char reg) {  }

    std::string AsString() {
      std::stringstream out_str;
      out_str << "a" << mVarID;
      return out_str.str();
    }

    std::string AsAssemblyString() {
      return std::string(""); // Place holder!
    }
    std::string GetReg() {
      std::stringstream out_str;
      out_str << "a" << mVarID;
      return out_str.str();
    }
    int GetID() { return mVarID; }

    //virtual bool IsArray() { return true; }
  };

  ICArray * mArray;
  std::string mInst;
  std::string label;
  std::string comment;
  std::vector<ICArg_Base*> args;

// END OF PRIVATE ICEntry

public:
  ICEntry(std::string inInst, std::string in_label, ICArray * array) : mInst(inInst), label(in_label), mArray(array) { ; }
  ~ICEntry() { ; }

  const std::string & GetInstName() const { return mInst; }
  const std::string & GetLabel() const { return label; }
  const std::string & GetComment() const { return comment; }
  unsigned int GetNumArgs() const { return args.size(); }

  void AddArrayArg(int id)         { args.push_back(new ICArg_VarArray(id)); }
  void AddConstArg(std::string id) { args.push_back(new ICArg_Const(id)); }
  void AddScalarArg(int id)        { args.push_back(new ICArg_VarScalar(id)); }

  void SetLabel(std::string in_lab) { label = in_lab; }
  void SetComment(std::string cmt) { comment = cmt; }

  void PrintIC(std::ostream & ofs);
};

//END OF ICEntry

class ICArray {
private:
  std::vector <ICEntry*> mICArray;
  std::map<std::string, int> mMemoryMap;
  int mMemPosition;
  // There are three types of argument requirements for instructions:
  // * VALUE  - This arg is an input value: literal numbers or chars, scalars, labels, etc.
  // * SCALAR - This arg is an output scalar variable that gets written to.
  // * ARRAY  - This arg is an array that gets somehow manipulated

  struct ArgType {
    enum type { NONE=0, VALUE, SCALAR, ARRAY };
  };

  std::map<std::string, std::vector<ArgType::type> > mArgTypeMap;

  // Helper method to identify types of arguments expected with each nstruction
  void SetupArgs(std::string inst_name, ArgType::type type1, ArgType::type type2, ArgType::type type3) {
    std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
    arg_types.push_back(type1);
    arg_types.push_back(type2);
    arg_types.push_back(type3);
  }

  // Helper methods to add arguments to mInstructions, while verifying their types.
  void AddArg(ICEntry * entry, int in_arg, ArgType::type expected_type);
  void AddArg(ICEntry * entry, const std::string & in_arg, ArgType::type expected_type);

public:
  int static_memory_size;
  ICArray() : mMemPosition(1) {
    // Fill out the arg types for each mInstruction
    SetupArgs("val_copy",    ArgType::VALUE,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("add",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("sub",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("mult",        ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("div",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("mod",         ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_less",   ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_gtr",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_equ",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_nequ",   ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_lte",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("test_gte",    ArgType::VALUE,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("jump",        ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("jump_if_0",   ArgType::VALUE,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("jump_if_n0",  ArgType::VALUE,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("random",      ArgType::VALUE,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("out_int",     ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("out_char",    ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("nop",         ArgType::NONE,   ArgType::NONE,   ArgType::NONE);
    SetupArgs("push",        ArgType::VALUE,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("pop",         ArgType::SCALAR, ArgType::NONE,   ArgType::NONE);
    SetupArgs("ar_get_idx",  ArgType::ARRAY,  ArgType::VALUE,  ArgType::SCALAR);
    SetupArgs("ar_set_idx",  ArgType::ARRAY,  ArgType::VALUE,  ArgType::VALUE);
    SetupArgs("ar_get_size", ArgType::ARRAY,  ArgType::SCALAR, ArgType::NONE);
    SetupArgs("ar_set_size", ArgType::ARRAY,  ArgType::VALUE,  ArgType::NONE);
    SetupArgs("ar_copy",     ArgType::ARRAY,  ArgType::ARRAY,  ArgType::NONE);
    SetupArgs("ar_push",     ArgType::ARRAY,  ArgType::NONE,   ArgType::NONE);
    SetupArgs("ar_pop",      ArgType::ARRAY,  ArgType::NONE,   ArgType::NONE);
  }
  ~ICArray() { ; }

  ICEntry& AddLabel(std::string label_id, std::string cmt="");

  void AddEntry(std::string key){
    mMemoryMap[key] = mMemPosition;
    mMemPosition = mMemPosition + 1;
  }

  int SearchMemMap(std::string key){
    if(mMemoryMap.find(key) != mMemoryMap.end()){
      return mMemoryMap[key];
    }
    else return -1;
  }
  // All forms of Add() method.
  // Arguments can either be variables (where an int represents the variable ID) or
  // constant values (where a string holds the constant's lexeme).  And 'a' or 's' will
  // automatically be prepended to an int for a variable based on the mInstruction used.
  ICEntry& Add(std::string mInst, int arg1=-1, int arg2=-1, int arg3=-1,
                                                    std::string cmt="");

  ICEntry& Add(std::string mInst, std::string arg1, int arg2=-1, int arg3=-1,
                                                         std::string cmt="");

  ICEntry& Add(std::string mInst, int arg1, std::string arg2, int arg3=-1,
                                                      std::string cmt="");

  ICEntry& Add(std::string mInst, std::string arg1, std::string arg2,
                                    int arg3=-1, std::string cmt="");

  ICEntry& Add(std::string mInst, int arg1, int arg2, std::string arg3,
                                                   std::string cmt="");

  ICEntry& Add(std::string mInst, std::string arg1, int arg2, std::string arg3,
                                                           std::string cmt="");

  ICEntry& Add(std::string mInst, int arg1, std::string arg2, std::string arg3,
                                                           std::string cmt="");
  ICEntry& Add(std::string mInst, std::string arg1, std::string arg2,
                               std::string arg3, std::string cmt="");

  void PrintIC(std::ostream & ofs);
};

#endif
