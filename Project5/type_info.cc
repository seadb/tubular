#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case INT: return "int";
    case CHAR: return "char";
    case CHAR_ARRAY: return "array(char)";
    case INT_ARRAY: return "array(int)";
    case CHAR_ARRAY_IDX: return "array(char)[]";
    case INT_ARRAY_IDX: return "array(char)[]";
    };
    return "unknown";
  }
};
