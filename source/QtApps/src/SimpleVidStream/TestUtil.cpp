#include <TestUtil.hpp>
#include <algorithm>

void playwithstrings(std::map<int, std::string>& stringmap)
{
  using namespace Util;

  for (auto itr = stringmap.begin(); itr != stringmap.end(); itr++)
  {
    Utility::to_lower(itr->second);
  }
}
