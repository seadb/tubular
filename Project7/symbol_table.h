#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

////////////////////////////////////////////////////////////////////////////////////////////////
//
// This file contains all of the information about the symbol table.
//
// CSymbolTable : interacted with by the rest of the code to look up information about variables,
//               labels, and (eventually) functions.
//
// CTableEntry : all of the stored information about a single variable.
//

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "type_info.h"

class ASTNode;
class CSymbolTable;

class CTableEntry {
  friend class CSymbolTable;
protected:
  int mTypeID;       // What is the type of this variable?
  std::string mName;  // Variable name used by sourcecode.
  int mScope;         // What scope was this variable declared at?
  bool mIsTemp;      // Is this variable just temporary (internal to compiler)
  int mVarID;        // What is the intermediate code ID for this variable?
  CTableEntry * mNext; // A pointer to another entry that this one is shadowing
  CTableEntry * mIndex; // Is there an index associated with this var
  CTableEntry * mArray; // Is there an array?
  int mSize;
  bool mNegative;
  std::string mContent;

  CTableEntry(int inType)
    : mTypeID (inType)
    , mName("__TEMP__")
    , mScope(-1)
    , mIsTemp(true)
    , mVarID(-1)
    , mNext(NULL)
    , mIndex(NULL)
    , mArray(NULL)
    , mSize(0)
    , mNegative(false)
    , mContent("")
  {
  }

  CTableEntry(int inType, const std::string inName)
    : mTypeID(inType)
    , mName(inName)
    , mScope(-1)
    , mIsTemp(false)
    , mVarID(-1)
    , mNext(NULL)
    , mIndex(NULL)
    , mArray(NULL)
    , mSize(0)
    , mNegative(false)
    , mContent("")
  {
  }
  virtual ~CTableEntry() { ; }

public:
  int GetType()            const { return mTypeID; }
  std::string GetName()    const { return mName; }
  int GetScope()           const { return mScope; }
  bool GetTemp()           const { return mIsTemp; }
  int GetVarID()           const { return mVarID; }
  int GetSize()            const { return mSize; }
  bool GetNegative()       const { return mNegative; }
  CTableEntry * GetIndex() const { return mIndex;}
  CTableEntry * GetNext()  const { return mNext; }
  CTableEntry * GetArray() const { return mArray; }
  std::string GetContent() const { return mContent; }


  void SetName(std::string inName)     { mName = inName; }
  void SetScope(int inScope)           { mScope = inScope; }
  void SetVarID(int inID)              { mVarID = inID; }
  void SetSize(int inSize)             { mSize = inSize; }
  void SetNegative(bool inNegative)        { mNegative = inNegative; }
  void SetNext(CTableEntry * inNext)   { mNext = inNext; }
  void SetIndex(CTableEntry * inIndex) { mIndex = inIndex; }
  void SetArray(CTableEntry * inArray ) { mArray = inArray;  }
  void SetContent(std::string inContent) { mContent = inContent; }
};

//END OF TABLE ENTRY

class CFunctionEntry {
 friend class CSymbolTable;
protected:
  int mReturnType;        // Type of variable that is returned
  std::string mName;      // Function name used by sourcecode.
  std::string mLabel;
  //CTableEntry * mDeclare;
  int mVarID;           // What is the intermediate code ID for this variable?
  CFunctionEntry * mNext; // A pointer to another entry that this one is shadowing
  std::vector <CTableEntry * > mArgs; // <name, type of the arg>
  CTableEntry * mReturn;             //
  CTableEntry * mReturnValue;             //
  ASTNode * mBody;                   // the body of the function
  //int mSize;                       // The size of the array
  //bool mIsTemp;      // Is this variable just temporary (internal to compiler)
  //bool mNegative;                  // Is the variable negative
  //std::string mContent;            // What is the variable equal to?

  CFunctionEntry()
    : mReturnType (0)
    , mName("__TEMP__")
    // mDeclare(NULL)
    , mVarID(-1)
    , mNext(NULL)
    , mArgs(std::vector<CTableEntry *>())
    , mBody(NULL)
    , mReturn(NULL)
    //, mSize(0)
    //, mIsTemp(true)
    //, mNegative(false)
    { ; }

