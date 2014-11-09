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

class CSymbolTable;

class CTableEntry {
  friend class CSymbolTable;
protected:
  int mTypeID;       // What is the type of this variable?
  std::string mName;  // Variable mName used by sourcecode.
  int mScope;         // What mScope was this variable declared at?
  bool mIsTemp;      // Is this variable just temporary (internal to compiler)
  int mVarID;        // What is the intermediate code ID for this variable?
  CTableEntry * mNext; // A pointer to another entry that this one is shadowing
  CTableEntry * mIndex; // Is there an index associated with this var
  CTableEntry * mArray; // Is there an array?
  int mSize;
  bool mNegative;

  CTableEntry(int inType)
    : mTypeID (inType)
    , mName("__TEMP__")
    , mScope(-1)
    , mIsTemp(true)
    , mVarID(-1)
    , mNext(NULL)
    , mIndex(NULL)
    , mArray(NULL)
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

  void SetName(std::string inName)     { mName = inName; }
  void SetScope(int inScope)           { mScope = inScope; }
  void SetVarID(int inID)              { mVarID = inID; }
  void SetSize(int inSize)             { mSize = inSize; }
  void SetNegative(bool inNegative)        { mNegative = inNegative; }
  void SetNext(CTableEntry * inNext)   { mNext = inNext; }
  void SetIndex(CTableEntry * inIndex) { mIndex = inIndex; }
  void SetArray(CTableEntry * inArray ) { mArray = inArray;  }
};



class CSymbolTable {
private:
  std::map<std::string, CTableEntry *> mTableMap;          // A map of active variables
  std::vector<std::vector<CTableEntry *> *> mScopeInfo;  // Variables declared in each scope
  std::vector<CTableEntry *> mVarArchive;                // Variables that are out of scope
  int mCurrentScope;                                        // Current mScope level
  int mNextVarID;                                      // Next variable ID to use.
  int mNextLabelID;                                    // Next label ID to use.
  std::vector<std::string> mWhileEndStack;             // End labels of active while commands

  // Figure out the next memory position to use.  Ideally, we should be recycling these!!
  int GetNextID() { return mNextVarID++; }
public:
  CSymbolTable() : mCurrentScope(0), mNextVarID(1), mNextLabelID(0) {
    mScopeInfo.push_back(new std::vector<CTableEntry *>);
  }
  ~CSymbolTable() {
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

  void IncScope() {
    mScopeInfo.push_back(new std::vector<CTableEntry *>);
    mCurrentScope++;
  }
  void DecScope() {
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

      // Otherwise just remove it from being an active variable mName.
      else {
        mTableMap.erase(oldEntry->GetName());
      }
    }

    delete old_scope;
    mCurrentScope--;
  }

  int NextLabelID() { return mNextLabelID++; }
  std::string NextLabelID(std::string prefix) {
    std::stringstream sstm;
    sstm << prefix << mNextLabelID++;
    return sstm.str();
  }

  int GetWhileDepth() { return (int) mWhileEndStack.size(); }
  void PushWhileEndLabel(const std::string & end_label) { mWhileEndStack.push_back(end_label); }
  const std::string & GetWhileEndLabel() { return mWhileEndStack.back(); }
  void PopWhileEndLabel() { mWhileEndStack.pop_back(); }

  // Lookup will find an entry and return it.  If that entry is not in the table, it will return NULL
  CTableEntry * Lookup(std::string inName) {
    if (mTableMap.find(inName) == mTableMap.end()) return NULL;
    return mTableMap[inName];
  }

  // Determine if a variable has been declared in the current scope.
  bool InCurScope(std::string inName) {
    if (mTableMap.find(inName) == mTableMap.end()) return false;
    return mTableMap[inName]->GetScope() == mCurrentScope;
  }

  // Insert an entry into the symbol table.
  CTableEntry * AddEntry(int inType, std::string inName) {
    // Create the new entry for this variable.
    CTableEntry * newEntry = new CTableEntry(inType, inName);
    newEntry->SetVarID( GetNextID() );
    newEntry->SetScope(mCurrentScope);

    // If an old entry exists by this mName, shadow it.
    CTableEntry * oldEntry = Lookup(inName);
    if (oldEntry) newEntry->SetNext(oldEntry);

    // Save the information for the new entry.
    mTableMap[inName] = newEntry;
    mScopeInfo[mCurrentScope]->push_back(newEntry);
    return newEntry;
  }

  // Insert a temp variable entry into the symbol table.
  CTableEntry * AddTempEntry(int inType) {
    CTableEntry * newEntry = new CTableEntry(inType);
    newEntry->SetVarID( GetNextID() );
    return newEntry;
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
