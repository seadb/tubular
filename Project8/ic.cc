#include "ic.h"
/******************************************
 * BEGIN ICEntry
 *****************************************/

bool first_run = true;
int label_num = 0;

void ICEntry::PrintIC(std::ostream & ofs)
{
  std::stringstream out_line;

  // If there is a label, include it in the output.
  if (label != "") { out_line << label << ": "; }
  else { out_line << "  "; }
  out_line << " blockid: " << mBlockID << " ";
  // If there is an instruction, print it and all its arguments.
  if (mInst != "") {
    out_line << mInst << " ";
    for (int i = 0; i < (int) mArgs.size(); i++) {
      out_line << mArgs[i]->AsString() << " ";
    }
  }

  // If there is a comment, print it!
  if (comment != "") {
    while (out_line.str().size() < 40) out_line << " "; // Align comments for easy reading.
    out_line << "# " << comment;
  }

  ofs << out_line.str() << std::endl;
}

bool ICEntry::OptimizeIC()
{
    variableTracker * tracker;
    bool progress = false;
    for (int i =0; i < GetNumArgs(); i++ ) {
        if (mDelete) {
            //check if entry should be deleted
        }
        if (mArgs[i]->IsScalar()) {
            std::string name = mArgs[i]->AsString();
            tracker = mArray->FindVariable(name);
            if (tracker != NULL) {
                if (tracker->replace != "") {
                    delete mArgs[i];
                    mArgs[i] = new ICArg_Const(tracker->replace);
                    tracker->usedCount = tracker->usedCount - 1;
                    progress = true;
                }
                else if (mSimplify && tracker->simplify != "") {
                    mInst = "val_copy";
                    mArgs.erase(mArgs.begin()+1);
                    mArgs.erase(mArgs.begin());
                    std::string simp = tracker->simplify;
                    if(simp[0] != 's')
                        mArgs.insert(mArgs.begin(),new ICArg_Const(simp));
                    else {// == 's'
                        std::string arg = simp.substr(1,simp.size()-1);
                        int id = atoi(arg.c_str());
                        mArgs.insert(mArgs.begin(),new ICArg_VarScalar(id));
                    }

                    progress = true;
                    tracker->simplify = "";
                    mSimplify = false;
                    //std::cout << GetArg(0) << GetArg(1) << std::endl;
                }

            }
        }
    }
    return progress;
}
void ICEntry::TrackVariables()
{

    for(int i =0; i < GetNumArgs(); i++ )
    {
        if(mArgs[i]->IsScalar())
        {
            std::string name = mArgs[i]->AsString();
            variableTracker * tracker = mArray->FindVariable(name);
            if (tracker != NULL)
            {   //variable already has a tracker
                tracker->lastLine = mLineNumber;
                tracker->usedCount = tracker->usedCount + 1;
                tracker->lastBlock = mBlockID;
                if (tracker->firstBlock == tracker->lastBlock)
                    tracker->local = true;

                else tracker->local = false;
                if ( mInst == "val_copy" && i==1 )
                    tracker->SSA = false;
            }

            else
            {   //new tracker
                tracker = new variableTracker;
                tracker->replace = "";
                tracker->simplify  = "";
                mArray->AddVariable(name, tracker);

                // first used here
                tracker->firstLine = mLineNumber;
                tracker->firstBlock = mBlockID;

                // last used here
                tracker->lastLine = mLineNumber;
                tracker->lastBlock = mBlockID;
                tracker->usedCount = 0;
                tracker->local = true; //lastBlock = firstBlock
                tracker->SSA = false;

                if(mInst == "val_copy" && i==1 )
                    tracker->SSA = true;

            }
        }
    }
    mArray->SetFirst(false);
}

