#include "types.h"

#include "fmt/core.h"
#include "fmt/format.h"

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
  } else if (typenm == "bool") {
    return FieldType::kBool;
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

  try {
    if (val.is_string()) {
      f.is_numeric = false;
      f.value = get<std::string>(val);
    } else if (val.is_floating()) {
      f.is_numeric = true;
      if (f.type == FieldType::kFloat) {
        f.value = fmt::format("{:.7E}", get<double>(val));

      } else if (f.type == FieldType::kDouble) {
        f.value = fmt::format("{:.15E}", get<double>(val));
      } else {
        f.value = fmt::format("{:g}", get<double>(val));
      }
    } else if (val.is_integer()) {
      f.is_numeric = true;
      f.value = std::to_string(get<int>(val));
    } else if (val.is_boolean()) {
      f.is_numeric = false;
      f.value = val.as_boolean() ? "true" : "false";
    } else {
      std::cout << "[ERROR]: Failed to parse parameter value as known type: "
                << val.as_string() << std::endl;
      abort();
    }
  } catch (toml::exception e) {
    std::cout << "[ERROR]: Failed to parse parameter value: " << e.what()
              << std::endl;
    abort();
  } catch (fmt::format_error e) {
    std::cout << "[ERROR]: Failed to format value: " << e.what() << std::endl;
    abort();
  } catch (...) {
    std::cout
        << "[ERROR]: Unspecified exception when parsing parameter value. "
           "Please report this message and your input file to the developer."
        << std::endl;
    abort();
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
        } else if (el.is_boolean()) {
          f.data.emplace_back(get<bool>(el));
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
      } else if (data_element.is_boolean()) {
        f.data.emplace_back(int(get<bool>(data_element)));
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
  case FieldType::kBool: {
    return os << "bool";
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

std::string to_string(FieldType ft) {
  std::stringstream ss("");
  ss << ft;
  return ss.str();
}
std::string to_string(ParameterFieldDescriptor const &fd) {
  std::stringstream ss("");
  ss << fd;
  return ss.str();
}
std::string to_string(FieldDescriptor const &fd) {
  std::stringstream ss("");
  ss << fd;
  return ss.str();
}