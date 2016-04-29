#include "type_info.h"

namespace Type {

  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case INT: return "int";
    case CHAR: return "char";
    case INT_ARRAY: return "array(int)";
    case CHAR_ARRAY: return "array(char)";
    };
    return "unknown";
  }

  int InternalType(int type) {
    if (type == INT_ARRAY) return INT;
    if (type == CHAR_ARRAY) return CHAR;
    return VOID;
  }

  bool IsArray(int type) {
    if (type == CHAR_ARRAY || type == INT_ARRAY) return true;
    return false;
  }

};