  CFunctionEntry(std::string inName, int returnType )
    : mReturnType(returnType)
    , mVarID(-1)
    , mNext(NULL)
    , mArgs(std::vector<CTableEntry *>())
    //, mArgs(std::map<std::string,int>())
    , mBody(NULL)
    , mReturn(NULL)
    //, mSize(0)
    //, mIsTemp(false)
    //, mNegative(false)
    { SetName(inName); }
  virtual ~CFunctionEntry() { ; }

public:
  int GetReturnType()      const { return mReturnType; }
  std::string GetName()    const { return mName; }
  std::string GetLabel()    const { return mLabel; }
  int GetVarID()           const { return mVarID; }
  CFunctionEntry * GetNext()  const { return mNext; }
  CTableEntry * GetArg(int position) const { return mArgs[position]; }
  ASTNode * GetBody()        const { return mBody; }
  int GetNumArgs()           const { return mArgs.size();}
  CTableEntry * GetReturn() const { return mReturn; }
  CTableEntry * GetReturnValue() const { return mReturnValue; }
  //bool GetTemp()           const { return mIsTemp; }
  //int GetSize()            const { return mSize; }
  //bool GetNegative()       const { return mNegative;}
  /*
  int GetArgType(std::string name) {
    std::map<std::string, int>::iterator it = mArgs.find(name);
    if(it != mArgs.end())
    {
      return mArgs[name];
    }
    std::cerr << "ERROR: " << name << " is not an argument" << std::endl;
  }*/

  void SetReturnType(int inType)         { mReturnType = inType; }
  void SetName(std::string inName)
  {
    mName = inName;
    mLabel = std::string("function_");
    mLabel += inName;
  }
  void SetVarID(int inID)                { mVarID = inID; }
  void SetNext(CFunctionEntry * inNext)  { mNext = inNext; }
  void PushArg(CTableEntry * entry)      { mArgs.push_back(entry); }
  void SetBody(ASTNode * body)           { mBody = body; }
  void SetReturn(CTableEntry * entry)    { mReturn = entry; }
  void SetReturnValue(CTableEntry * entry)    { mReturnValue = entry; }
  //void SetSize(int inSize)             { mSize = inSize; }
  //void SetNegative(bool inNegative)    { mNegative = inNegative; }
};


class CSymbolTable {

private:
  // A map of active variables
  std::map<std::string, CTableEntry *> mTableMap;
  // A map of defined functions
  std::map<std::string, CFunctionEntry *> mFunctionMap;

  // Variables declared in each scope
  std::vector<std::vector<CTableEntry *> *> mScopeInfo;
  // Variables that are out of scope
  std::vector<CTableEntry *> mVarArchive;
  CFunctionEntry * mCurrentFunction;
  int mCurrentScope;                                // Current mScope level
  int mNextVarID;                                   // Next variable ID to use.
  int mNextLabelID;                                 // Next label ID to use.
  std::vector<std::string> mWhileEndStack;  // End labels of active while commands
  bool mFunctionMode;

  // Figure out the next memory position to use.
  int GetNextID() { return mNextVarID++; }

public:
  CSymbolTable()
    : mCurrentScope(0), mNextVarID(1), mNextLabelID(0), mCurrentFunction(NULL),
  mFunctionMode(false){
      mScopeInfo.push_back(new std::vector<CTableEntry *>);
  }

  ~CSymbolTable()
  {
    // Clean up all variable entries
    while (mCurrentScope >= 0) DecScope();
    for (int i = 0; i < (int) mVarArchive.size(); i++) delete mVarArchive[i];
  }

  int GetSize()     const { return (int) mTableMap.size(); }
  int GetCurScope() const { return mCurrentScope; }
  const std::vector<CTableEntry *> & GetScopeVars(int scope)
  {
    if (scope < 0 || scope >= (int) mScopeInfo.size()) {
      std::cerr << "Internal Compiler Error: Requesting vars from scope #"
        << scope << ", but only " << mScopeInfo.size() << " scopes exist."
        << std::endl;
    }
    return *(mScopeInfo[scope]);
  }

  //std::vector<CTableEntry *> GetVars() const { return mScopeInfo[mCurrentScope];}

  void SetMode(bool inMode) { mFunctionMode = inMode; }
  void SetCurrentFunction(CFunctionEntry * entry) {mCurrentFunction = entry; }
  CFunctionEntry * GetCurrentFunction() { return mCurrentFunction; }

