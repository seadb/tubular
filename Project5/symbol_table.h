#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

//using namespace std;

// A CTableEntry contains all of the stored information about a single variable.

class CTableEntry {
protected:
  std::string mName;       // Variable name used by sourcecode.
  std::string mType;       // Type (char, int, array)
  int mVarID;              // What is the intermediate code ID for this variable?

  // NOTE: This is also where you want to track other var info like
  //       type, line it was declared on, array status, etc.

public:
  CTableEntry() : mName(""), mVarID(-1), mType("") { ; }
  CTableEntry(const std::string & name, const std::string & type) : mName(name), 
              mVarID(-1), mType(type) { ; }
  ~CTableEntry() { ; }

  const std::string & GetName() const { return mName; }
  void SetName(std::string name) { mName = name; }
  
  const std::string & GetType() const { return mType; }
  void SetType(std::string type) { mType = type; }
  
  int GetVarID() const { return mVarID; }
  void SetVarID(int varID) { mVarID = varID; }
};


class CArrayEntry: public CTableEntry {
  private:
  int mSize;                // Length of the array (optional)
  char * mCharArray;       // char array content
  int * mIntArray;         // int array content
  public:
    CArrayEntry() : mSize(0), mCharArray(NULL)
      ,mIntArray(NULL){ mName = ""; mType = ""; mVarID = -1; }

    CArrayEntry(const std::string & name, const std::string & type, const int size) 
      :  mSize(size) { mName = name; mType = type; mSize = size; }
    ~CArrayEntry() { ; }

   const int & GetSize() const { return mSize; } 
   void SetSize(int size) { mSize = size; }

   void SetCharArray(char * charArray) { mCharArray = charArray;} 
   void SetIntArray(int * intArray)    { mIntArray = intArray;} 

   const int * GetIntArray() const { return mIntArray; }
   const char * GetCharArray() const { return mCharArray; }
};


// The CSymbolTable allows easy lookup of CTableEntry objects.
class CSymbolTable {
  private:
    bool visible;
    std::map<std::string, CTableEntry *> mTableMap;
    //CSymbolTables *tables;
  public:
    CSymbolTable(/*CSymbolTables *t*/)/* : tables(t)*/ { ; }
    ~CSymbolTable() { ; }

    int GetSize() { return mTableMap.size(); }

    int SetVisible(bool value) { visible = value; }
    int Visible() { return visible; }

    // Lookup will find an entry and return it.
    // If that entry is not in the table, it will return NULL
    CTableEntry * Lookup(std::string in_name) {
      if (mTableMap.find(in_name) == mTableMap.end()) return NULL;
      return mTableMap[in_name];
    }

    // Insert an entry into the symbol table.
    CTableEntry * AddEntry(std::string name, int nextVarID, std::string type) {
      CTableEntry * newEntry = new CTableEntry(name, type);
      newEntry->SetVarID( nextVarID );
      mTableMap[name] = newEntry;
      return newEntry;
    }
    
    // Insert an array entry into the symbol table
    CTableEntry * AddArray(std::string name, int nextVarID, std::string type, int size) {
      CTableEntry * newEntry = new CArrayEntry(name, type, size);
      newEntry->SetVarID( nextVarID );
      mTableMap[name] = newEntry;
      return newEntry;
    }

  };

class CSymbolTables {
private:
  std::vector<CSymbolTable*> mTables;
 // std::vector<CSymbolTable*> mDiscarded;      // not used
  //int mScope;                                 // not used
  int mNextVarID;                // Next variable ID to use.
  int mNextLabelID;              // Next label ID to use.
  std::vector<std::string> mBreaks;             // Keeps track of "break;" statements

  // Figure out the next memory position to use. Ideally, we should recycle these!

public:
  CSymbolTables() : mNextVarID(1), mNextLabelID(0)/*, mScope(-1)*/ {
      AddTable();
  }

  ~CSymbolTables() { ; }
  
  std::vector<std::string> breaks;
  
  int GetNextID() { return mNextVarID++; }
  const int GetNumVars() const { return mNextVarID; }

  void PopBackBreaks(){ mBreaks.pop_back(); }
  void PushBackBreaks(std::string element){ mBreaks.push_back(element); }
  int BreaksSize(){ return mBreaks.size();}
  std::string BreaksBack() { return mBreaks.back(); }

  CSymbolTable * current() {
    for(int i=mTables.size()-1; i >= 0; i--){
      if(mTables[i]->Visible()){
        return mTables[i];
      }
    }
    return NULL;
  }

  CTableEntry * AddEntry(std::string name, std::string type) {
    return current()->AddEntry(name, GetNextID(), type);
  }

  CTableEntry * AddArray(std::string name, std::string type, int size){
    return current()->AddArray(name, GetNextID(), type, size);
  }

  // Add a new symbol table for a new scope
  void AddTable() {
    mTables.push_back(new CSymbolTable(/*this*/));//doesnt use scope as the index
    mTables.back()->SetVisible(true);
    //mScope +=1;
    }
  
  // Hide the last table
  void HideTable() {
    mTables.pop_back();
    //mScope -=1;
  }


  int NextLabelID() { return mNextLabelID++; }
  std::string NextLabelID(std::string prefix) {
    std::stringstream ss;
    ss << prefix << mNextLabelID++;
    return ss.str();
  }


  // Lookup will find an entry and return it.
  // If that entry is not in the table, it will return NULL
  CTableEntry * Lookup(std::string name) {
    for(int i = mTables.size()-1 ; i >= 0; i--) {
      CTableEntry * result = mTables[i]->Lookup(name);
      if(mTables[i]->Visible() && result != 0) {
        return result;
        }
    }
    return NULL;
  }

  // Insert a temporary variable entry into the symbol table.
  CTableEntry * AddTempEntry() {
    CTableEntry * newEntry = new CTableEntry();
    newEntry->SetVarID( GetNextID() );
    return newEntry;
  }

  void ShowAll(){
    std::vector<CSymbolTable*>::iterator it = mTables.begin();
    for( it; it != mTables.end(); it++){
      (*it)->SetVisible(true);
    }
  }

};

#endif
// Insert an entry into the symbol table.
/*CTableEntry * CSymbolTable::AddEntry(std::string in_name) {
  CTableEntry * new_entry = new CTableEntry(in_name);
  new_entry->SetVarID( tables->GetNextID() );
  mTableMap[in_name] = new_entry;
  return new_entry;
};*/


