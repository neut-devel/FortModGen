#include "types.h"

#include "fmt/core.h"

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

  auto val = find(v, "value");

  if (val.is_string()) {
    f.value = get<std::string>(val);
  } else if (val.is_floating()) {
    f.value = fmt::format("{:g}", get<double>(val));
  } else if (val.is_integer()) {
    f.value = std::to_string(get<int>(val));
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
      if (f.is_string() && (size_element.as_array().size() > 1)) {
        std::cout << "[ERROR]: We cannot currently handle arrays of strings, "
                     "please submit an issue/PR if this is a problem."
                  << std::endl;
        abort();
      }

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

  if (v.contains("data")) {
    auto data_element = find(v, "data");
    if (data_element.is_array()) {
      size_t ind = 0;
      for (auto const &el : data_element.as_array()) {
        if (el.is_integer()) {
          f.data.emplace_back(get<int>(el));
        } else if (el.is_floating()) {
          f.data.emplace_back(get<double>(el));
        } else if (el.is_string()) {
          f.data.emplace_back(get<std::string>(el));
        } else {
          std::cout << "[ERROR] When parsing descriptor for field: \"" << f.name
                    << "\", found invalid element type at index: " << ind
                    << std::endl;
          abort();
        }
        ind++;
      }
    } else {
      if (data_element.is_integer()) {
        f.data.emplace_back(get<int>(data_element));
      } else if (data_element.is_floating()) {
        f.data.emplace_back(get<double>(data_element));
      } else if (data_element.is_string()) {
        f.data.emplace_back(get<std::string>(data_element));
      } else {
        std::cout << "[ERROR] When parsing descriptor for field: \"" << f.name
                  << "\", found invalid element type " << std::endl;
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

std::ostream &operator<<(std::ostream &os, ParameterFieldDescriptor const &fd) {
  return os << fd.type << ": " << fd.name << " = " << fd.value;
}