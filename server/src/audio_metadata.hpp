#pragma once

#include <string>
#include <iostream>
#include <optional>
struct SongMetaData
{
  std::string _artist;
  uint16_t    _year = 2000;
  std::string _album;
  std::string _title;
};

inline std::ostream & operator<<(std::ostream & os, const SongMetaData & smd)
{
  os << smd._artist << " - " << smd._title << " | " << smd._album << " | " << smd._year;
  return os;
}

std::optional<SongMetaData> read_metadata(const fs::path & file);