/*void ICEntry::EliminateDeadCode()
{
    if (mInst == "val_copy")
    {
        variableTracker * tracker = mArray->FindVariable(mArgs[1]->AsString());
    }
    else if(mInst == "add" || mInst == "sub" || mInst == "mult" || mInst == "div"
            || mInst == "mod" )
    {

    }
        if ( tracker == NULL)
        {
            std::string errString = "argument not in tracker: ";
            errString += mArgs[1]->AsString();
            std::cout << errString;
            exit(1);
        }
        else
        {
            if(tracker->SSA)
            {
                return True;
            }
        }
    }
}*/
void ICEntry::PrintTC(std::ostream & ofs)
{
    variableTracker * tracker ;
  if (first_run) {
    ofs << "  store 20000 0" << std::endl;
    ofs << "  val_copy 10000 regH" << std::endl;
    first_run = false;
  }

  // If there is a label, include it in the output.
  if (label != "") {
    ofs << label << ": " << std::endl << "  nop" << std::endl;
  }

  if (mInst != "") {
    // Print intermediate code as comment
    std::stringstream out_line;
    out_line << "# " << mInst << " ";
    for (int i = 0; i < (int) mArgs.size(); i++) {
      out_line << mArgs[i]->AsString() << " ";
    }
    ofs << out_line.str() << std::endl;

    if(mInst == "val_copy"){
      //if(mArray->GetReg("regA") == (std::string) mArgs[0]->GetID())
      //{
      //}
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      //mArgs[0]->SetReg('A');
      //ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
     // mArray->AddVariable("regA", tracker);
     //mArray->AddReg(mArgs[0]->GetID(),"regA");

      ofs << "  val_copy " << mArgs[0]->AsAssemblyString() << " regB" << std::endl;
      //mArray->AddReg( mArgs[0]->AsAssemblyString(), "regB");
      mArgs[1]->AssemblyWrite(ofs, mArgs[1]->GetID(), 'B');
      //mArray->AddReg(mArgs[1]->GetID(),);
      //ofs << "  store regB " << mArgs[1]->GetID() << std::endl;
    }
      //mArgs[0]->AssemblyWrite(ofs, mArgs[0]->GetID(), 'A');
      //ofs << "  store regA " << mArgs[0]->GetID() << std::endl;

      //mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      //mArgs[0]->SetReg('A');
      //ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;

    else if(mInst == "add" || mInst == "sub" || mInst == "mult" ||
            mInst == "div" || mInst == "mod" || mInst == "test_less" ||
            mInst == "test_gtr" || mInst == "test_equ" ||
            mInst == "test_nequ" || mInst == "test_lte" ||
            mInst == "test_lte" || mInst == "test_gte") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      mArgs[1]->AssemblyRead(ofs, mArgs[1]->GetID(), 'B');
      ofs << "  " << mInst << " " << mArgs[0]->AsAssemblyString() << " ";
      ofs <<  mArgs[1]->AsAssemblyString() << " regC" << std::endl;
      mArgs[2]->AssemblyWrite(ofs, mArgs[2]->GetID(), 'C');
    }
    else if(mInst == "jump" || mInst == "out_int" || mInst == "out_char") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      ofs << "  " << mInst << " " << mArgs[0]->AsAssemblyString() << " " << std::endl;
    }
    else if(mInst == "jump_if_0" || mInst == "jump_if_n0") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      mArgs[1]->AssemblyRead(ofs, mArgs[1]->GetID(), 'A');
      ofs << "  " << mInst << " " << mArgs[0]->AsAssemblyString() << " ";
      ofs <<  mArgs[1]->AsAssemblyString() << std::endl;
    }
    else if(mInst == "random") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      ofs << "  " << mInst << " " << mArgs[0]->AsAssemblyString() << " ";
      ofs << mArgs[1]->AsAssemblyString() << " regB" << std::endl;
      mArgs[1]->AssemblyWrite(ofs, mArgs[1]->GetID(), 'B');
    }
    else if(mInst == "nop") {
      ofs << "  nop" << std::endl;
    }
    else if(mInst == "push") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      ofs << "  store regA regH" << std::endl;
      ofs << "  add 1 regH regH" << std::endl;
    }
    else if(mInst == "pop") {
      ofs << "  load regH regA" << std::endl;
      mArgs[0]->AssemblyWrite(ofs, mArgs[0]->GetID(), 'A');
      //ofs << "  store regA " << mArgs[0]->GetID() << std::endl;

      ofs << "  sub regH 1 regH" << std::endl;
    }
    else if(mInst == "ar_push") {
      mArgs[0]->AssemblyRead(ofs, mArgs[0]->GetID(), 'A');
      //mArgs[0]->SetReg('A');
      //ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;

      ofs << "  load regA regB" << std::endl;
      ofs << "  val_copy 0 regC" << std::endl;
      ofs << "ar_push_start" << label_num << ":" << std::endl;
      ofs << "  comp_equ regC regB regD" << std::endl;
      ofs << "  jump_if_n0 regD ar_push_end" << label_num << std::endl;
      ofs << "  add regA regC regE" << std::endl;
      ofs << "  mem_copy regE regH" << std::endl;
      ofs << "  add regC 1 regC" << std::endl;
      ofs << "  add regH 1 regH" << std::endl;
      ofs << "  jump ar_push_start" << label_num << std::endl;
      ofs << "ar_push_end" << label_num << std::endl;

      // Store size in last memory position
      ofs << "  mem_copy regB regH" << std::endl;
      ofs << "  add regH 1 regH" << std::endl;
    }
    // Assumes argument is large enough to fit popped array!
    else if(mInst == "ar_pop") {
      mArgs[0]->AssemblyWrite(ofs, mArgs[0]->GetID(), 'A');
      ofs << "  add regA regH regB" << std::endl;
      ofs << "  val_copy regA regC" << std::endl;
      ofs << "ar_pop_start" << label_num << ":" << std::endl;
      ofs << "  comp_equ regB regC regD" << std::endl;
      ofs << "  jump_if_n0 regD ar_pop_end" << label_num << std::endl;
      ofs << "  mem_copy regH regC" << std::endl;
      ofs << "  sub regH 1 regH" << std::endl;
      ofs << "  add regC 1 regC" << std::endl;
      ofs << "  jump ar_pop_start" << label_num << std::endl;
      ofs << "ar_pop_end" << label_num << std::endl;
    }
    else if(mInst == "ar_get_idx" || mInst == "ar_set_idx") {
      ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
      mArgs[1]->AssemblyRead(ofs, mArgs[1]->GetID(), 'B');
      ofs << "  add regA 1 regA" << std::endl;
      ofs << "  add regA " << mArgs[1]->AsAssemblyString() << " regA" << std::endl;
      if(mInst == "ar_get_idx") {
        ofs << "  mem_copy regA " << mArgs[2]->GetID() << std::endl;
      }
      else {
        if(mArgs[2]->IsScalar()) {
          ofs << "  mem_copy " << mArgs[2]->GetID() << " regA" << std::endl;
        }
        else {
          ofs << "  val_copy " << mArgs[2]->AsString() << " regC" << std::endl;
          ofs << "  store regC regA" << std::endl;
        }
      }
    }
    else if(mInst == "ar_get_size") {
      ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
      ofs << "  mem_copy regA " << mArgs[1]->GetID() << std::endl;
    }
    else if(mInst == "ar_set_size") {
      ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
      if(mArgs[1]->IsScalar()) {
        ofs << "  load " << mArgs[1]->GetID() << " regB" << std::endl;
      }
      else {
        ofs << "  val_copy " << mArgs[1]->AsString() << " regB" << std::endl;
      }
      ofs << "  jump_if_0 regA do_resize" << label_num << std::endl;
      ofs << "  load regA regC" << std::endl;
      ofs << "  store regB regA" << std::endl;
      ofs << "  test_lte regB regC regD" << std::endl;
      ofs << "  jump_if_n0 regD resize_end_" << label_num + 1 << std::endl;
      ofs << "do_resize" << label_num << ":" << std::endl;
      ofs << "  load 0 regD" << std::endl;
      ofs << "  add regD 1 regE" << std::endl;
      ofs << "  add regE regB regE" << std::endl;
      ofs << "  store regE 0" << std::endl;
      ofs << "  store regD " << mArgs[0]->GetID() << std::endl;
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
      // Set size
      ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
      ofs << "  load regA regB" << std::endl;
      ofs << "  load " << mArgs[1]->GetID() << " regA" << std::endl;
      ofs << "  jump_if_0 regA do_resize" << label_num << std::endl;
      ofs << "  load regA regC" << std::endl;
      ofs << "  store regB regA" << std::endl;
      ofs << "  test_lte regB regC regD" << std::endl;
      ofs << "  jump_if_n0 regD resize_end_" << label_num + 1 << std::endl;
      ofs << "do_resize" << label_num << ":" << std::endl;
      ofs << "  load 0 regD" << std::endl;
      ofs << "  add regD 1 regE" << std::endl;
      ofs << "  add regE regB regE" << std::endl;
      ofs << "  store regE 0" << std::endl;
      ofs << "  store regD " << mArgs[1]->GetID() << std::endl;
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

      // Copy contents
      ofs << "  load " << mArgs[0]->GetID() << " regA" << std::endl;
      ofs << "  load " << mArgs[1]->GetID() << " regB" << std::endl;
      ofs << "  load regA regC" << std::endl;
      //ofs << "  out_int regC" << std::endl;
      ofs << "  add 1 regA regA" << std::endl;
      ofs << "  add 1 regB regB" << std::endl;
      ofs << "  val_copy 0 regD" << std::endl;
      ofs << "copy_start" << label_num << ":" << std::endl;
      //ofs << "  out_int regD" << std::endl;
      //ofs << "  out_char '\\n'" << std::endl;
      ofs << "  test_gte regD regC regE" << std::endl;
      ofs << "  jump_if_n0 regE copy_end" << label_num << std::endl;
      ofs << "  mem_copy regA regB" << std::endl;
      ofs << "  add 1 regA regA" << std::endl;
      ofs << "  add 1 regB regB" << std::endl;
      ofs << "  add 1 regD regD" << std::endl;
      ofs << "  jump copy_start" << label_num << std::endl;
      ofs << "copy_end" << label_num << ":" << std::endl;
      ofs << "  nop" << std::endl;
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
  ofs << '\n';
}

