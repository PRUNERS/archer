#include "archer/Util.h"

void split(std::vector<std::string> *tokens, char *str, std::string split_value)
{
  char *pch = strtok(str, split_value.c_str());
  while (pch != NULL)
  {
    tokens->push_back(std::string(pch));
    pch = strtok(NULL, split_value.c_str());
  }
}