  void IncScope()
  {
    mScopeInfo.push_back(new std::vector<CTableEntry *>);
    mCurrentScope++;
  }
  void DecScope()
  {
    // Remove variables in the old mScope and store them in the archive.
    std::vector<CTableEntry *> * old_scope = mScopeInfo.back();
    mScopeInfo.pop_back();
    mVarArchive.insert(mVarArchive.end(), old_scope->begin(), old_scope->end());

    // Make sure to clean up the mTableMap.
    for (int i = 0; i < (int) old_scope->size(); i++) {
      CTableEntry * oldEntry = (*old_scope)[i];

      // If this entry is shadowing another, make shadowed version active again.
      if (oldEntry->GetNext() != NULL) {
        mTableMap[oldEntry->GetName()] = oldEntry->GetNext();
      }

      // Otherwise just remove it from being an active variable name.
      else {
        mTableMap.erase(oldEntry->GetName());
      }
    }

    delete old_scope;
    mCurrentScope--;
  }

  int NextLabelID() { return mNextLabelID++; }
  std::string NextLabelID(std::string prefix)
  {
    std::stringstream sstm;
    sstm << prefix << mNextLabelID++;
    return sstm.str();
  }

  int GetWhileDepth() { return (int) mWhileEndStack.size(); }
  void PushWhileEndLabel(const std::string & end_label)
  {
    mWhileEndStack.push_back(end_label);
  }
  const std::string & GetWhileEndLabel() { return mWhileEndStack.back(); }
  void PopWhileEndLabel() { mWhileEndStack.pop_back(); }

  // Lookup will find an entry and return it.  If that entry is not in the table, it will return NULL
  CTableEntry * Lookup(std::string inName)
  {
    if (mTableMap.find(inName) == mTableMap.end()) return NULL;
    return mTableMap[inName];
  }

  CFunctionEntry * FunctionLookup(std::string inName)
  {
    if(mFunctionMap.find(inName) == mFunctionMap.end()) return NULL;
    return mFunctionMap[inName];
  }
  // Determine if a variable has been declared in the current scope.
  bool InCurScope(std::string inName)
  {
    if (mTableMap.find(inName) == mTableMap.end()) return false;
    return mTableMap[inName]->GetScope() == mCurrentScope;
  }



  // Insert an entry into the symbol table.
  CTableEntry * AddEntry(int inType, std::string inName)
  {
    // Create the new entry for this variable.
    CTableEntry * newEntry = new CTableEntry(inType, inName);
    newEntry->SetVarID( GetNextID() );
    newEntry->SetScope(mCurrentScope);

    // If an old entry exists by this name, shadow it.
    CTableEntry * oldEntry = Lookup(inName);
    if (oldEntry) newEntry->SetNext(oldEntry);

    // Save the information for the new entry.
    mTableMap[inName] = newEntry;
    mScopeInfo[mCurrentScope]->push_back(newEntry);

    if(mFunctionMode) {
      //std::cout << "mCurrentFunction: ";
      //std::cout << mCurrentFunction << std::endl;
      mCurrentFunction->PushArg(newEntry);
    }

    return newEntry;
  }

  // Insert a function into the symbol table
  CFunctionEntry * AddFunction(std::string name, int type)
  {
    CFunctionEntry * newEntry = new CFunctionEntry(name, type);
    newEntry->SetVarID( GetNextID());
    mFunctionMap[name] = newEntry;
    newEntry->SetReturn(AddTempEntry(type));
    newEntry->SetReturnValue(AddTempEntry(type));
    return newEntry;
  }

  // Insert a temp variable entry into the symbol table.
  CTableEntry * AddTempEntry(int inType) {
    CTableEntry * newEntry = new CTableEntry(inType);
    newEntry->SetVarID( GetNextID() );
    return newEntry;
  }

  void AddFunctionMap(std::string name){
    mFunctionMap[name];
  }

  // Don't create a full variable; just get an unused variable ID.
  int GetTempVarID() { return GetNextID(); }
  void FreeTempVarID(int id) { (void) id; /* Nothing for now... */ }

  void RemoveEntry(CTableEntry * del_var) {
    // We no longer nead this entry...
    delete del_var;
  }
};

#endif
