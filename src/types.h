#pragma once

#include "toml.hpp"

#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum class FieldType { kInteger, kString, kCharacter, kFloat, kDouble };
enum class AttributeType { kConfigurable };

struct ParameterFieldDescriptor {
  std::string name;
  FieldType type;
  std::string comment;
  std::string value;

  bool is_integer() const { return type == FieldType::kInteger; }
  bool is_string() const { return type == FieldType::kString; }
};

using ParameterFields =
    std::unordered_map<std::string, ParameterFieldDescriptor>;

struct FieldDescriptor {

  static const int kSizeInt = 0;
  static const int kSizeString = 1;

  std::string name;
  FieldType type;
  std::vector<std::variant<int, std::string>> size;
  std::set<AttributeType> attributes;
  std::string comment;

  int get_dim_size(int i, ParameterFields const &parameters) const {
    if (i >= size.size()) {
      std::cout << "[ERROR]: When accessing dimension size for field: " << name
                << " asked for size of dimension: " << i << ", but " << name
                << " only has " << size.size() << " dimensions." << std::endl;
      abort();
    }
    auto const &dim = size[i];
    if (dim.index() == FieldDescriptor::kSizeString) {
      return std::stol(
          parameters.at(std::get<FieldDescriptor::kSizeString>(dim)).value);
    } else {
      return std::get<FieldDescriptor::kSizeInt>(dim);
    }
  }

  bool is_array() const { return size.size(); }
  bool is_string() const { return (type == FieldType::kString); }
};

struct DerivedType {
  std::string comment;
  std::vector<FieldDescriptor> fields;
};

using DerivedTypes = std::unordered_map<std::string, DerivedType>;

namespace toml {

template <> struct from<FieldType> {
  static FieldType from_toml(const value &v);
};

template <> struct from<AttributeType> {
  static AttributeType from_toml(const value &v);
};

template <> struct from<ParameterFieldDescriptor> {
  static ParameterFieldDescriptor from_toml(const value &v);
};

template <> struct from<FieldDescriptor> {
  static FieldDescriptor from_toml(const value &v);
};

} // namespace toml

std::ostream &operator<<(std::ostream &os, FieldType ft);
std::ostream &operator<<(std::ostream &os, ParameterFieldDescriptor const &fd);
std::ostream &operator<<(std::ostream &os, FieldDescriptor const &fd);