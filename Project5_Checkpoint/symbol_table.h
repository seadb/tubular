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
  CTableEntry * mNext; // A pointer to another variable that this one is shadowing

  CTableEntry(int in_type) 
    : mTypeID (in_type)
    , mName("__TEMP__")
    , mScope(-1)
    , mIsTemp(true)
    , mVarID(-1)
    , mNext(NULL)
  {
  }

  CTableEntry(int in_type, const std::string in_name)
    : mTypeID(in_type)
    , mName(in_name)
    , mScope(-1)
    , mIsTemp(false)
    , mVarID(-1)
    , mNext(NULL)
  {
  }
  virtual ~CTableEntry() { ; }

public:
  int GetType()           const { return mTypeID; }
  std::string GetName()   const { return mName; }
  int GetScope()          const { return mScope; }
  bool GetTemp()          const { return mIsTemp; }
  int GetVarID()          const { return mVarID; }
  CTableEntry * GetNext() const { return mNext; }

  void SetName(std::string in_name)     { mName = in_name; }
  void SetScope(int in_scope)           { mScope = in_scope; }
  void SetVarID(int in_id)              { mVarID = in_id; }
  void SetNext(CTableEntry * in_next)   { mNext = in_next; }
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
  CSymbolTable() : mCurrentScope(0), mNextVarID(0), mNextLabelID(0) { 
    mScopeInfo.push_back(new std::vector<CTableEntry *>);
  }
  ~CSymbolTable() {
    // Clean up all variable entries
    while (mCurrentScope >= 0) DecScope();
    for (int i = 0; i < (int) mVarArchive.size(); i++) delete mVarArchive[i];
  }

  int GetSize() const { return (int) mTableMap.size(); }
  int GetCurScope() const { return mCurrentScope; }
  const std::vector<CTableEntry *> & GetScopeVars(int scope) {
    if (scope < 0 || scope >= (int) mScopeInfo.size()) {
      std::cerr << "Internal Compiler Error: Requesting vars from scope #" << scope
                << ", but only " << mScopeInfo.size() << " scopes exist." << std::endl;
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
      CTableEntry * old_entry = (*old_scope)[i];

      // If this entry is shadowing another, make shadowed version active again.
      if (old_entry->GetNext() != NULL) {
        mTableMap[old_entry->GetName()] = old_entry->GetNext();
      }

      // Otherwise just remove it from being an active variable mName.
      else {
        mTableMap.erase(old_entry->GetName());
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
  CTableEntry * Lookup(std::string in_name) {
    if (mTableMap.find(in_name) == mTableMap.end()) return NULL;
    return mTableMap[in_name];
  }

  // Determine if a variable has been declared in the current scope.
  bool InCurScope(std::string in_name) {
    if (mTableMap.find(in_name) == mTableMap.end()) return false;
    return mTableMap[in_name]->GetScope() == mCurrentScope;
  }

  // Insert an entry into the symbol table.
  CTableEntry * AddEntry(int in_type, std::string in_name) {
    // Create the new entry for this variable.
    CTableEntry * new_entry = new CTableEntry(in_type, in_name);
    new_entry->SetVarID( GetNextID() );
    new_entry->SetScope(mCurrentScope);

    // If an old entry exists by this mName, shadow it.
    CTableEntry * old_entry = Lookup(in_name);
    if (old_entry) new_entry->SetNext(old_entry);

    // Save the information for the new entry.
    mTableMap[in_name] = new_entry;
    mScopeInfo[mCurrentScope]->push_back(new_entry);
    return new_entry;
  }

  // Insert a temp variable entry into the symbol table.
  CTableEntry * AddTempEntry(int in_type) {
    CTableEntry * new_entry = new CTableEntry(in_type);
    new_entry->SetVarID( GetNextID() );
    return new_entry;
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
