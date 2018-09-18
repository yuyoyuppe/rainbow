#pragma once

#include <vector>
#include <string>

struct Playlist
{
  std::vector<struct Artist> _artists;
};

struct Artist
{
  std::vector<struct Album> _albums;
};

struct Album
{
  uint16_t                  _year;
  std::vector<struct Song> _songs;
};

struct Song
{
  std::string _title;
};