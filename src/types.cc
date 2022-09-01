#include "types.h"

#include <iostream>

namespace toml {

FieldType from<FieldType>::from_toml(const value &v) {
  auto typenm = get<std::string>(v);
  if (typenm == "integer") {
    return FieldType::kInteger;
  } else if (typenm == "string") {
    return FieldType::kString;
  } else if (typenm == "character") {
    return FieldType::kCharacter;
  } else if (typenm == "float") {
    return FieldType::kFloat;
  } else if (typenm == "double") {
    return FieldType::kDouble;
  } else {
    std::cout << "[ERROR]: Unhandled FieldType: " << typenm << std::endl;
    abort();
  }
}

AttributeType from<AttributeType>::from_toml(const value &v) {
  auto typenm = get<std::string>(v);
  if (typenm == "configurable") {
    return AttributeType::kConfigurable;
  } else {
    std::cout << "[ERROR]: Unhandled AttributeType: " << typenm << std::endl;
    abort();
  }
}

ParameterFieldDescriptor
from<ParameterFieldDescriptor>::from_toml(const value &v) {
  ParameterFieldDescriptor f;
  f.name = find<std::string>(v, "name");
  f.type = find<FieldType>(v, "type");

  f.comment = find_or<std::string>(v, "comment", "");

  switch (f.type) {
  case FieldType::kInteger: {
    auto rval = find<int>(v, "value");
    f.value = std::to_string(rval);
    break;
  }
  case FieldType::kString: {
    f.value = find<std::string>(v, "value");
    break;
  }
  case FieldType::kCharacter: {
    f.value = find<std::string>(v, "value");
    break;
  }
  case FieldType::kFloat: {
    auto rval = find<float>(v, "value");
    f.value = std::to_string(rval);
    break;
  }
  case FieldType::kDouble: {
    auto rval = find<double>(v, "value");
    f.value = std::to_string(rval);
    break;
  }
  }

  return f;
}

FieldDescriptor from<FieldDescriptor>::from_toml(const value &v) {
  FieldDescriptor f;
  f.name = find<std::string>(v, "name");
  f.type = find<FieldType>(v, "type");

  auto attribute_list =
      find_or<std::vector<AttributeType>>(v, "attributes", {});
  for (auto a : attribute_list) {
    f.attributes.insert(a);
  }
  f.comment = find_or<std::string>(v, "comment", "");

  if (v.contains("size")) {
    auto size_element = find(v, "size");
    if (size_element.is_array()) {
      size_t ind = 0;
      for (auto const &el : size_element.as_array()) {
        if (el.is_integer()) {
          f.size.emplace_back(get<int>(el));
        } else if (el.is_string()) {
          f.size.emplace_back(get<std::string>(el));
        } else {
          std::cout << "[ERROR] When parsing descriptor for field: \"" << f.name
                    << "\", found invalid size element type at index: " << ind
                    << std::endl;
          abort();
        }
        ind++;
      }
    } else {
      if (size_element.is_integer()) {
        f.size.emplace_back(get<int>(size_element));
      } else if (size_element.is_string()) {
        f.size.emplace_back(get<std::string>(size_element));
      } else {
        std::cout << "[ERROR] When parsing descriptor for field: \"" << f.name
                  << "\", found invalid size element type " << std::endl;
        abort();
      }
    }
  }

  return f;
}

} // namespace toml

std::ostream &operator<<(std::ostream &os, FieldType ft) {
  switch (ft) {
  case FieldType::kInteger: {
    return os << "integer";
  }
  case FieldType::kString: {
    return os << "string";
  }
  case FieldType::kCharacter: {
    return os << "character";
  }
  case FieldType::kFloat: {
    return os << "float";
  }
  case FieldType::kDouble: {
    return os << "double";
  }
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, AttributeType ft) {
  switch (ft) {
  case AttributeType::kConfigurable: {
    return os << "configurable";
  }
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, FieldDescriptor const &fd) {
  os << fd.type << ": " << fd.name;

  if (fd.is_array()) {
    os << "(";
    for (int i = 0; i < fd.size.size(); ++i) {
      auto dim = fd.size[i];
      if (dim.index() == FieldDescriptor::kSizeString) {
        os << std::get<FieldDescriptor::kSizeString>(dim)
           << ((i + 1 == fd.size.size()) ? "" : ", ");
      } else {
        os << std::get<FieldDescriptor::kSizeInt>(dim)
           << ((i + 1 == fd.size.size()) ? "" : ", ");
      }
    }
    os << ")";
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, ParameterFieldDescriptor const &fd){
  return os << fd.type << ": " << fd.name << " = " << fd.value;
}