#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case INT: return "int";
    case CHAR: return "char";
    case CHAR_ARRAY: return "array(char)";
    case INT_ARRAY: return "array(int)";
    //case CHAR_ARRAY_IDX: return "array(char)[]";
    //case INT_ARRAY_IDX: return "array(char)[]";
    };
    return "unknown";
  }

  bool IsArray(int type)
  {
    if(type == CHAR_ARRAY || type == INT_ARRAY )
    {
        return true;
    }
    else
    {
      return false;
    }
  }

  int InnerType(int type)
  {
    if(type == INT_ARRAY)
    {
      return INT;
    }
    else if (type == CHAR_ARRAY)
    {
      return CHAR;
    }
    else
    {
      return VOID;
    }
  }

};

