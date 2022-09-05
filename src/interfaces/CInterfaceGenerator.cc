#include "CInterfaceGenerator.h"

#include "utils.h"

#include "fmt/os.h"

#include <map>

std::map<FieldType, std::string> CFieldTypes = {
    {FieldType::kInteger, "int"},    {FieldType::kString, "char"},
    {FieldType::kCharacter, "char"}, {FieldType::kFloat, "float"},
    {FieldType::kDouble, "double"},
};

void ModuleStructsHeader(fmt::ostream &os, std::string const &modname) {
  os.print(R"(#pragma once

#ifdef __cplusplus
#include<iostream>
#include<string>
#include<cstring>

extern "C" {{

#endif

)");
}

void ModuleStructsParameters(fmt::ostream &os,
                             ParameterFields const &ParameterFieldDescriptors) {
  for (auto const &p : ParameterFieldDescriptors) {
    std::string comment = SanitizeComment(p.comment, "//");
    if (comment.length()) {
      os.print("//{}\n", comment);
    }
    if (p.is_string()) {
      os.print("static char const * {} = \"{}\";\n", p.name, p.value);
    } else {
      os.print("static {} const {} = {};\n", CFieldTypes[p.type], p.name,
               p.value);
    }
    os.print("\n");
  }
}

void ModuleStructsDerivedTypeHeader(fmt::ostream &os,
                                    std::string const &dtypename,
                                    std::string comment) {

  comment = SanitizeComment(comment, "//");
  if (comment.length()) {
    os.print("//{}\n", comment);
  }
  os.print("struct {}_t {{\n\n", dtypename);
}

void ModuleStructsDerivedTypeField(fmt::ostream &os,
                                   std::string const &dtypename,
                                   FieldDescriptor const &fd,
                                   ParameterFields const &parameters) {
  std::string comment = SanitizeComment(fd.comment, "  //");
  if (comment.length()) {
    os.print("  //{}\n", comment);
  }
  os.print("  {} {}", CFieldTypes[fd.type], fd.name);

  if (fd.is_array()) {
    for (int i = fd.size.size(); i > 0; --i) {

      int dim_size = fd.get_dim_size(i - 1, parameters);
      if (fd.is_string()) {
        dim_size++;
      }

      os.print("[{}]", dim_size);
    }
  }
  os.print(";\n");

  if (fd.is_string()) {
    os.print(R"(
#ifdef __cplusplus
  std::string get_{1}() const {{ return std::string({1}, {2}); }}
  void set_{1}(std::string in_str) {{
    if (in_str.size() > {2}) {{
      std::cout
          << "[WARN]: String: \"" << in_str
          << "\", is too large to fit in {0}::{1}, truncated to {2} characters."
          << std::endl;
    }}
    std::memcpy({1}, in_str.c_str(), std::min(size_t({2}), in_str.size()));
  }}
#endif
)",
             dtypename, fd.name, fd.get_size(parameters));
  }
}

void ModuleStructsDerivedTypeFooter(fmt::ostream &os,
                                    std::string const &dtypename) {
  os.print(R"(
}};
)");
}

void ModuleStructsFooter(fmt::ostream &os, std::string const &modname) {
  os.print("#ifdef __cplusplus\n}}\n#endif\n");
}

void CInterfaceHeader(fmt::ostream &os) {
  os.print("\n#ifndef __cplusplus\n#include <stdlib.h>\n#include <string.h>\n");
}

void CInterfaceDerivedTypeHeader(fmt::ostream &os,
                                 std::string const &dtypename) {
  os.print(R"(

//Fortran function declarations for struct interface for {0}
void copy_{0}(struct {0}_t const *);
void update_{0}(struct {0}_t const *);

//C memory management helpers for {0}
inline struct {0}_t *alloc_{0}(){{
  return malloc(sizeof(struct {0}_t));
}}

inline void free_{0}(struct {0}_t *inst){{
  if(inst != NULL){{
    free(inst);
  }}
}}
)",
           dtypename);
}

void CInterfaceFooter(fmt::ostream &os) { os.print("\n#else\n"); }

void CPPInterfaceHeader(fmt::ostream &os) {
  os.print(R"(namespace FortMod {{
)");
}

void CPPInterfaceDerivedType(fmt::ostream &os, std::string const &dtypename) {
  os.print(R"(
//C++ Interface for {0}

extern "C" {{
  //Fortran function declarations for struct interface for {0}
  void copy_{0}(void *);
  void update_{0}(void *);
}}

namespace {0}IF {{

inline {0}_t copy(){{
  {0}_t inst;
  copy_{0}(&inst);
  return inst;
}}

inline void update({0}_t inst){{
  update_{0}(&inst);
}}

}}

)",
           dtypename);
}

void CPPInterfaceFooter(fmt::ostream &os) { os.print("}}\n#endif\n"); }

void GenerateCInterface(std::string const &fname, std::string const &modname,
                        ParameterFields const &parameters,
                        DerivedTypes const &dtypes,
                        std::vector<std::string> const &Uses) {

  auto out = fmt::output_file(fname);

  ModuleStructsHeader(out, modname);

  ModuleStructsParameters(out, parameters);

  for (auto const &dt : dtypes) {

    ModuleStructsDerivedTypeHeader(out, dt.first, dt.second.comment);

    for (auto const &fd : dt.second.fields) {

      ModuleStructsDerivedTypeField(out, dt.first, fd, parameters);
    }

    ModuleStructsDerivedTypeFooter(out, dt.first);
  }

  ModuleStructsFooter(out, modname);

  CInterfaceHeader(out);
  for (auto const &dt : dtypes) {
    CInterfaceDerivedTypeHeader(out, dt.first);
  }
  CInterfaceFooter(out);

  CPPInterfaceHeader(out);
  for (auto const &dt : dtypes) {
    CPPInterfaceDerivedType(out, dt.first);
  }
  CPPInterfaceFooter(out);
}