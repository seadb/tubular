#ifndef TYPE_INFO_H
#define TYPE_INFO_H

#include <string>

namespace Type {
  enum TypeNames { VOID=0, INT, CHAR, CHAR_ARRAY, INT_ARRAY, CHAR_ARRAY_IDX, INT_ARRAY_IDX };
  
  std::string AsString(int type); // Convert the internal type to a string like "int"

  // Write addtional Type helper methods here, such as bool IsArray(int type) or
  // int InnerType(int type)  to identify what type is inside an array.
};

#endif
