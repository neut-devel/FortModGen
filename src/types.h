#pragma once

#include "toml.hpp"

#include <iostream>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum class FieldType { kInteger, kString, kCharacter, kFloat, kDouble, kBool };
enum class AttributeType { kConfigurable };

struct ParameterFieldDescriptor {
  std::string name;
  FieldType type;
  bool is_numeric;
  std::string comment;
  std::string value;

  bool is_integer() const { return type == FieldType::kInteger; }
  bool is_string() const { return type == FieldType::kString; }
  bool is_floating() const {
    return (type == FieldType::kFloat) || (type == FieldType::kDouble);
  }
};

using ParameterFields = std::vector<ParameterFieldDescriptor>;

struct FieldDescriptor {

  static const int kSizeInt = 0;
  static const int kSizeString = 1;

  std::string name;
  FieldType type;
  std::vector<std::variant<int, std::string>> size;
  std::set<AttributeType> attributes;
  std::string comment;
  using data_element_type = std::variant<int, double, std::string>;
  std::vector<data_element_type> data;

  int get_size(ParameterFields const &parameters) const {
    int full_size = 1;
    for (int i = 0; i < size.size(); ++i) {
      full_size *= get_dim_size(i, parameters);
    }
    return full_size;
  }

  std::string get_fort_shape_str(ParameterFields const &parameters) const {
    std::stringstream ss("");
    for (int i = 0; i < size.size(); ++i) {
      ss << get_dim_size(i, parameters) << ((i + 1 == size.size()) ? "" : ":");
    }
    return ss.str();
  }

  std::string get_cshape_str(ParameterFields const &parameters) const {
    std::stringstream ss("");
    for (int i = size.size(); i > 0; --i) {
      ss << "[" << get_dim_size(i - 1, parameters) << "]";
    }
    return ss.str();
  }

  int get_dim_size(int i, ParameterFields const &parameters) const {
    if (i >= size.size()) {
      std::cout << "[ERROR]: When accessing dimension size for field: " << name
                << " asked for size of dimension: " << i << ", but " << name
                << " only has " << size.size() << " dimensions." << std::endl;
      abort();
    }
    auto const &dim = size[i];
    if (dim.index() == FieldDescriptor::kSizeString) {

      for (auto const &p : parameters) {
        if (std::get<FieldDescriptor::kSizeString>(dim) == p.name) {
          return std::stol(p.value);
        }
      }
      std::cout << "[ERROR]: No known parameter named: "
                << std::get<FieldDescriptor::kSizeString>(dim) << std::endl;
      abort();
    } else {
      return std::get<FieldDescriptor::kSizeInt>(dim);
    }
  }
  std::string get_dim_size_str(int i) const {
    if (i >= size.size()) {
      std::cout << "[ERROR]: When accessing dimension size for field: " << name
                << " asked for size of dimension: " << i << ", but " << name
                << " only has " << size.size() << " dimensions." << std::endl;
      abort();
    }
    auto const &dim = size[i];
    if (dim.index() == FieldDescriptor::kSizeString) {
      return std::get<FieldDescriptor::kSizeString>(dim);
    } else {
      return std::to_string(std::get<FieldDescriptor::kSizeInt>(dim));
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

std::string to_string(FieldType ft);
std::string to_string(ParameterFieldDescriptor const &fd);
std::string to_string(FieldDescriptor const &fd);