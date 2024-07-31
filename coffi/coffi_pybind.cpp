#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>

#include <coffi/coffi.hpp>

namespace py = pybind11;
using namespace COFFI;

PYBIND11_MAKE_OPAQUE(std::vector<directory*>);

PYBIND11_MODULE(coffipy, m) {
  py::class_<string_to_name_provider>(m, "string_to_name_provider")
    .def("string_to_name", &string_to_name_provider::string_to_name)
    .def("section_string_to_name", &string_to_name_provider::section_string_to_name)
    .def("name_to_string", &string_to_name_provider::name_to_string)
    .def("name_to_section_string", &string_to_name_provider::name_to_section_string);

  py::class_<coffi_strings, string_to_name_provider>(m, "coffi_strings", py::multiple_inheritance())
    .def(py::init<>())
    .def("get_strings_size", &coffi_strings::get_strings_size)
    .def("set_strings_size", &coffi_strings::set_strings_size)
    .def("get_strings", [](coffi_strings &self){
      const char *data = self.get_strings();
      if(data)
        return py::bytes(data, self.get_strings_size());

      return py::bytes("");
    })
    .def("set_strings", &coffi_strings::set_strings);

  py::class_<auxiliary_symbol_record>(m, "auxiliary_symbol_record")
    .def(py::init<>())
    .def("get_value", [](auxiliary_symbol_record &self) {
      return py::bytes(self.value, sizeof(self.value));
    })
    .def("set_value", [](auxiliary_symbol_record &self, const std::string &new_value) {
        memset(self.value, 0, sizeof(self.value));
        memcpy(self.value, new_value.data(), std::min(sizeof(self.value), new_value.size()));
    });

  py::class_<image_data_directory>(m, "image_data_directory")
    .def(py::init<>())
    .def(py::init<uint32_t, uint32_t>())
    .def_readwrite("virtual_address", &image_data_directory::virtual_address)
    .def_readwrite("size", &image_data_directory::size);

  py::class_<symbol>(m, "symbol")
    .def(py::init<string_to_name_provider*>())
    .def_property("value", &symbol::get_value, &symbol::set_value)
    .def_property("section_number", &symbol::get_section_number, &symbol::set_section_number)
    .def_property("type", &symbol::get_type, &symbol::set_type)
    .def_property("storage_class", &symbol::get_storage_class, &symbol::set_storage_class)
    .def_property("aux_symbols_number", &symbol::get_aux_symbols_number, &symbol::set_aux_symbols_number)
    .def("get_index", &symbol::get_index)
    .def("set_index", &symbol::set_index)
    .def("get_name", &symbol::get_name)
    .def("get_auxiliary_symbols", static_cast<std::vector<auxiliary_symbol_record>& (symbol::*)()>(&symbol::get_auxiliary_symbols), py::return_value_policy::reference_internal)
    .def("load", &symbol::load)
    .def("save", &symbol::save);

  py::class_<symbol_provider>(m, "symbol_provider")
    .def("get_symbol", static_cast<symbol* (symbol_provider::*)(uint32_t)>(&symbol_provider::get_symbol), py::return_value_policy::reference_internal)
    .def("get_symbol", static_cast<symbol* (symbol_provider::*)(const std::string&)>(&symbol_provider::get_symbol), py::return_value_policy::reference_internal)
    .def("add_symbol", &symbol_provider::add_symbol, py::return_value_policy::reference_internal);

  py::class_<coffi_symbols, symbol_provider, string_to_name_provider>(m, "coffi_symbols", py::multiple_inheritance())
    .def("get_symbols", static_cast<std::vector<symbol>* (coffi_symbols::*)()>(&coffi_symbols::get_symbols), py::return_value_policy::reference_internal);

  py::class_<architecture_provider>(m, "architecture_provider")
    .def("get_architecture", &architecture_provider::get_architecture)
    .def("get_addressable_unit", &architecture_provider::get_addressable_unit);

  py::class_<sections_provider, architecture_provider>(m, "sections_provider", py::multiple_inheritance())
    .def("get_msdos_header", &sections_provider::get_msdos_header, py::return_value_policy::reference_internal)
    .def("get_header", &sections_provider::get_header, py::return_value_policy::reference_internal)
    .def("get_optional_header", &sections_provider::get_optional_header, py::return_value_policy::reference_internal)
    .def("get_win_header", &sections_provider::get_win_header, py::return_value_policy::reference_internal)
    .def("get_sections", &sections_provider::get_sections, py::return_value_policy::reference_internal);

  py::class_<coffi, coffi_strings, coffi_symbols, sections_provider>(m, "coffi", py::multiple_inheritance())
    .def(py::init<>())
    .def("load", py::overload_cast<const std::string&>(&coffi::load))
    .def("load", py::overload_cast<std::istream&>(&coffi::load))
    .def("save", py::overload_cast<const std::string&>(&coffi::save))
    .def("save", py::overload_cast<std::ostream&>(&coffi::save))
    .def("create", &coffi::create)
    .def("create_optional_header", &coffi::create_optional_header)
    .def("get_msdos_header", static_cast<dos_header* (coffi::*)()>(&coffi::get_msdos_header), py::return_value_policy::reference_internal)
    .def("get_header", static_cast<coff_header* (coffi::*)()>(&coffi::get_header), py::return_value_policy::reference_internal)
    .def("get_optional_header", static_cast<optional_header* (coffi::*)()>(&coffi::get_optional_header), py::return_value_policy::reference_internal)
    .def("get_win_header", static_cast<win_header* (coffi::*)()>(&coffi::get_win_header), py::return_value_policy::reference_internal)
    .def("get_sections", static_cast<sections& (coffi::*)()>(&coffi::get_sections), py::return_value_policy::reference_internal)
    .def("add_section", &coffi::add_section)
    .def("get_directories", static_cast<directories& (coffi::*)()>(&coffi::get_directories), py::return_value_policy::reference_internal)
    .def("add_directory", &coffi::add_directory)
    .def("is_PE32_plus", &coffi::is_PE32_plus)
    .def("layout", &coffi::layout);

  py::class_<dos_header>(m, "dos_header")
    .def(py::init<>())
    .def_property("signature", &dos_header::get_signature, &dos_header::set_signature)
    .def_property("bytes_in_last_block", &dos_header::get_bytes_in_last_block, &dos_header::set_bytes_in_last_block)
    .def_property("blocks_in_file", &dos_header::get_blocks_in_file, &dos_header::set_blocks_in_file)
    .def_property("num_relocs", &dos_header::get_num_relocs, &dos_header::set_num_relocs)
    .def_property("header_paragraphs", &dos_header::get_header_paragraphs, &dos_header::set_header_paragraphs)
    .def_property("min_extra_paragraphs", &dos_header::get_min_extra_paragraphs, &dos_header::set_min_extra_paragraphs)
    .def_property("max_extra_paragraphs", &dos_header::get_max_extra_paragraphs, &dos_header::set_max_extra_paragraphs)
    .def_property("ss", &dos_header::get_ss, &dos_header::set_ss)
    .def_property("sp", &dos_header::get_sp, &dos_header::set_sp)
    .def_property("checksum", &dos_header::get_checksum, &dos_header::set_checksum)
    .def_property("ip", &dos_header::get_ip, &dos_header::set_ip)
    .def_property("cs", &dos_header::get_cs, &dos_header::set_cs)
    .def_property("reloc_table_offset", &dos_header::get_reloc_table_offset, &dos_header::set_reloc_table_offset)
    .def_property("overlay_number", &dos_header::get_overlay_number, &dos_header::set_overlay_number)
    .def_property("oem_id", &dos_header::get_oem_id, &dos_header::set_oem_id)
    .def_property("oem_info", &dos_header::get_oem_info, &dos_header::set_oem_info)
    .def_property("pe_sign_location", &dos_header::get_pe_sign_location, &dos_header::set_pe_sign_location)
    .def("get_sizeof", &dos_header::get_sizeof)
    .def("load", &dos_header::load)
    .def("save", &dos_header::save)
    .def("get_stub", &dos_header::get_stub)
    .def("get_stub_size", &dos_header::get_stub_size)
    .def("set_stub", py::overload_cast<const char*, uint32_t>(&dos_header::set_stub))
    .def("set_stub", py::overload_cast<const std::string&>(&dos_header::set_stub));

  py::class_<coff_header>(m, "coff_header")
    .def_property("version", &coff_header::get_version, &coff_header::set_version)
    .def_property("machine", &coff_header::get_machine, &coff_header::set_machine)
    .def_property("sections_count", &coff_header::get_sections_count, &coff_header::set_sections_count)
    .def_property("time_data_stamp", &coff_header::get_time_data_stamp, &coff_header::set_time_data_stamp)
    .def_property("symbol_table_offset", &coff_header::get_symbol_table_offset, &coff_header::set_symbol_table_offset)
    .def_property("symbols_count", &coff_header::get_symbols_count, &coff_header::set_symbols_count)
    .def_property("optional_header_size", &coff_header::get_optional_header_size, &coff_header::set_optional_header_size)
    .def_property("flags", &coff_header::get_flags, &coff_header::set_flags)
    .def_property("target_id", &coff_header::get_target_id, &coff_header::set_target_id)
    .def("get_sizeof", &coff_header::get_sizeof)
    .def("load", &coff_header::load)
    .def("save", &coff_header::save);

  py::class_<optional_header>(m, "optional_header")
    .def_property("magic", &optional_header::get_magic, &optional_header::set_magic)
    .def_property("major_linker_version", &optional_header::get_major_linker_version, &optional_header::set_major_linker_version)
    .def_property("minor_linker_version", &optional_header::get_minor_linker_version, &optional_header::set_minor_linker_version)
    .def_property("linker_version", &optional_header::get_linker_version, &optional_header::set_linker_version)
    .def_property("code_size", &optional_header::get_code_size, &optional_header::set_code_size)
    .def_property("initialized_data_size", &optional_header::get_initialized_data_size, &optional_header::set_initialized_data_size)
    .def_property("uninitialized_data_size", &optional_header::get_uninitialized_data_size, &optional_header::set_uninitialized_data_size)
    .def_property("entry_point_address", &optional_header::get_entry_point_address, &optional_header::set_entry_point_address)
    .def_property("code_base", &optional_header::get_code_base, &optional_header::set_code_base)
    .def_property("data_base", &optional_header::get_data_base, &optional_header::set_data_base)
    .def("get_sizeof", &optional_header::get_sizeof)
    .def("load", &optional_header::load)
    .def("save", &optional_header::save);

  py::class_<win_header>(m, "win_header")
    .def_property("image_base", &win_header::get_image_base, &win_header::set_image_base)
    .def_property("section_alignment", &win_header::get_section_alignment, &win_header::set_section_alignment)
    .def_property("file_alignment", &win_header::get_file_alignment, &win_header::set_file_alignment)
    .def_property("major_os_version", &win_header::get_major_os_version, &win_header::set_major_os_version)
    .def_property("minor_os_version", &win_header::get_minor_os_version, &win_header::set_minor_os_version)
    .def_property("major_image_version", &win_header::get_major_image_version, &win_header::set_major_image_version)
    .def_property("minor_image_version", &win_header::get_minor_image_version, &win_header::set_minor_image_version)
    .def_property("major_subsystem_version", &win_header::get_major_subsystem_version, &win_header::set_major_subsystem_version)
    .def_property("minor_subsystem_version", &win_header::get_minor_subsystem_version, &win_header::set_minor_subsystem_version)
    .def_property("win32_version_value", &win_header::get_win32_version_value, &win_header::set_win32_version_value)
    .def_property("image_size", &win_header::get_image_size, &win_header::set_image_size)
    .def_property("headers_size", &win_header::get_headers_size, &win_header::set_headers_size)
    .def_property("checksum", &win_header::get_checksum, &win_header::set_checksum)
    .def_property("subsystem", &win_header::get_subsystem, &win_header::set_subsystem)
    .def_property("dll_flags", &win_header::get_dll_flags, &win_header::set_dll_flags)
    .def_property("stack_reserve_size", &win_header::get_stack_reserve_size, &win_header::set_stack_reserve_size)
    .def_property("stack_commit_size", &win_header::get_stack_commit_size, &win_header::set_stack_commit_size)
    .def_property("heap_reserve_size", &win_header::get_heap_reserve_size, &win_header::set_heap_reserve_size)
    .def_property("heap_commit_size", &win_header::get_heap_commit_size, &win_header::set_heap_commit_size)
    .def_property("loader_flags", &win_header::get_loader_flags, &win_header::set_loader_flags)
    .def_property("number_of_rva_and_sizes", &win_header::get_number_of_rva_and_sizes, &win_header::set_number_of_rva_and_sizes)
    .def("get_sizeof", &win_header::get_sizeof)
    .def("load", &win_header::load)
    .def("save", &win_header::save);

  /* py::bind_vector<std::vector<relocation>>(m, "RelocationVector"); */

  py::class_<section>(m, "section")
    .def_property("index", &section::get_index, &section::set_index)
    .def_property("virtual_size", &section::get_virtual_size, &section::set_virtual_size)
    .def_property("physical_address", &section::get_physical_address, &section::set_physical_address)
    .def_property("virtual_address", &section::get_virtual_address, &section::set_virtual_address)
    .def_property("data_size", &section::get_data_size, &section::set_data_size)
    .def_property("data_offset", &section::get_data_offset, &section::set_data_offset)
    .def_property("reloc_offset", &section::get_reloc_offset, &section::set_reloc_offset)
    .def_property("line_num_offset", &section::get_line_num_offset, &section::set_line_num_offset)
    .def_property("reloc_count", &section::get_reloc_count, &section::set_reloc_count)
    .def_property("line_num_count", &section::get_line_num_count, &section::set_line_num_count)
    .def_property("flags", &section::get_flags, &section::set_flags)
    .def_property("page_number", &section::get_page_number, &section::set_page_number)
    .def_property("alignment", &section::get_alignment, &section::set_alignment)
    .def("get_sizeof", &section::get_sizeof)
    .def("get_name", &section::get_name)
    .def("set_name", &section::set_name)
    .def("get_data", [](section &self){
      const char *data = self.get_data();
      if(data)
        return py::bytes(data, self.get_data_size());

      return py::bytes("");
    })
    .def("set_data", py::overload_cast<const char*, uint32_t>(&section::set_data))
    .def("set_data", py::overload_cast<const std::string&>(&section::set_data))
    .def("append_data", py::overload_cast<const char*, uint32_t>(&section::append_data))
    .def("append_data", py::overload_cast<const std::string&>(&section::append_data))
    .def("load", &section::load)
    .def("save_header", &section::save_header)
    .def("save_data", &section::save_data)
    .def("save_relocations", &section::save_relocations)
    .def("get_relocations_filesize", &section::get_relocations_filesize)
    .def("save_line_numbers", &section::save_line_numbers)
    .def("get_line_numbers_filesize", &section::get_line_numbers_filesize)
    .def("get_relocations", &section::get_relocations, py::return_value_policy::reference_internal)
    .def("add_relocation_entry", &section::add_relocation_entry);

  /* py::bind_vector<std::vector<section*>>(m, "VectorRefSection"); */
  py::class_<COFFI::sections>(m, "sections")
    .def(py::init<>())
    .def("clean", &COFFI::sections::clean)
    .def("__getitem__", 
         py::overload_cast<size_t>(&COFFI::sections::operator[], py::const_),
         py::return_value_policy::reference)
    .def("__getitem__", 
         py::overload_cast<const std::string&>(&COFFI::sections::operator[], py::const_),
         py::return_value_policy::reference)
    .def("__delitem__", [](COFFI::sections &self, size_t i) {
        delete self[i];
        self.erase(self.begin() + i);
    })
    .def("__len__", [](const COFFI::sections &self) { 
        return self.size(); 
    })
    .def("append", [](COFFI::sections &self, COFFI::section* sec) {
        self.push_back(sec);
    });

  py::class_<directory>(m, "directory")
    .def(py::init<uint32_t>())
    .def_property("virtual_address", &directory::get_virtual_address, &directory::set_virtual_address)
    .def_property("size", &directory::get_size, &directory::set_size)
    .def("get_sizeof", &directory::get_sizeof)
    .def("get_index", &directory::get_index)
    .def("get_data", [](directory &self){
      const char *data = self.get_data();
      if(data)
        return py::bytes(data, self.get_size());

      return py::bytes("");
    })
    .def("set_data", &directory::set_data)
    .def("get_data_filesize", &directory::get_data_filesize)
    .def("load", &directory::load)
    .def("load_data", &directory::load_data)
    .def("save", &directory::save)
    .def("save_data", &directory::save_data)
    .def("clean", &directory::clean);

  py::bind_vector<std::vector<directory*>>(m, "VectorRefDirectory");
  py::class_<directories, std::vector<directory*>>(m, "directories", py::multiple_inheritance())
    .def(py::init<sections_provider*>())
    .def("clean", &directories::clean)
    .def("load", &directories::load)
    .def("load_data", &directories::load_data)
    .def("save", &directories::save)
    .def("get_sizeof", &directories::get_sizeof);

  py::enum_<coffi_architecture_t>(m, "coffi_architecture_t")
    .value("COFFI_ARCHITECTURE_NONE", coffi_architecture_t::COFFI_ARCHITECTURE_NONE)
    .value("COFFI_ARCHITECTURE_PE", coffi_architecture_t::COFFI_ARCHITECTURE_PE)
    .value("COFFI_ARCHITECTURE_TI", coffi_architecture_t::COFFI_ARCHITECTURE_TI)
    .value("COFFI_ARCHITECTURE_CEVA", coffi_architecture_t::COFFI_ARCHITECTURE_CEVA)
    .export_values();

  py::class_<relocation>(m, "relocation")
    .def_property("virtual_address", &relocation::get_virtual_address, &relocation::set_virtual_address)
    .def("set_virtual_address", &relocation::set_virtual_address)
    .def_property_readonly("symbol_table_index", &relocation::get_symbol_table_index)
    .def_property("type", &relocation::get_type, &relocation::set_type)
    .def_property("reserved", &relocation::get_reserved, &relocation::set_reserved)
    .def("get_symbol", &relocation::get_symbol)
    .def("set_symbol", &relocation::set_symbol)
    .def("load", &relocation::load)
    .def("save", &relocation::save)
    .def("get_sizeof", &relocation::get_sizeof);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
