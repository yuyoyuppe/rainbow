#include "pch.hpp"

#include "stack_allocator.hpp"
#include "utils.hpp"
#include "audio_metadata.hpp"



int main()
{
#ifdef _MSC_VER
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif

  using namespace std::literals;

  std::array<std::string_view, 2> supported_extensions = {/*u8".ogg" */u8".aac", u8".mp3"};

  const auto root_dir = u8R"r(D:\Music\)r";
  auto all_files = gather_files(root_dir, supported_extensions);
  size_t nFiles = 0;
  for(const auto & f : all_files)
  {
    const auto full_path = root_dir / f;
    if(auto md = read_metadata(full_path))
      ++nFiles;
    else
      std::cout << "couldn't read metadata " << f.u8string() << std::endl;
  }
  std::cout << nFiles << " files read\n";
  std::string playlist;
  playlist.reserve(1 << 22);

  return 0;
}
