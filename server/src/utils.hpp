#pragma once

#include <string>
#ifdef _MSC_VER
#include <cstdlib>
#else
#include <byteswap.h>
#endif

template <typename StringT, typename StringViewT, bool case_sensitive = true>
inline constexpr bool ends_with_impl(const StringT & str, const StringViewT & suffix)
{
  const size_t str_len = std::size(str), suffix_len = size(suffix);
  if(str_len < suffix_len)
    return false;

  constexpr auto transformer = case_sensitive ? ::tolower : [](int x) { return x; };
  for(size_t i = 0; i < suffix_len; ++i)
    if(transformer(static_cast<unsigned char>(str[str_len - suffix_len + i])) !=
       transformer(static_cast<unsigned char>(suffix[i])))
      return false;
  return true;
}

template <typename StringT, typename StringViewT>
inline constexpr bool ends_with(const StringT & str, const StringViewT & suffix)
{
  return ends_with_impl<StringT, StringViewT, true>(str, suffix);
}

template <typename StringT, typename StringViewT>
inline constexpr bool iends_with(const StringT & str, const StringViewT & suffix)
{
  return ends_with_impl<StringT, StringViewT, false>(str, suffix);
}

template <typename T, typename Y>
inline T byteswap(const Y value)
{
  static_assert(false, "invalid type provided");
}

template <>
inline unsigned long byteswap(const unsigned long value)
{
#ifdef _MSC_VER
  return _byteswap_ulong(value);
#else
#error "implement me";
#endif
}

template <>
inline uint32_t byteswap(const uint32_t value)
{
  static_assert(sizeof uint32_t == sizeof(unsigned long));
#ifdef _MSC_VER
  return _byteswap_ulong(value);
#else
#error "implement me";
#endif
}

template <>
inline uint32_t byteswap(const char * value)
{
#ifdef _MSC_VER
  return _byteswap_ulong(*reinterpret_cast<const uint32_t *>(value));
#else
#error "implement me";
#endif
}

template <>
inline uint64_t byteswap(const uint64_t value)
{
#ifdef _MSC_VER
  return _byteswap_uint64(value);
#else
#error "implement me";
#endif
}

template <>
inline uint64_t byteswap(const char * value)
{
#ifdef _MSC_VER
  return _byteswap_uint64(*reinterpret_cast<const uint64_t *>(value));
#else
#error "implement me";
#endif
}

template <>
inline unsigned short byteswap(const unsigned short value)
{
#ifdef _MSC_VER
  return _byteswap_ushort(value);
#else
#error "implement me";
#endif
}

template <>
inline unsigned short byteswap(const char * value)
{
#ifdef _MSC_VER
  return _byteswap_ushort(*reinterpret_cast<const unsigned short *>(value));
#else
#error "implement me";
#endif
}

inline std::string utf16_to_utf8(const char * bytes, const size_t nbytes)
{
  std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
  return convert.to_bytes(reinterpret_cast<const int16_t *>(bytes), reinterpret_cast<const int16_t *>(bytes + nbytes));
}

template <size_t NExtensions>
std::vector<fs::path> gather_files(const fs::path &                                  root_dir,
                                   const std::array<std::string_view, NExtensions> & included_extensions)
{
  std::vector<fs::path>  result;
  std::array<char, 1024> stack_buffer;
  for(auto & it : fs::recursive_directory_iterator{root_dir})
  {
    auto & path = it.path();
    if(!fs::is_regular_file(path))
      continue;
#if _MSC_VER >= 1915
    stack_state           ss{stack_buffer.data(), size(stack_buffer), size(stack_buffer)};
    stack_allocator<char> allocator{ss};
    auto                  utf8path =
      fs::_Convert_wide_to_narrow<std::char_traits<char>>(__std_code_page::_Utf8, path.native(), allocator);
#else
    static_assert(sizeof(path.native()[0]) == sizeof(char), "implement me ^_^'");
    auto utf8path = path.u8string();
#endif
    if(!std::any_of(begin(included_extensions), end(included_extensions), [&utf8path](const std::string_view & ext) {
         return iends_with(utf8path, ext);
       }))
      continue;
    result.emplace_back(fs::u8path(utf8path.data() + size(root_dir.native())));
  }
  return result;
}


template <size_t N>
std::ostream & operator<<(std::ostream & os, const std::array<char, N> & a)
{
  os.write(a.data(), N);
  return os;
}