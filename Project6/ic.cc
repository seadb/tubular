#include "ic.h"
/******************************************
 * BEGIN ICEntry
 *****************************************/

bool first_run = true;
int label_num = 0;

void ICEntry::PrintIC(std::ostream & ofs)
{
  if(first_run) {
    ofs << "  store " << mArray->static_memory_size << " 0" << std::endl;
    first_run = false;
  }

  // If there is a label, include it in the output.
  if (label != "") {
    ofs << label << ": " << std::endl << "  nop" << std::endl;
  }

  if (mInst != "") {
    // Print intermediate code as comment
    std::stringstream out_line;
    out_line << '#' << mInst << " ";
    for (int i = 0; i < (int) args.size(); i++) {
      out_line << args[i]->AsString() << " ";
    }
    ofs << out_line.str() << std::endl;

    if(mInst == "val_copy"){
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      ofs << "  val_copy " << args[0]->AsAssemblyString() << " regB" << std::endl;
      args[1]->AssemblyWrite(ofs, args[1]->GetID(), 'B');
    }

    else if(mInst == "add" || mInst == "sub" || mInst == "mult" ||
            mInst == "div" || mInst == "mod" || mInst == "test_less" ||
            mInst == "test_gtr" || mInst == "test_equ" ||
            mInst == "test_nequ" || mInst == "test_lte" ||
            mInst == "test_lte" || mInst == "test_gte") {
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      args[1]->AssemblyRead(ofs, args[1]->GetID(), 'B');
      ofs << "  " << mInst << " " << args[0]->AsAssemblyString() << " ";
      ofs <<  args[1]->AsAssemblyString() << " regC" << std::endl;
      args[1]->AssemblyWrite(ofs, args[2]->GetID(), 'C');
    } 
    else if(mInst == "jump" || mInst == "out_int" || mInst == "out_char") {
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      ofs << "  " << mInst << " " << args[0]->AsAssemblyString() << " " << std::endl;
    }
    else if(mInst == "jump_if_0" || mInst == "jump_if_n0") {
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      args[1]->AssemblyRead(ofs, args[1]->GetID(), 'A');
      ofs << "  " << mInst << " " << args[0]->AsAssemblyString() << " ";
      ofs <<  args[1]->AsAssemblyString() << std::endl;
    }
    else if(mInst == "random") {
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      ofs << "  " << mInst << " " << args[0]->AsAssemblyString() << " ";
      ofs << args[1]->AsAssemblyString() << " regB" << std::endl;
      args[1]->AssemblyWrite(ofs, args[1]->GetID(), 'B');
    }
    else if(mInst == "nop") {
      ofs << "  nop" << std::endl;
    }
    else if(mInst == "push") {
      args[0]->AssemblyRead(ofs, args[0]->GetID(), 'A');
      ofs << "  " << mInst << " " << args[0]->AsAssemblyString();
    }
    else if(mInst == "pop") {
      ofs << "  " << mInst << " regA" << std::endl;
      args[0]->AssemblyWrite(ofs, args[0]->GetID(), 'A');
    }
    else if(mInst == "ar_get_idx" || mInst == "ar_set_idx") {
      ofs << "  load " << args[0]->GetID() << " regA" << std::endl;
      args[1]->AssemblyRead(ofs, args[1]->GetID(), 'B');
      ofs << "  add regA 1 regA" << std::endl;
      ofs << "  add regA regB regA" << std::endl;
      if(mInst == "ar_get_idx") {
        ofs << "  mem_copy regA " << args[2]->GetID() << std::endl;
      }
      else {
        if(args[2]->IsScalar()) {
          ofs << "  mem_copy " << args[2]->GetID() << " regA" << std::endl;
        }
        else {
          ofs << "  val_copy " << args[2]->AsString() << " regC" << std::endl;
          ofs << "  store regC regA" << std::endl;
        }
      }
    }
    else if(mInst == "ar_get_size") {
      ofs << "  load " << args[0]->GetID() << " regA" << std::endl;
      ofs << "  mem_copy regA " << args[1]->GetID() << std::endl;
    }
    else if(mInst == "ar_set_size") {
      ofs << "  load " << args[0]->GetID() << " regA" << std::endl;
      if(args[1]->IsScalar()) {
        ofs << "  load " << args[1]->GetID() << " regB" << std::endl;
      }
      else {
        ofs << "  val_copy " << args[1]->AsString() << " regB" << std::endl;
      }
      ofs << "  load regA regC" << std::endl;
      ofs << "  store regB regA" << std::endl;
      ofs << "  test_lte regB regC regD" << std::endl;
      ofs << "  jump_if_n0 regD resize_end_" << label_num + 1 << std::endl;
      ofs << "  load 0 regD" << std::endl;
      ofs << "  add regD 1 regE" << std::endl;
      ofs << "  add regE regB regE" << std::endl;
      ofs << "  store regE 0" << std::endl;
      ofs << "  store regD " << args[0]->GetID() << std::endl;
      ofs << "  store regB regD" << std::endl;
      ofs << "resize_start_" << label_num++ << ":" << std::endl;
      ofs << "  add regA 1 regA" << std::endl;
      ofs << "  add regD 1 regD" << std::endl;
      ofs << "  test_gtr regD regE regF" << std::endl;
      ofs << "  jump_if_n0 regF resize_end_" << label_num << std::endl;
      ofs << "  mem_copy regA regD" << std::endl;
      ofs << "  jump resize_start_" << label_num - 1 << std::endl;
      ofs << "resize_end_" << label_num++ << ":" << std::endl;
      ofs << "  nop" << std::endl;
    }
    else if(mInst == "ar_copy") {
    }

  }

  // If there is a comment, print it!
  //if (comment != "") {
    //while (out_line.str().size() < 40) out_line << " "; // Align comments for easy reading.
    //out_line << "# " << comment;
 // }

  //ofs << out_line.str() << std::endl;
}

