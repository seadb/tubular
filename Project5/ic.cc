#include "ic.h"

void ICEntry::PrintIC(std::ostream & ofs)
{
  std::stringstream out_line;

  // If there is a label, include it in the output.
  if (label != "") { out_line << label << ": "; }
  else { out_line << "  "; }

  // If there is an instruction, print it and all its arguments.
  if (mInst != "") {
    out_line << mInst << " ";
    for (int i = 0; i < (int) args.size(); i++) {
      out_line << args[i]->AsString() << " ";
    }
  }

  // If there is a comment, print it!
  if (comment != "") {
    while (out_line.str().size() < 40) out_line << " "; // Align comments for easy reading.
    out_line << "# " << comment;
  }

  ofs << out_line.str() << std::endl;
}

ICEntry& ICArray::AddLabel(std::string label_id, std::string cmt)
{
  ICEntry * new_entry = new ICEntry();
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
  ICEntry * new_entry = new ICEntry(inst_name);
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
