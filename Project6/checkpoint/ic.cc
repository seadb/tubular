#include "ic.h"

void IC_Entry::PrintIC(std::ostream & ofs)
{
  std::stringstream out_line;

  // If there is a label, include it in the output.
  if (label != "") { out_line << label << ": "; }
  else { out_line << "  "; }

  // If there is an instruction, print it and all its arguments.
  if (inst != "") {
    out_line << inst << " ";
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

void IC_Entry::PrintTubeCode(std::ostream & ofs)
{
  // If this entry has a label, print it on its own line.
  if (label != "") { ofs << label << ":" << std::endl; }

  // If there is a comment, print it on its own line.
  if (comment != "") { ofs << "# COMMENT: " << comment << std::endl; }

  // If we don't have an instruction, stop here.
  if (inst == "") return;

  // Print a comment listing the intermediate code instruction being converted.
  ofs << "### Converting: " << inst;
  for (int i = 0; i < (int) args.size(); i++) {
    ofs << " " << args[i]->AsString();
  }
  ofs << std::endl;

  // By default, don't load or store any arguments.
  bool load1 = false,  load2 = false,  load3 = false;
  bool store1 = false, store2 = false, store3 = false;
  
  // Depending on the instruction, determine which arguments should be loaded or stored.
  if (inst == "val_copy")        { load1 = true; store2 = true; }
  else if (inst == "add")        { load1 = true; load2 = true; store3 = true; }
  else if (inst == "sub")        { load1 = true; load2 = true; store3 = true; }
  else if (inst == "mult")       { load1 = true; load2 = true; store3 = true; }
  else if (inst == "div")        { load1 = true; load2 = true; store3 = true; }
  else if (inst == "mod")        { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_less")  { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_gtr")   { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_equ")   { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_nequ")  { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_gte")   { load1 = true; load2 = true; store3 = true; }
  else if (inst == "test_lte")   { load1 = true; load2 = true; store3 = true; }
  else if (inst == "jump")       { load1 = true; }
  else if (inst == "jump_if_0")  { load1 = true; load2 = true; }
  else if (inst == "jump_if_n0") { load1 = true; load2 = true; }
  else if (inst == "nop")        { ; }
  else if (inst == "random")     { load1 = true; store2 = true; }
  else if (inst == "out_int")    { load1 = true; }
  else if (inst == "out_float")  { load1 = true; }
  else if (inst == "out_char")   { load1 = true; }
  
  else if (inst == "ar_get_idx")  { load1 = true; load2 = true;  store3 = true; }
  else if (inst == "ar_set_idx")  { load1 = true; load2 = true;  load3 = true; }
  else if (inst == "ar_get_size") { load1 = true; store2 = true; }
  else if (inst == "ar_set_size") { load1 = true; load2 = true;  }
  else if (inst == "ar_copy")     { load1 = true; store2 = true; }
  
  else {
    std::cerr << "Internal Compiler Error! Unknown instruction '"
              << inst
              << "' in IC_Entry::IC_Entry(): " << inst << std::endl;
  }

  // For the moment, we're locking in registers.
  // NOTE: We should do more here for optimizations in Project 8
  std::string reg1="regA", reg2="regB", reg3="regC", reg4="regD", reg5="regE", reg6="regF", reg7="regG";

  // Load any relevant values.
  if (load1 && !args[0]->IsConst()) ofs << "  load " << args[0]->GetID() << " " << reg1 << std::endl;
  if (load2 && !args[1]->IsConst()) ofs << "  load " << args[1]->GetID() << " " << reg2 << std::endl;
  if (load3 && !args[2]->IsConst()) ofs << "  load " << args[2]->GetID() << " " << reg3 << std::endl;

  // If there is an instruction, print it and all its arguments.
  if (inst == "ar_get_idx") {            // *******************************************************
    ofs << "  add " << reg1 << " 1 " << reg4 << std::endl;
    ofs << "  add " << reg4 << " "
        << (args[1]->IsConst() ? args[1]->AsString() : reg2)
        << " " << reg4 << std::endl;
    ofs << "  load " << reg4 << " " << reg3 << std::endl;

  } else if (inst == "ar_set_idx") {     // *******************************************************
    ofs << "  add " << reg1 << " 1 " << reg4 << std::endl;
    if (args[1]->IsConst()) ofs << "  add " << reg4 << " " << args[1]->AsString() << " " << reg4 << std::endl;
    else                    ofs << "  add " << reg4 << " " << reg2 << " " << reg4 << std::endl;
    if (args[2]->IsConst()) ofs << "  store " << args[2]->AsString() << " " << reg4 << std::endl;
    else                    ofs << "  store " << reg3 << " " << reg4 << std::endl;

  } else if (inst == "ar_get_size") {    // *******************************************************
    ofs << "  load " << reg1 << " " << reg2 << std::endl;

  } else if (inst == "ar_set_size") {    // *******************************************************
    static int label_id = 0;
    std::stringstream do_copy_label, start_label, end_label;
    do_copy_label << "ar_resize_do_copy_" << label_id++;
    start_label << "ar_resize_start_" << label_id++;
    end_label << "ar_resize_end_" << label_id++;

    if (args[1]->IsConst()) reg2 = args[1]->AsString();

    // Start by calculating old_array_size in reg3
    ofs << "  val_copy 0 " << reg3 << "                       # Default old array size to 0 if uninitialized." << std::endl;
    ofs << "  jump_if_0 " << reg1 << " " << do_copy_label.str() << "    # Leave 0 size (nothing to copy) for uninitialized arrays." << std::endl;  // Jump if original array is uninitialized
    ofs << "  load " << reg1 << " " << reg3 << "                        # Load old array size into " << reg3 << std::endl;

    // Test if old_array_size (reg3) >= new_array_size (reg2)
    ofs << "  test_gtr " << reg2 << " " << reg3 << " " << reg4 << "               # " << reg4 << " = new_size > old_size?" << std::endl;
    ofs << "  jump_if_n0 " << reg4 << " " << do_copy_label.str() << "   # Jump to array copy if new size is bigger than old size." << std::endl;  // If not, proceed to move...
    ofs << "  store " << reg2 << " " << reg1 << "                       # Otherwise, replace old size w/ new size.  Done." << std::endl;
    ofs << "  jump " << end_label.str() << "                  # Skip copying contents." << std::endl;

    // If we made it here, we need to copy the array and have original size in reg3 and new size in reg2
    ofs << do_copy_label.str() << ":" << std::endl;

    // Set up memory for the new array.
    ofs << "  load 0 " << reg4 << "                           # Set " << reg4 << " = free mem position" << std::endl;
    ofs << "  store " << reg4 << " " << args[0]->GetID() << "                          # Set indirect pointer to new mem pos." << std::endl;
    ofs << "  store " << reg2 << " " << reg4 << "                       # Store new size at new array start" << std::endl;
    ofs << "  add " << reg4 << " 1 " << reg5 << "                       # Set " << reg5 << " = first pos. in new array" << std::endl;
    ofs << "  add " << reg5 << " " << reg2 << " " << reg5 << "                    # Set " << reg5 << " = new free mem position" << std::endl;
    ofs << "  store " << reg5 << " 0                          # Store new free memory at pos. zero" << std::endl;

    // Figure out where to stop copying in E
    ofs << "  add " << reg1 << " " << reg3 << " " << reg5 << "                    # Set " << reg5 << " = the last index to be copied" << std::endl;

    // Copy the array over from A to D.
    ofs << start_label.str() << ":" << std::endl;
    ofs << "  add " << reg1 << " 1 " << reg1 << "                       # Increment pointer for FROM array" << std::endl;
    ofs << "  add " << reg4 << " 1 " << reg4 << "                       # Increment pointer for TO array" << std::endl;
    ofs << "  test_gtr " << reg1 << " " << reg5 << " " << reg6 << "               # If we are done copying, jump to end of loop" << std::endl;
    ofs << "  jump_if_n0 " << reg6 << " " << end_label.str() << std::endl;
    ofs << "  mem_copy " << reg1 << " " << reg4 << "                    # Copy the current index." << std::endl;
    ofs << "  jump " << start_label.str() << std::endl;
    ofs << end_label.str() << ":" << std::endl;

  } else if (inst == "ar_copy") {     // *******************************************************
    static int label_id = 0;
    std::stringstream do_copy_label, start_label, end_label;
    do_copy_label << "ar_do_copy_" << label_id++;
    start_label << "ar_copy_start_" << label_id++;
    end_label << "ar_copy_end_" << label_id++;

    // reg1 holds the pointer the array to copy from.  If it's zero, set array2 to zero and stop.
    ofs << "  jump_if_n0 " << reg1 << " " << do_copy_label.str() << "          # Jump if we actually have something to copy." << std::endl;
    ofs << "  val_copy 0 " << reg2 << "                             # Set indirect pointer to new mem pos." << std::endl;
    ofs << "  jump " << end_label.str() << std::endl;

    // If we made it here, we need to copy the array.
    ofs << do_copy_label.str() << ":" << std::endl;

    // Set up memory for the new array.
    std::string reg_to_mem = reg4;
    std::string reg_ar_size = reg5;
    std::string reg_to_end = reg6;
    ofs << "  load 0 " << reg_to_mem << "                           # Set " << reg_to_mem << " = free mem position" << std::endl;
    ofs << "  val_copy " << reg_to_mem << " " << reg2 << "                          # Set indirect pointer to new mem pos." << std::endl;
    ofs << "  load " << reg1 << " " << reg_ar_size << "                        # Set " << reg_ar_size << " = Array size." << std::endl;
    ofs << "  add " << reg_to_mem << " 1 " << reg_to_end << "                       # Set " << reg_to_end << " = first pos. in new array" << std::endl;
    ofs << "  add " << reg_to_end << " " << reg_ar_size << " " << reg_to_end << "                    # Set " << reg_to_end << " = new free mem position" << std::endl;
    ofs << "  store " << reg_to_end << " 0                          # Store new free memory at pos. zero" << std::endl;

    // Copy the array over from A to B; increment until B==D
    ofs << start_label.str() << ":" << std::endl;
    std::string reg_test_done = reg7;
    ofs << "  test_equ " << reg_to_mem << " " << reg_to_end << " " << reg_test_done << "               # If we are done copying, jump to end of loop" << std::endl;
    ofs << "  jump_if_n0 " << reg_test_done << " " << end_label.str() << std::endl;
    ofs << "  mem_copy " << reg1 << " " << reg_to_mem << "                    # Copy the current index." << std::endl;
    ofs << "  add " << reg1 << " 1 " << reg1 << "                       # Increment pointer for FROM array" << std::endl;
    ofs << "  add " << reg_to_mem << " 1 " << reg_to_mem << "                       # Increment pointer for TO array" << std::endl;
    ofs << "  jump " << start_label.str() << std::endl;
    ofs << end_label.str() << ":" << std::endl;

  } else if (inst != "") {
    ofs << "  " << inst << " ";
    if (args.size() >= 1) {
      if (args[0]->IsConst()) ofs << args[0]->AsString() << " ";
      else ofs << reg1 << " ";
    }
    if (args.size() >= 2) {
      if (args[1]->IsConst()) ofs << args[1]->AsString() << " ";
      else ofs << reg2 << " ";
    }
    if (args.size() >= 3) {
      if (args[2]->IsConst()) ofs << args[2]->AsString() << " ";
      else ofs << reg3 << " ";
    }
    ofs << std::endl;
  }


  // Store any relevant values.
  if (store1) ofs << "  store " << reg1 << " " << args[0]->GetID() << std::endl;
  if (store2) ofs << "  store " << reg2 << " " << args[1]->GetID() << std::endl;
  if (store3) ofs << "  store " << reg3 << " " << args[2]->GetID() << std::endl;
}


IC_Entry& IC_Array::AddLabel(std::string label_id, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry();
  new_entry->SetLabel(label_id);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}


// This is a quick way to add scalar/array/none args, with all needed error checking.
void IC_Array::AddArg(IC_Entry * entry, int in_arg, ArgType::type expected_type)
{
  if (in_arg > max_id) max_id = in_arg; // Keep track of the maximum argument found.

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
      std::cerr << "INTERNAL ERROR: Insufficient arguments provided for inst '"
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


void IC_Array::AddArg(IC_Entry * entry, const std::string & in_arg, ArgType::type expected_type)
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
IC_Entry& IC_Array::Add(std::string inst_name, int arg1, int arg2, int arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, int arg1, int arg2, std::string arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, int arg1, std::string arg2, int arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, int arg1, std::string arg2, std::string arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, std::string arg1, int arg2, int arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, std::string arg1, int arg2, std::string arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, std::string arg1, std::string arg2, int arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}

IC_Entry& IC_Array::Add(std::string inst_name, std::string arg1, std::string arg2, std::string arg3, std::string cmt)
{
  IC_Entry * new_entry = new IC_Entry(inst_name);
  if (arg_type_map.find(inst_name) == arg_type_map.end()) {
    std::cerr << "INTERNAL ERROR: Unknown instruction '" << inst_name << "'." << std::endl;
  }
  std::vector<ArgType::type> & arg_types = arg_type_map[inst_name];
  AddArg(new_entry, arg1, arg_types[0]);
  AddArg(new_entry, arg2, arg_types[1]);
  AddArg(new_entry, arg3, arg_types[2]);
  new_entry->SetComment(cmt);
  ic_array.push_back(new_entry);
  return *new_entry;
}


void IC_Array::PrintIC(std::ostream & ofs)
{
  ofs << "# TubeIC ouput from checkpoint compiler." << std::endl;
  for (int i = 0; i < (int) ic_array.size(); i++) {
    ic_array[i]->PrintIC(ofs);
  }
}

void IC_Array::PrintTubeCode(std::ostream & ofs)
{
  ofs << "# Tubecode Assembly ouput from checkpoint compiler." << std::endl;
  ofs << "  store " << max_id+1 << " 0                         # Store next free memory at 0" << std::endl;

  // Convert each line of intermediate code, one at a time.
  for (int i = 0; i < (int) ic_array.size(); i++) {
    ic_array[i]->PrintTubeCode(ofs);
  }
}