/***************************************************
 * BEGIN ICArray
****************************************************/

ICEntry& ICArray::AddLabel(std::string label_id, std::string cmt)
{
  ICEntry * new_entry = new ICEntry("","",this);
  new_entry->SetLabel(label_id);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}


// This is a quick way to add scalar/array/none args, with all needed error checking.
void ICArray::AddArg(ICEntry * entry, int in_arg, ArgType::type expected_type)
{
  switch (expected_type) {
  case ArgType::NONE:       // No argument expected...
    if (in_arg != -1) {     // ... so make sure we're not passing one in...
      std::cerr << "INTERNAL ERROR: Too many arguments provided for inst '"
                << entry->GetInstName() << "'." << std::endl;
    }
    break;
  case ArgType::VALUE:      // Argument should have a VALUE, in this case must be a scalar input
  case ArgType::SCALAR:     // Argument must be a scalar that gets read into!
    if (in_arg == -1) {     // ... Make sure we're actually passing an argument in!
      std::cerr << "INTERNAL ERROR: Too insufficient arguments provided for inst '"
                << entry->GetInstName() << "'." << std::endl;
    }
    else {
      entry->AddScalarArg(in_arg);
    }
    break;

  case ArgType::ARRAY:      // Argument must be an array variable.
    if (in_arg == -1) {     // ... Make sure we're actually passing an argument in!
      std::cerr << "INTERNAL ERROR: Insufficient arguments provided for inst '"
                << entry->GetInstName() << "'." << std::endl;
    }
    else {
      entry->AddArrayArg(in_arg);
    }
    break;
  }
}


void ICArray::AddArg(ICEntry * entry, const std::string & in_arg, ArgType::type expected_type)
{
  switch (expected_type) {
  case ArgType::NONE:       // No argument expected, but in input string means we received one!
    std::cerr << "INTERNAL ERROR: Too many arguments provided for inst '"
              << entry->GetInstName() << "'." << std::endl;
    break;
  case ArgType::VALUE:      // Argument should have a VALUE, in this case const was input.
    entry->AddConstArg(in_arg);
    break;
  case ArgType::SCALAR:     // Argument should have been a scalar variable, not a const!
  case ArgType::ARRAY:      // Argument should have been array variable, not a const!
    std::cerr << "INTERNAL ERROR: Incorrect type of arguments provided for inst '"
              << entry->GetInstName() << "'." << std::endl;
    break;
  }
}


// This Add is called when all numbers are given to the Add method -- in other words all
// arguments should be scalars, arrays, or none based on type layout.
ICEntry& ICArray::Add(std::string inst_name, int arg1, int arg2, int arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, int arg1, int arg2, std::string arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name,"", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, int arg1, std::string arg2, int arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, int arg1, std::string arg2, std::string arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, std::string arg1, int arg2, int arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, std::string arg1, int arg2, std::string arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, std::string arg1, std::string arg2, int arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}

ICEntry& ICArray::Add(std::string inst_name, std::string arg1, std::string arg2, std::string arg3, std::string cmt)
{
  ICEntry * new_entry = new ICEntry(inst_name, "", this);
  if (mArgTypeMap.find(inst_name) == mArgTypeMap.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = mArgTypeMap[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  mICArray.push_back(new_entry);
  return *new_entry;
}


void ICArray::PrintIC(std::ostream & ofs)
{
  //ofs << "# Output from Dr. Charles Ofria's sample compiler." << std::endl;
  for (int i = 0; i < (int) mICArray.size(); i++) {
    mICArray[i]->PrintIC(ofs);
  }
}


