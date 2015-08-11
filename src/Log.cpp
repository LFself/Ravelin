#include <Ravelin/Log.h>

using namespace Ravelin;

std::ofstream OutputToFile::stream;

void OutputToFile::output(const std::string& msg)
{
  if (!stream.is_open())
  {
    std::ofstream stderr_stream("/dev/stderr", std::ofstream::app);
    stderr_stream << msg << std::flush;
  }
  else
    stream << msg << std::flush;
}

