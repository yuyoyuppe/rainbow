#include "pch.hpp"
#include "utils.hpp"
#include "audio_metadata.hpp"

enum class id3v2_encoding { ASCII, UTF16 };
std::optional<SongMetaData> parse_id3v2(std::ifstream & f)
{
  std::vector<unsigned char> buffer;
  buffer.resize(7);
  f.read((char *)&buffer[0], size(buffer));

  const uint8_t  version_major = buffer[0], version_minor = buffer[1];
  const bool     v2_2                   = version_major < 3;
  std::bitset<8> flags                  = buffer[2];
  const bool     unsynchronisation      = flags.test(7);
  const bool     extended_header        = flags.test(6);
  const bool     experimental_indicator = flags.test(5);
  auto from_syncsafe = [](const unsigned char * d) -> uint32_t { return d[0] << 21 | d[1] << 14 | d[2] << 7 | d[3]; };
  const uint32_t tag_size = from_syncsafe(&buffer[3]);
  if(extended_header)
  {
    char header[10];
    f.read(header, 5);
    f.read(header, header[4] & 0x80 ? /*is crc data present*/ 10 : 6);
    // ignore the contents
  }
  auto parse_tag_header = [&from_syncsafe, v2_2](const char *& d) {
    std::array<char, 4> frameID{};
    memcpy(frameID.data(), d, 4);
    d += v2_2 ? 3 : 4;
    const auto frame_size = byteswap<uint32_t>(d) >> (v2_2 ? 8 : 0);
    d += v2_2 ? 3 : 4;
    if(!v2_2)
    {
      std::bitset<16> flags = *(uint16_t *)d;
      d += 2; // skip flags
    }
    return std::make_pair(frameID, frame_size);
  };
  SongMetaData      result;
  std::vector<char> frames(tag_size, '\0');
  f.read(&frames[0], tag_size);
  const char * d       = &frames[0];
  int64_t      advance = 0;
  while(advance < static_cast<int64_t>(size(frames)) && advance < tag_size && advance >= 0)
  {
    auto [frameID, frame_size] = parse_tag_header(d);
    if(frameID[0] == 'T')
    {
      if(v2_2)
        frameID[3] = 0;
      const auto encoding = *d++ == 1 ? id3v2_encoding::UTF16 : id3v2_encoding::ASCII;
      const bool is_utf16 = encoding == id3v2_encoding::UTF16;
      --frame_size;
      auto frameID_is = [&frameID, v2_2](const char id[4]) {
        uint32_t actual_id = *(uint32_t *)id;
        if(v2_2)
          actual_id &= 0x00ffffff;
        return *(uint32_t *)frameID.data() == actual_id;
      };
      auto get_data = [d, is_utf16, frame_size]() {
        return is_utf16 ? utf16_to_utf8(d, frame_size) : std::string(d, d + frame_size);
      };
      if(frameID_is("TALB"))
        result._album = get_data();
      else if(frameID_is("TIT2") || frameID_is("TT2"))
        result._title = get_data();
      else if(frameID_is("TYER"))
      {
        auto         data   = get_data();
        const char * p      = &data[0];
        const bool   hasBOM = is_utf16 && *(uint16_t *)p == 0xbbef && uint8_t(p[2]) == 0xbf;
        result._year        = static_cast<uint16_t>(std::atoi(hasBOM ? p + 3 : p));
      }
      else if(frameID_is("TPE1") || frameID_is("TP1"))
        result._artist = get_data();
    }
    d += frame_size;
    advance = d - &frames[0];
  }

  return result;
}

struct vorbis_field_parse_state
{
  const char * const _name;
  const size_t       _name_length;
  size_t             _current_char;
};

template <size_t... Lengths>
auto init_vorbis_parse_state(const char (&... field_names)[Lengths])
  -> std::array<vorbis_field_parse_state, sizeof...(Lengths)>
{
  return {(vorbis_field_parse_state{field_names, Lengths, 0})...};
}

std::optional<SongMetaData> parse_vorbis(std::ifstream & f)
{
  SongMetaData result;

  std::vector<char>        buffer;
  constexpr std::ptrdiff_t buffer_size = 4096; // totally guessing
  buffer.resize(buffer_size);
  f.read(&buffer[0], size(buffer));
  auto   parse_field_states = init_vorbis_parse_state("album=", "artist=", "date=", "title=");
  char * p                  = &buffer[0];
  size_t num_parsed_fields  = 0;
  while(++p - &buffer[0] < buffer_size)
  {
    for(size_t i = 0; i < size(parse_field_states); ++i)
    {
      if(parse_field_states[i]._name[parse_field_states[i]._current_char] == ::tolower(*p))
      {
        ++parse_field_states[i]._current_char;
        if(parse_field_states[i]._current_char + 1 == parse_field_states[i]._name_length)
        {
          parse_field_states[i]._current_char = 0;
          ++p;
          ++num_parsed_fields;
          const size_t field_value_length = strlen(p) - 1;
          switch(i)
          {
          case 0:
            result._album.assign(p, p + field_value_length);
            break;
          case 1:
            result._artist.assign(p, p + field_value_length);
            break;
          case 2:
            p[4]         = '\0';
            result._year = static_cast<uint16_t>(std::atoi(p));
            break;
          case 3:
            result._title.assign(p, p + field_value_length);
            break;
          default:
            assert(false);
          }
        }
      }
      else
        parse_field_states[i]._current_char = 0;
    }
  }
  return num_parsed_fields > 2 ? std::optional<SongMetaData>{result} : std::nullopt;
}

std::optional<SongMetaData> parse_id3v1(std::ifstream & f)
{
  constexpr size_t id3v1_magic = 0x00474154;
  constexpr size_t block_size  = 128;
  f.seekg(0, std::ios_base::end);
  const size_t block_pos = static_cast<std::streamoff>(f.tellg()) - block_size;
  f.seekg(block_pos);
  std::vector<char> block(block_size, '\0');
  f.read(block.data(), block_size);
  if(((*(uint32_t *)&block[0]) & 0x00ffffff) != id3v1_magic)
    return std::nullopt;
  SongMetaData result;
  const char * data_start = &block[3];
  result._title.assign(data_start, strlen(data_start));
  data_start += 30;
  result._artist.assign(data_start, strlen(data_start));
  data_start += 30;
  result._album.assign(data_start, strlen(data_start));
  data_start += 30;
  std::array<char, 5> year{};
  memcpy(year.data(), data_start, 4);
  result._year = static_cast<uint16_t>(std::atoi(year.data()));
  return result;
}

std::optional<SongMetaData> read_metadata(const fs::path & file)
{
  constexpr unsigned vorbis_magic = 0x67674f, id3v2_magic = 0x334449;
  std::ifstream      f{file, std::ios_base::binary};
  if(!f)
    return std::nullopt;
  char magic_buf[3];
  f.read(magic_buf, 3);
  const auto magic = *reinterpret_cast<unsigned *>(magic_buf) & 0x00ffffff;
  if(magic == id3v2_magic)
    return parse_id3v2(f);
  else if(magic == vorbis_magic)
    return parse_vorbis(f);
  else
    return parse_id3v1(f);
}