void ICArray::OptimizeIC()
{
  // count block id
  std::stringstream ss;
  //std::cout << "hello" << std::endl;
  for (int i = 0; i < (int) mICArray.size(); i++)
  {
      if (mICArray[i]->GetLabel() != ""
              || mICArray[i]->GetInstName() == "jump_if_0"
              || mICArray[i]->GetInstName() == "jump_if_n0" )
          mICArray[i]->SetBlockID(IncBlockID());
      else
          mICArray[i]->SetBlockID(mBlockID);

      //printf("Line Number: %d\n", i);
      mICArray[i]->SetLineNumber(i+1);
      mICArray[i]->TrackVariables();
  }

  bool progress = true;
  while(progress) {
    progress = false;
    for(int j = 0; j < (int) mICArray.size(); j++) { // for each entry in
      ICEntry * entry = mICArray[j];                 // mArray
      std::string inst = entry->GetInstName();
      //ss.str(""); ss << j;
      //std::cout << "j: " << ss.str() << std::endl;

      //Constant Propogation and Dead Code Elimination
      if (inst == "val_copy") {

        std::string arg0 = entry->GetArg(0);
        variableTracker * arg0Tracker = FindVariable(arg0); //var copied into
        std::string arg1 = entry->GetArg(1);
        variableTracker * arg1Tracker = FindVariable(arg1);
          //Constant Propagation
          if (arg1Tracker->SSA)
          {
              if (arg0[0] != 's') { //constant value
// std::cout << "const prop" << inst << " " << arg0 << " " << arg1 << std::endl;
              arg1Tracker->replace = arg0;
              entry->SetDelete(true);
              progress = true; break;
          }}

      }
      else if (inst == "add" || inst == "sub" || inst == "mult" || inst == "div"
              || inst == "mod" || inst == "test_less" || inst == "test_gtr"
              || inst == "test_equ" || inst == "test_nequ" || inst == "test_lte"
              || inst == "test_gte") {
          std::string arg0 = entry->GetArg(0);
          variableTracker * arg0Tracker = FindVariable(arg0); //augend
          std::string arg1 = entry->GetArg(1);
          variableTracker * arg1Tracker = FindVariable(arg1); //addend
          std::string arg2 = entry->GetArg(2);
          variableTracker * arg2Tracker = FindVariable(arg2); //target var
          //Eliminate Dead Code
          if (arg2Tracker->firstLine == arg2Tracker->lastLine) {
              entry->SetDelete(true);
              //entry->OptimizeIC();
              progress = true;
              //continue;
              break;
          }
          if (arg0[0] != 's' && arg1[0] != 's'
                  && arg0[0] != 'a' && arg1[0] != 'a')
          {
              entry->SetSimplify(true);
              int a0 = atoi(arg0.c_str());
              int a1 = atoi(arg1.c_str());
              // algebraic simplification
              if (inst == "add")
              {
                int result = a0 + a1; ss.str(""); ss << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "sub")
              {
                int result = a0 - a1; ss.str(""); ss << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "mult")
              {
                int result = a0 * a1;ss.str(""); ss << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "div")
              {
                int result = a0 / a1; ss.str("");ss << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "mod")
              {
                int result = a0 % a1; ss.str("");ss << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "test_less") {
                bool result = a0 < a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "test_gtr") {
                bool result = a0 > a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "test_equ") {
                bool result = a0 == a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst != "test_nequ") {
                bool result = a0 == a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "test_lte") {
                bool result = a0 <= a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              else if (inst == "test_gte") {
                bool result = a0 >= a1; ss.str("");
                ss << std::noboolalpha << result;
                arg2Tracker->simplify = ss.str();
              }
              progress = true; break;
          }
          if (inst == "mult") {
            if (arg0[0] == 's' && arg1[0] != 's' && arg1[0] != 'a') {
                if (arg1 == "0") {
                    arg2Tracker->simplify = arg1; //answer always 0
                    entry->SetSimplify(true);
                    progress = true; break;
                }
                else if (arg1 == "1") {
                    arg2Tracker->simplify = arg0; //answer always a0
                    entry->SetSimplify(true);
                    progress = true; break;
                }
            }
            if(arg0[0] != 's' && arg1[0] == 's' && arg0[0] != 'a') {
                if(arg0 == "0") {
                    arg2Tracker->simplify = arg0; //answer always 0
                    entry->SetSimplify(true);
                    progress = true; break;
                }
                else if (arg0 == "1") {
                    arg2Tracker->simplify = arg1; //answer always a1
                    entry->SetSimplify(true);
                    progress = true; break;
                }
            }
          }
      }

      if (inst == "val_copy") {
        std::string arg0 = entry->GetArg(0);
        variableTracker * arg0Tracker = FindVariable(arg0); //var copied into
        std::string arg1 = entry->GetArg(1);
        variableTracker * arg1Tracker = FindVariable(arg1);

        //Variable Propagation
        //this only works if both args are scalar
        if (arg0[0] == 's' && arg1[1] == 's' ) {
            //std::cout << "inside first variable propagation check" << std::endl;
            int ZLL = arg0Tracker->lastLine;
            int OFL = arg1Tracker->firstLine;
            bool ZL = arg0Tracker->local;
            bool OL = arg1Tracker->local;
            if (ZLL == OFL && ZL && OL){
              //std::cout << inst << " " << arg0 << " " << arg1 << std::endl;
              arg1Tracker->replace = arg0; //replace arg1 with arg0
              entry->SetDelete(true);
              progress = true; break;
          }
        }
        //Eliminate Dead Code
          if (arg1Tracker->usedCount < 1) {
//  std::cout << "dead code el" << inst << " " << arg0 << " " << arg1 << std::endl;
              entry->SetDelete(true);
              progress = true; break;
          }


      }
      /*if (!progress) {
          bool temp = entry->OptimizeIC();
          if (temp) progress = true;
      }*/
    } // END FOR
    //mTrackerMap.clear();
    if(progress)
    {

    }
    for(int j = 0; j < (int) mICArray.size(); j++) {
      ICEntry * entry = mICArray[j];
      bool temp = entry->OptimizeIC();
      if(temp) progress = true;
      if (entry->GetDelete()){
          mICArray.erase(mICArray.begin()+j);
          progress = true;
      }
      //entry->TrackVariables();
    } // END FOR 2
  } // END WHILE
//
/*
for(std::map<std::string, variableTracker*>::const_iterator it
                = mTrackerMap.begin(); it != mTrackerMap.end(); ++it)
    {
        std::cout << it->first  << ": \n";
        ss.str(""); ss << it->second->firstLine;
        std::cout << "  First Line: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->lastLine;
        std::cout << "  Last Line: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->usedCount;
        std::cout << "  Used Count: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->firstBlock;
        std::cout << "  First Block: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->lastBlock;
        std::cout << "  Last Block: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->local;
        std::cout << "  Local: " << ss.str() << std::endl;
        ss.str(""); ss << it->second->SSA;
        std::cout << "  SSA: " << ss.str() << std::endl;
    }*/

} // END OptimizeIC
void ICArray::PrintTC(std::ostream & ofs)
{
  //ofs << "# Tubecode Assembly ouput from checkpoint compiler." << std::endl;
  //ofs << "  store " << max_id+1 << " 0                         # Store next free memory at 0" << std::endl;
  // Convert each line of intermediate code, one at a time.
  for (int i = 0; i < (int) mICArray.size(); i++) {
    mICArray[i]->PrintTC(ofs);
  }
}

