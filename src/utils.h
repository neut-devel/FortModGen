#pragma once

#include <string>

inline std::string SanitizeComment(std::string comment,
                                   std::string const &comment_characters) {
  size_t next = comment.find('\n');
  while (next != std::string::npos) {

    if (next == comment.size()) { // strip off a trailing newline
      comment = comment.substr(0, comment.size() - 1);
      break;
    }

    comment.insert(next + 1, comment_characters);
    next = comment.find('\n', next + 1);
  }
  return comment;
}