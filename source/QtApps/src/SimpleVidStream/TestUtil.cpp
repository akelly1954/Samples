#include <TestUtil.hpp>
#include <iostream>
#include <algorithm>

void playwithstrings(std::map<int, std::string>& stringmap, std::shared_ptr<Log::Logger> loggerp)
{
  using namespace Util;

  if (loggerp == nullptr)
  {
    std::cerr << "WARNING...  logger pointer is null" << std::endl;
  }
  for (auto itr = stringmap.begin(); itr != stringmap.end(); itr++)
  {
    if(loggerp) loggerp->debug() << "String before: " << itr->second;
    Utility::to_lower(itr->second);
    if(loggerp) loggerp->debug() << "String after: " << itr->second;
  }
}
