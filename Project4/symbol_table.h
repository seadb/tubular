#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include <string>
#include <sstream>
#include <vector>

// A tableEntry contains all of the stored information about a single variable.

class tableEntry {
protected:
  std::string name;       // Variable name used by sourcecode.
  int var_id;        // What is the intermediate code ID for this variable?
  // NOTE: This is also where you want to track other var info like
  //       type, line it was declared on, array status, etc.

public:
  tableEntry() : name(""), var_id(-1) { ; }
  tableEntry(const std::string & in_name) : name(in_name), var_id(-1) { ; }
  ~tableEntry() { ; }

  const std::string & GetName() const { return name; }
  int GetVarID() const { return var_id; }

  void SetName(std::string in_name) { name = in_name; }
  void SetVarID(int in_id) { var_id = in_id; }
};

// The symbolTable allows easy lookup of tableEntry objects.

class symbolTable {
private:
  bool visible;
  std::map<std::string, tableEntry *> tbl_map;
  symbolTables *tables;
public:
  symbolTable(symbolTables *t) : next_var_id(1), next_label_id(0), tables(t) { ; }
  ~symbolTable() { ; }

  int GetSize() { return tbl_map.size(); }
  int GetNumVars() { return next_var_id; }

  int SetVisible(bool value) { visible = value; }
  int Visible() { return visible; }

  // Lookup will find an entry and return it.
  // If that entry is not in the table, it will return NULL
  tableEntry * Lookup(std::string in_name) {
    if (tbl_map.find(in_name) == tbl_map.end()) return NULL;
    return tbl_map[in_name];
  }


  // Insert an entry into the symbol table.
  tableEntry * AddEntry(std::string in_name) {
    tableEntry * new_entry = new tableEntry(in_name);
    new_entry->SetVarID( t->GetNextID() );
    tbl_map[in_name] = new_entry;
    return new_entry;
  }

  // Insert a temporary variable entry into the symbol table.
  tableEntry * AddTempEntry() {
    tableEntry * new_entry = new tableEntry();
    new_entry->SetVarID( t->GetNextID() );
    return new_entry;
  }
};

class symbolTables {
private:
  std::vector<symbolTable> tables;
  std::vector<symbolTable> discarded;
  int scope;
  int next_var_id;                // Next variable ID to use.
  int next_label_id;              // Next label ID to use.

  // Figure out the next memory position to use. Ideally, we should recycle these!
  int GetNextID() { return next_var_id++; }
public:
  symbolTables() : next_var_id(1), next_label_id(0), current_scope(0) { ; }
  ~symbolTables() { ; }

  void AddTable() {
    symbolTable temp;
    std::vector<symbolTable>::iterator it = tables.begin();
    tables.insert(it+scope, temp);
    scope +=1;
    }

  void PopTable() {
    symbolTable temp = tables.back(); //grab last element
    std::vector<symbolTable>::iterator it = discarded.begin();
    discarded.insert(it+scope, temp);
    tables.pop_back();
    scope -=1;
  }

  int NextLabelID() { return next_label_id++; }
  std::string NextLabelID(std::string prefix) {
    std::stringstream ss;
    ss << prefix << next_label_id++;
    return ss.str();
  }

  // Lookup will find an entry and return it.
  // If that entry is not in the table, it will return NULL
  tableEntry * Lookup(std::string in_name) {
    for(int i = scope - 1; i >= 0; i--) {
      tableEntry result = tables[i].Lookup(in_name);
      if(tables[i].Visible() && result != 0) {
        return result;
        }
    }
  }

  void IncreaseScope() { scope++; }
  void DecreaseScope() { scope--; }

};

#endif

