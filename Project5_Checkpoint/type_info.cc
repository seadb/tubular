#include "type_info.h"

namespace Type {
  std::string AsString(int type) {
    switch (type) {
    case VOID: return "void";
    case INT: return "int";
    case CHAR: return "char";
    case CHAR_ARRAY: return "char_array";
    case INT_ARRAY: return "int_array";
    };
    return "unknown";
  }
};
