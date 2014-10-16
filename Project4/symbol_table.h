#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>

using namespace std;

// A tableEntry contains all of the stored information about a single variable.

class tableEntry {
protected:
  std::string name;       // Variable name used by sourcecode.
  int var_id;        // What is the intermediate code ID for this variable?
  std::string type;
  // NOTE: This is also where you want to track other var info like
  //       type, line it was declared on, array status, etc.

public:
  tableEntry() : name(""), var_id(-1), type("") { ; }
  tableEntry(const std::string & in_name, const std::string & in_type) : name(in_name), var_id(-1), type(in_type) { ; }
  ~tableEntry() { ; }

  const std::string & GetName() const { return name; }
  int GetVarID() const { return var_id; }

  void SetType(std::string in_type) { type = in_type; }
  void SetName(std::string in_name) { name = in_name; }
  void SetVarID(int in_id) { var_id = in_id; }
  const std::string & GetType() const { return type; }
};

class symbolTables;

// The symbolTable allows easy lookup of tableEntry objects.
class symbolTable {
private:
  bool visible;
  std::map<std::string, tableEntry *> tbl_map;
  symbolTables *tables;
public:
  symbolTable(symbolTables *t) : tables(t) { ; }
  ~symbolTable() { ; }

  int GetSize() { return tbl_map.size(); }

  int SetVisible(bool value) { visible = value; }
  int Visible() { return visible; }

  // Lookup will find an entry and return it.
  // If that entry is not in the table, it will return NULL
  tableEntry * Lookup(std::string in_name) {
    if (tbl_map.find(in_name) == tbl_map.end()) return NULL;
    return tbl_map[in_name];
  }

  // Insert an entry into the symbol table.
  tableEntry * AddEntry(std::string in_name, int next_var_id, std::string in_type) {
    tableEntry * new_entry = new tableEntry(in_name, in_type);
    new_entry->SetVarID( next_var_id );
    tbl_map[in_name] = new_entry;
    return new_entry;
  }

};

class symbolTables {
private:
  std::vector<symbolTable*> tables;
  std::vector<symbolTable*> discarded;
  int scope;
  int next_var_id;                // Next variable ID to use.
  int next_label_id;              // Next label ID to use.

  // Figure out the next memory position to use. Ideally, we should recycle these!

public:
  symbolTables() : next_var_id(1), next_label_id(0), scope(-1) {
      AddTable();
  }

  ~symbolTables() { ; }
  int GetNextID() { return next_var_id++; }

  vector<string> breaks;

  symbolTable * current() {
    for(int i=tables.size()-1; i >= 0; i--){
      if(tables[i]->Visible()){
        return tables[i];
      }
    }
    return NULL;
  }

  tableEntry * AddEntry(std::string in_name, std::string in_type) {
    return current()->AddEntry(in_name, GetNextID(), in_type);
  }

  // Add a new symbol table for a new scope
  void AddTable() {
    tables.push_back(new symbolTable(this));//doesnt use scope as the index
    tables.back()->SetVisible(true);
    //cout << "push back" << scope << endl;
    //std::vector<symbolTable*>::iterator it = tables.begin();
    //tables.insert(it+scope, new symbolTable(this));
    scope +=1;
    }
  // Hide the last table
  void HideTable() {
    //tables.back()->SetVisible(false);
    //discarded.push_back(tables[scope]);
    //std::cout << "pop" << scope << endl;
    tables.pop_back();
    scope -=1;
  }

  int NextLabelID() { return next_label_id++; }
  std::string NextLabelID(std::string prefix) {
    std::stringstream ss;
    ss << prefix << next_label_id++;
    return ss.str();
  }

  int GetNumVars() { return next_var_id; }

  // Lookup will find an entry and return it.
  // If that entry is not in the table, it will return NULL
  tableEntry * Lookup(std::string in_name) {
    for(int i = tables.size()-1 ; i >= 0; i--) {
      tableEntry * result = tables[i]->Lookup(in_name);
      if(tables[i]->Visible() && result != 0) {
        return result;
        }
    }
    return NULL;
  }

  // Insert a temporary variable entry into the symbol table.
  tableEntry * AddTempEntry() {
    tableEntry * new_entry = new tableEntry();
    new_entry->SetVarID( GetNextID() );
    return new_entry;
  }

  void ShowAll(){
    std::vector<symbolTable*>::iterator it = tables.begin();
    for( it; it != tables.end(); it++){
      (*it)->SetVisible(true);
    }
  }


};

#endif
// Insert an entry into the symbol table.
/*tableEntry * symbolTable::AddEntry(std::string in_name) {
  tableEntry * new_entry = new tableEntry(in_name);
  new_entry->SetVarID( tables->GetNextID() );
  tbl_map[in_name] = new_entry;
  return new_entry;
};*/


