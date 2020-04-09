/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 * Contains some contributions under the Thrift Software License.
 * Please see doc/old-thrift-license.txt in the Thrift distribution for
 * details.
 */

#include <cassert>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <cctype>

#include <stdlib.h>
#include <sys/stat.h>
#include <sstream>

#include "thrift/platform.h"
#include "thrift/generate/t_oop_generator.h"
#include "thrift/generate/t_vala_generator.h"

using std::map;
using std::ostream;
using std::ostringstream;
using std::string;
using std::stringstream;
using std::vector;

t_vala_generator::t_vala_generator(t_program* program, const map<string, string>& parsed_options, const string& option_string)
    : t_oop_generator(program)
{
    (void)option_string;
    use_pascal_case_properties = false;

    map<string, string>::const_iterator iter;

    for (iter = parsed_options.begin(); iter != parsed_options.end(); ++iter)
    {
        if (iter->first.compare("pascal") == 0)
        {
          use_pascal_case_properties = true;
        }
        else 
        {
          throw "unknown option vala:" + iter->first;
        }
    }

    out_dir_base_ = "gen-vala";
}

static string correct_function_name_for_async(string const& function_name)
{
    string const async_end = "Async";
    size_t i = function_name.find(async_end);
    if (i != string::npos)
    {
        return function_name + async_end;
    }

    return function_name;
}

/**
* \brief Search and replace "_args" substring in struct name if exist (for Vala class naming)
* \param struct_name
* \return Modified struct name ("Struct_args" -> "StructArgs") or original name
*/
static string check_and_correct_struct_name(const string& struct_name)
{
    string args_end = "_args";
    size_t i = struct_name.find(args_end);
    if (i != string::npos)
    {
        string new_struct_name = struct_name;
        new_struct_name.replace(i, args_end.length(), "Args");
        return new_struct_name;
    }

    string result_end = "_result";
    size_t j = struct_name.find(result_end);
    if (j != string::npos)
    {
        string new_struct_name = struct_name;
        new_struct_name.replace(j, result_end.length(), "Result");
        return new_struct_name;
    }

    return struct_name;
}

static bool field_has_default(t_field* tfield) { return tfield->get_value() != NULL; }

static bool field_is_required(t_field* tfield) { return tfield->get_req() == t_field::T_REQUIRED; }

static bool type_can_be_null(t_type* ttype)
{
    while (ttype->is_typedef())
    {
        ttype = static_cast<t_typedef*>(ttype)->get_type();
    }

    return ttype->is_container() || ttype->is_struct() || ttype->is_xception() || ttype->is_string();
}

map<string, int> t_vala_generator::get_keywords_list() const
{
    return vala_keywords;
}

void t_vala_generator::init_generator()
{
    MKDIR(get_out_dir().c_str());

    // for usage of vala namespaces in thrift files (from files for vala)
    namespace_name_ = program_->get_namespace("vala");
    if (namespace_name_.empty())
    {
        namespace_name_ = program_->get_namespace("vala");
    }

    string dir = namespace_name_;
    string subdir = get_out_dir().c_str();
    string::size_type loc;

    while ((loc = dir.find(".")) != string::npos)
    {
        subdir = subdir + "/" + dir.substr(0, loc);
        MKDIR(subdir.c_str());
        dir = dir.substr(loc + 1);
    }
    if (dir.size() > 0)
    {
        subdir = subdir + "/" + dir;
        MKDIR(subdir.c_str());
    }

    namespace_dir_ = subdir;
    init_keywords();

    while (!member_mapping_scopes.empty())
    {
        cleanup_member_name_mapping(member_mapping_scopes.back().scope_member);
    }

    pverbose("Vala options:\n");
    pverbose("- pascal ..... %s\n", (use_pascal_case_properties ? "ON" : "off"));
}

string t_vala_generator::normalize_name(string name)
{
    string tmp(name);
    transform(tmp.begin(), tmp.end(), tmp.begin(), static_cast<int(*)(int)>(tolower));

    // un-conflict keywords by prefixing with "@"
    if (vala_keywords.find(tmp) != vala_keywords.end())
    {
        return "@" + name;
    }

    // no changes necessary
    return name;
}

void t_vala_generator::init_keywords()
{
    vala_keywords.clear();

    // Vala keywords
    vala_keywords["abstract"] = 1;
    vala_keywords["as"] = 1;
    vala_keywords["async"] = 1;
    vala_keywords["base"] = 1;
    vala_keywords["break"] = 1;
    vala_keywords["case"] = 1;
    vala_keywords["catch"] = 1;
    vala_keywords["class"] = 1;
    vala_keywords["const"] = 1;
    vala_keywords["construct"] = 1;
    vala_keywords["continue"] = 1;
    vala_keywords["default"] = 1;
    vala_keywords["delegate"] = 1;
    vala_keywords["delete"] = 1;
    vala_keywords["do"] = 1;
    vala_keywords["dynamic"] = 1;
    vala_keywords["else"] = 1;
    vala_keywords["ensures"] = 1;
    vala_keywords["enum"] = 1;
    vala_keywords["errordomain"] = 1;
    vala_keywords["extern"] = 1;
    vala_keywords["false"] = 1;
    vala_keywords["finally"] = 1;
    vala_keywords["for"] = 1;
    vala_keywords["foreach"] = 1;
    vala_keywords["get"] = 1;
    vala_keywords["if"] = 1;
    vala_keywords["in"] = 1;
    vala_keywords["inline"] = 1;
    vala_keywords["interface"] = 1;
    vala_keywords["internal"] = 1;
    vala_keywords["is"] = 1;
    vala_keywords["lock"] = 1;
    vala_keywords["namespace"] = 1;
    vala_keywords["new"] = 1;
    vala_keywords["null"] = 1;
    vala_keywords["out"] = 1;
    vala_keywords["override"] = 1;
    vala_keywords["owned"] = 1;
    vala_keywords["params"] = 1;
    vala_keywords["private"] = 1;
    vala_keywords["protected"] = 1;
    vala_keywords["public"] = 1;
    vala_keywords["ref"] = 1;
    vala_keywords["requires"] = 1;
    vala_keywords["return"] = 1;
    vala_keywords["sealed"] = 1;
    vala_keywords["set"] = 1;
    vala_keywords["signal"] = 1;
    vala_keywords["sizeof"] = 1;
    vala_keywords["static"] = 1;
    vala_keywords["struct"] = 1;
    vala_keywords["switch"] = 1;
    vala_keywords["this"] = 1;
    vala_keywords["throw"] = 1;
    vala_keywords["throws"] = 1;
    vala_keywords["true"] = 1;
    vala_keywords["try"] = 1;
    vala_keywords["typeof"] = 1;
    vala_keywords["unlock"] = 1;
    vala_keywords["unowned"] = 1;
    vala_keywords["using"] = 1;
    vala_keywords["var"] = 1;
    vala_keywords["virtual"] = 1;
    vala_keywords["void"] = 1;
    vala_keywords["volatile"] = 1;
    vala_keywords["weak"] = 1;
    vala_keywords["while"] = 1;
    vala_keywords["yield"] = 1;
}

void t_vala_generator::start_vala_namespace(ostream& out)
{
    if (!namespace_name_.empty())
    {
        out << "namespace " << namespace_name_ << endl;
        scope_up(out);
    }
}

void t_vala_generator::end_vala_namespace(ostream& out)
{
    if (!namespace_name_.empty())
    {
        scope_down(out);
    }
}

string t_vala_generator::vala_type_usings() const
{
    string namespaces =
        "using Gee;\n";

    return namespaces + endl;
}

string t_vala_generator::vala_thrift_usings() const
{
    string namespaces =
        "using Thrift;\n"
        "using Thrift.Collections;\n";;

    return namespaces + endl;
}

void t_vala_generator::close_generator()
{
}

void t_vala_generator::generate_typedef(t_typedef* ttypedef)
{
    (void)ttypedef;
}

void t_vala_generator::generate_enum(t_enum* tenum)
{
    int ic = indent_count();
    string f_enum_name = namespace_dir_ + "/" + tenum->get_name() + ".vala";

    ofstream_with_content_based_conditional_update f_enum;
    f_enum.open(f_enum_name.c_str());

    generate_enum(f_enum, tenum);

    f_enum.close();
    indent_validate(ic, "generate_enum");
}

void t_vala_generator::generate_enum(ostream& out, t_enum* tenum)
{
    out << autogen_comment() << endl;

    start_vala_namespace(out);
    generate_vala_doc(out, tenum);

    out << indent() << "public enum " << tenum->get_name() << endl;
    scope_up(out);

    vector<t_enum_value*> constants = tenum->get_constants();
    vector<t_enum_value*>::iterator c_iter;

    for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter)
    {
        generate_vala_doc(out, *c_iter);
        int value = (*c_iter)->get_value();
        out << indent() << (*c_iter)->get_name() << " = " << value << "," << endl;
    }

    scope_down(out);
    end_vala_namespace(out);
}

void t_vala_generator::generate_consts(vector<t_const*> consts)
{
    if (consts.empty())
    {
        return;
    }

    string f_consts_name = namespace_dir_ + '/' + program_name_ + ".Constants.vala";
    ofstream_with_content_based_conditional_update f_consts;
    f_consts.open(f_consts_name.c_str());

    generate_consts(f_consts, consts);

    f_consts.close();
}

void t_vala_generator::generate_consts(ostream& out, vector<t_const*> consts)
{
    if (consts.empty())
    {
        return;
    }

    out << autogen_comment() << vala_type_usings() << endl;

    start_vala_namespace(out);

    out << indent() << "public static class " << make_valid_vala_identifier(program_name_) << "Constants" << endl;

    scope_up(out);

    vector<t_const*>::iterator c_iter;
    bool need_static_constructor = false;
    for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter)
    {
        generate_vala_doc(out, *c_iter);
        if (print_const_value(out, (*c_iter)->get_name(), (*c_iter)->get_type(), (*c_iter)->get_value(), false))
        {
            need_static_constructor = true;
        }
    }

    if (need_static_constructor)
    {
        print_const_constructor(out, consts);
    }

    scope_down(out);
    end_vala_namespace(out);
}

void t_vala_generator::print_const_def_value(ostream& out, string name, t_type* type, t_const_value* value)
{
    if (type->is_struct() || type->is_xception())
    {
        const vector<t_field*>& fields = static_cast<t_struct*>(type)->get_members();
        const map<t_const_value*, t_const_value*, t_const_value::value_compare>& val = value->get_map();
        vector<t_field*>::const_iterator f_iter;
        map<t_const_value*, t_const_value*, t_const_value::value_compare>::const_iterator v_iter;
        prepare_member_name_mapping(static_cast<t_struct*>(type));

        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter)
        {
            t_field* field = NULL;

            for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
            {
                if ((*f_iter)->get_name() == v_iter->first->get_string())
                {
                    field = *f_iter;
                }
            }

            if (field == NULL)
            {
                throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
            }

            t_type* field_type = field->get_type();

            string val = render_const_value(out, name, field_type, v_iter->second);
            out << indent() << name << "." << prop_name(field) << " = " << val << ";" << endl;
        }

        cleanup_member_name_mapping(static_cast<t_struct*>(type));
    }
    else if (type->is_map())
    {
        t_type* ktype = static_cast<t_map*>(type)->get_key_type();
        t_type* vtype = static_cast<t_map*>(type)->get_val_type();
        const map<t_const_value*, t_const_value*, t_const_value::value_compare>& val = value->get_map();
        map<t_const_value*, t_const_value*, t_const_value::value_compare>::const_iterator v_iter;
        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter)
        {
            string key = render_const_value(out, name, ktype, v_iter->first);
            string val = render_const_value(out, name, vtype, v_iter->second);
            out << indent() << name << "[" << key << "]" << " = " << val << ";" << endl;
        }
    }
    else if (type->is_list() || type->is_set())
    {
        t_type* etype;
        if (type->is_list())
        {
            etype = static_cast<t_list*>(type)->get_elem_type();
        }
        else
        {
            etype = static_cast<t_set*>(type)->get_elem_type();
        }

        const vector<t_const_value*>& val = value->get_list();
        vector<t_const_value*>::const_iterator v_iter;
        for (v_iter = val.begin(); v_iter != val.end(); ++v_iter)
        {
            string val = render_const_value(out, name, etype, *v_iter);
            out << indent() << name << ".Add(" << val << ");" << endl;
        }
    }
}

void t_vala_generator::print_const_constructor(ostream& out, vector<t_const*> consts)
{
    out << indent() << "static " << make_valid_vala_identifier(program_name_).c_str() << "Constants()" << endl;
    scope_up(out);

    vector<t_const*>::iterator c_iter;
    for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter)
    {
        string name = (*c_iter)->get_name();
        t_type* type = (*c_iter)->get_type();
        t_const_value* value = (*c_iter)->get_value();

        print_const_def_value(out, name, type, value);
    }
    scope_down(out);
}

bool t_vala_generator::print_const_value(ostream& out, string name, t_type* type, t_const_value* value, bool in_static, bool defval, bool needtype)
{
    out << indent();
    bool need_static_construction = !in_static;
    while (type->is_typedef())
    {
        type = static_cast<t_typedef*>(type)->get_type();
    }

    if (!defval || needtype)
    {
        out << (in_static ? "" : type->is_base_type() ? "public const " : "public static ") << type_name(type) << " ";
    }

    if (type->is_base_type())
    {
        string v2 = render_const_value(out, name, type, value);
        out << normalize_name(name) << " = " << v2 << ";" << endl;
        need_static_construction = false;
    }
    else if (type->is_enum())
    {
        out << name << " = " << type_name(type) << "." << value->get_identifier_name() << ";" << endl;
        need_static_construction = false;
    }
    else if (type->is_struct() || type->is_xception())
    {
        out << name << " = new " << type_name(type) << "();" << endl;
    }
    else if (type->is_map())
    {
        out << name << " = new " << type_name(type) << "();" << endl;
    }
    else if (type->is_list() || type->is_set())
    {
        out << name << " = new " << type_name(type) << "();" << endl;
    }

    if (defval && !type->is_base_type() && !type->is_enum())
    {
        print_const_def_value(out, name, type, value);
    }

    return need_static_construction;
}

string t_vala_generator::render_const_value(ostream& out, string name, t_type* type, t_const_value* value)
{
    (void)name;
    ostringstream render;

    if (type->is_base_type())
    {
        t_base_type::t_base tbase = static_cast<t_base_type*>(type)->get_base();
        switch (tbase)
        {
        case t_base_type::TYPE_STRING:
            render << '"' << get_escaped_string(value) << '"';
            break;
        case t_base_type::TYPE_BOOL:
            render << ((value->get_integer() > 0) ? "true" : "false");
            break;
        case t_base_type::TYPE_I8:
        case t_base_type::TYPE_I16:
        case t_base_type::TYPE_I32:
        case t_base_type::TYPE_I64:
            render << value->get_integer();
            break;
        case t_base_type::TYPE_DOUBLE:
            if (value->get_type() == t_const_value::CV_INTEGER)
            {
                render << value->get_integer();
            }
            else
            {
                render << value->get_double();
            }
            break;
        default:
            throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
        }
    }
    else if (type->is_enum())
    {
        render << type->get_name() << "." << value->get_identifier_name();
    }
    else
    {
        string t = tmp("tmp");
        print_const_value(out, t, type, value, true, true, true);
        render << t;
    }

    return render.str();
}

void t_vala_generator::generate_struct(t_struct* tstruct)
{
    generate_vala_struct(tstruct, false);
}

void t_vala_generator::generate_xception(t_struct* txception)
{
    generate_vala_struct(txception, true);
}

void t_vala_generator::generate_vala_struct(t_struct* tstruct, bool is_exception)
{
    int ic = indent_count();

    string f_struct_name = namespace_dir_ + "/" + (tstruct->get_name()) + ".vala";
    ofstream_with_content_based_conditional_update f_struct;

    f_struct.open(f_struct_name.c_str());

    f_struct << autogen_comment() << vala_type_usings() << vala_thrift_usings() << endl;

    generate_vala_struct_definition(f_struct, tstruct, is_exception);

    f_struct.close();

    indent_validate(ic, "generate_vala_struct");
}

void t_vala_generator::generate_vala_struct_definition(ostream& out, t_struct* tstruct, bool is_exception, bool in_class, bool is_result)
{
    if (!in_class)
    {
        start_vala_namespace(out);
    }

    out << endl;

    generate_vala_doc(out, tstruct);
    prepare_member_name_mapping(tstruct);

    bool is_final = tstruct->annotations_.find("final") != tstruct->annotations_.end();

    string sharp_struct_name = check_and_correct_struct_name(normalize_name(tstruct->get_name()));

    out << indent() << "public " << (is_final ? "sealed " : "") << sharp_struct_name << " : ";

    if (is_exception)
    {
        out << " : " << "ApplicationException, ";
    }

    out << indent() << "{" << endl;
    indent_up();

    const vector<t_field*>& members = tstruct->get_members();
    vector<t_field*>::const_iterator m_iter;

    // make private members with public Properties
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        // if the field is required, then we use auto-properties
        if (!field_is_required((*m_iter)))
        {
            out << indent() << "private " << declare_field(*m_iter, false, "_") << endl;
        }
    }
    out << endl;

    bool has_non_required_fields = false;
    bool has_required_fields = false;
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        generate_vala_doc(out, *m_iter);
        generate_property(out, *m_iter, true, true);
        bool is_required = field_is_required((*m_iter));
        if (is_required)
        {
            has_required_fields = true;
        }
        else
        {
            has_non_required_fields = true;
        }
    }

    bool generate_isset = has_non_required_fields;
    if (generate_isset)
    {
        out << endl;
        out << indent() << "public Isset __isset;" << endl;

        out << indent() << "public struct Isset" << endl
            << indent() << "{" << endl;
        indent_up();

        for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
        {
            bool is_required = field_is_required((*m_iter));
            // if it is required, don't need Isset for that variable
            // if it is not required, if it has a default value, we need to generate Isset
            if (!is_required)
            {
                out << indent() << "public bool " << normalize_name((*m_iter)->get_name()) << ";" << endl;
            }
        }

        indent_down();
        out << indent() << "}" << endl << endl;
    }

    // We always want a default, no argument constructor for Reading
    out << indent() << "public " << sharp_struct_name << "()" << endl
        << indent() << "{" << endl;
    indent_up();

    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
    {
        t_type* t = (*m_iter)->get_type();
        while (t->is_typedef())
        {
            t = static_cast<t_typedef*>(t)->get_type();
        }
        if ((*m_iter)->get_value() != NULL)
        {
            if (field_is_required((*m_iter)))
            {
                print_const_value(out, "this." + prop_name(*m_iter), t, (*m_iter)->get_value(), true, true);
            }
            else
            {
                print_const_value(out, "this._" + (*m_iter)->get_name(), t, (*m_iter)->get_value(), true, true);
                // Optionals with defaults are marked set
                out << indent() << "this.__isset." << normalize_name((*m_iter)->get_name()) << " = true;" << endl;
            }
        }
    }
    indent_down();
    out << indent() << "}" << endl << endl;

    if (has_required_fields)
    {
        out << indent() << "public " << sharp_struct_name << "(";
        bool first = true;
        for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
        {
            if (field_is_required(*m_iter))
            {
                if (first)
                {
                    first = false;
                }
                else
                {
                    out << ", ";
                }
                out << type_name((*m_iter)->get_type()) << " " << (*m_iter)->get_name();
            }
        }
        out << ") : this()" << endl
            << indent() << "{" << endl;
        indent_up();

        for (m_iter = members.begin(); m_iter != members.end(); ++m_iter)
        {
            if (field_is_required(*m_iter))
            {
                out << indent() << "this." << prop_name(*m_iter) << " = " << (*m_iter)->get_name() << ";" << endl;
            }
        }

        indent_down();
        out << indent() << "}" << endl << endl;
    }

    generate_vala_struct_reader(out, tstruct);
    if (is_result)
    {
        generate_vala_struct_result_writer(out, tstruct);
    }
    else
    {
        generate_vala_struct_writer(out, tstruct);
    }

    indent_down();
    out << indent() << "}" << endl << endl;

    cleanup_member_name_mapping(tstruct);
    if (!in_class)
    {
        end_vala_namespace(out);
    }
}

void t_vala_generator::generate_vala_struct_reader(ostream& out, t_struct* tstruct)
{
    out << indent() << "public void read(Protocol protocol)" << endl
        << indent() << "{" << endl;
    indent_up();

    const vector<t_field*>& fields = tstruct->get_members();
    vector<t_field*>::const_iterator f_iter;

    // Required variables aren't in __isset, so we need tmp vars to check them
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        if (field_is_required(*f_iter))
        {
            out << indent() << "bool isset_" << (*f_iter)->get_name() << " = false;" << endl;
        }
    }

    out << indent() << "string struct_name;" << endl
        << indent() << "protocol.read_struct_begin(struct_name);" << endl
        << indent() << "while (true)" << endl
        << indent() << "{" << endl;
    indent_up();
    out << indent() << "string field_name;" << endl
        << indent() << "Type field_type;" << endl
        << indent() << "int16 field_id" << endl
        << indent() << "protocol.read_field_begin(field_name, field_type, field_id);" << endl
        << indent() << "if (field_type == Type.STOP)" << endl
        << indent() << "{" << endl;
    indent_up();
    out << indent() << "break;" << endl;
    indent_down();
    out << indent() << "}" << endl << endl
        << indent() << "switch (field_id)" << endl
        << indent() << "{" << endl;
    indent_up();

    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        bool is_required = field_is_required(*f_iter);
        out << indent() << "case " << (*f_iter)->get_key() << ":" << endl;
        indent_up();
        out << indent() << "if (field_type == " << type_to_enum((*f_iter)->get_type()) << ")" << endl
            << indent() << "{" << endl;
        indent_up();

        generate_deserialize_field(out, *f_iter);
        if (is_required)
        {
            out << indent() << "isset_" << (*f_iter)->get_name() << " = true;" << endl;
        }

        indent_down();
        out << indent() << "}" << endl
            << indent() << "else" << endl
            << indent() << "{" << endl;
        indent_up();
        out << indent() << "protocol.skip(field_type);" << endl;
        indent_down();
        out << indent() << "}" << endl
            << indent() << "break;" << endl;
        indent_down();
    }

    out << indent() << "default: " << endl;
    indent_up();
    out << indent() << "protocol.skip(field_type);" << endl
        << indent() << "break;" << endl;
    indent_down();
    indent_down();
    out << indent() << "}" << endl
        << endl
        << indent() << "protocol.read_field_end();" << endl;
    indent_down();
    out << indent() << "}" << endl
        << endl
        << indent() << "protocol.read_struct_end();" << endl;

    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        if (field_is_required((*f_iter)))
        {
            out << indent() << "if (!isset_" << (*f_iter)->get_name() << ")" << endl
                << indent() << "{" << endl;
            indent_up();
            out << indent() << "throw new ProtocolError.INVALID_DATA();" << endl;
            indent_down();
            out << indent() << "}" << endl;
        }
    }

    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_vala_struct_writer(ostream& out, t_struct* tstruct)
{
    out << indent() << "public void write(Protocol protocol)" << endl
        << indent() << "{" << endl;
    indent_up();

    string name = tstruct->get_name();
    const vector<t_field*>& fields = tstruct->get_sorted_members();
    vector<t_field*>::const_iterator f_iter;

    out << indent() << "protocol.write_struct_begin(\"" << name << "\");" << endl;

    if (fields.size() > 0)
    {
        for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
        {
            bool is_required = field_is_required(*f_iter);
            if (!is_required)
            {
                bool null_allowed = type_can_be_null((*f_iter)->get_type());
                if (null_allowed)
                {
                    out << indent() << "if (" << prop_name(*f_iter) << " != null && __isset." << normalize_name((*f_iter)->get_name()) << ")" << endl
                        << indent() << "{" << endl;
                    indent_up();
                }
                else
                {
                    out << indent() << "if (__isset." << normalize_name((*f_iter)->get_name()) << ")" << endl
                        << indent() << "{" << endl;
                    indent_up();
                }
            }
            out << indent() << "protocol.write_field_begin(\"" << (*f_iter)->get_name() << "\", " << type_to_enum((*f_iter)->get_type()) 
                << ", " << (*f_iter)->get_key() << ");" << endl;

            generate_serialize_field(out, *f_iter);

            out << indent() << "protocol.write_field_end();" << endl;
            if (!is_required)
            {
                indent_down();
                out << indent() << "}" << endl;
            }
        }
    }

    out << indent() << "protocol.write_field_stop();" << endl
        << indent() << "protocol.write_struct_end();" << endl;
    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_vala_struct_result_writer(ostream& out, t_struct* tstruct)
{
    out << indent() << "public void write(Protocol protocol)" << endl
        << indent() << "{" << endl;
    indent_up();

    string name = tstruct->get_name();
    const vector<t_field*>& fields = tstruct->get_sorted_members();
    vector<t_field*>::const_iterator f_iter;

    out << indent() << "protocol.write_struct_begin(\"" << name << "\");" << endl;

    if (fields.size() > 0)
    {
        bool first = true;
        for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
        {
            if (first)
            {
                first = false;
                out << endl << indent() << "if";
            }
            else
            {
                out << indent() << "else if";
            }

            out << "(this.__isset." << normalize_name((*f_iter)->get_name()) << ")" << endl
                << indent() << "{" << endl;
            indent_up();

            bool null_allowed = type_can_be_null((*f_iter)->get_type());
            if (null_allowed)
            {
                out << indent() << "if (" << prop_name(*f_iter) << " != null)" << endl
                    << indent() << "{" << endl;
                indent_up();
            }

            out << indent() << "protocol.write_field_begin(\"" << prop_name(*f_iter) << "\", " << type_to_enum((*f_iter)->get_type()) 
                << ", " << (*f_iter)->get_key() << ");" << endl;

            generate_serialize_field(out, *f_iter);

            out << indent() << "protocol.write_field_end();" << endl;

            if (null_allowed)
            {
                indent_down();
                out << indent() << "}" << endl;
            }

            indent_down();
            out << indent() << "}" << endl;
        }
    }

    out << indent() << "protocol.write_field_stop();" << endl
        << indent() << "protocol.write_struct_end();" << endl;
    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_service(t_service* tservice)
{
    int ic = indent_count();

    string f_service_name = namespace_dir_ + "/" + service_name_ + ".vala";
    ofstream_with_content_based_conditional_update f_service;
    f_service.open(f_service_name.c_str());

    f_service << autogen_comment() << vala_type_usings() << vala_thrift_usings() << endl;

    start_vala_namespace(f_service);

    f_service << indent() << "public class " << normalize_name(service_name_) << endl
              << indent() << "{" << endl;
    indent_up();

    generate_service_client(f_service, tservice);
    generate_service_server(f_service, tservice);
    generate_service_helpers(f_service, tservice);

    indent_down();
    f_service << indent() << "}" << endl;

    end_vala_namespace(f_service);
    f_service.close();

    indent_validate(ic, "generate_service.");
}

void t_vala_generator::generate_service_interface(ostream& out, t_service* tservice)
{
    string extends = "";
    string extends_iface = "";
    if (tservice->get_extends() != NULL)
    {
        extends = type_name(tservice->get_extends());
        extends_iface = " : " + extends;
    }

    generate_vala_doc(out, tservice);

    out << indent() << "public interface" << extends_iface << endl
        << indent() << "{" << endl;

    indent_up();
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;
    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        generate_vala_doc(out, *f_iter);

        out << indent() << function_signature_async(*f_iter) << ";" << endl << endl;
    }
    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_service_helpers(ostream& out, t_service* tservice)
{
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        t_struct* ts = (*f_iter)->get_arglist();
        generate_vala_struct_definition(out, ts, false, true);
        generate_function_helpers(out, *f_iter);
    }
}

void t_vala_generator::generate_service_client(ostream& out, t_service* tservice)
{
    string extends = "";
    string extends_client = "";
    if (tservice->get_extends() != NULL)
    {
        extends = type_name(tservice->get_extends());
        extends_client = " : " +extends + ".Client, ";
    }
    else
    {
        extends_client = "";
    }

    out << endl;

    generate_vala_doc(out, tservice);

    out << indent() << "public class Client" << extends_client << endl
        << indent() << "{" << endl;
    indent_up();

    out << indent() << "public Client(Protocol protocol)" << endl
        << indent() << "{" << endl;
    indent_up();

    out << indent() << "this.with_protocols(protocol, protocol);" << endl;
    indent_down();

    out << indent() << "}" << endl
        << endl
        << indent() << "public Client.with_protocols(Protocol input_protocol, Protocol output_protocol)" << endl
        << indent() << "{" << endl;
    indent_up();

    out << indent() << "base(protocol, protocol);" << endl;
    indent_down();

    out << indent() << "}" << endl;

    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::const_iterator functions_iterator;

    for (functions_iterator = functions.begin(); functions_iterator != functions.end(); ++functions_iterator)
    {
        string function_name = (*functions_iterator)->get_name();
        //string function_name = correct_function_name_for_async((*functions_iterator)->get_name());

        // async
        //out << indent() << "public async " << function_signature_async(*functions_iterator, "") << endl
        out << indent() << "public " << *functions_iterator << endl
            << indent() << "{" << endl;
        indent_up();

        string argsname = (*functions_iterator)->get_name() + "Args";

        out << indent() << "output_protocol.write_message_begin(\"" << function_name
            << "\", " << ((*functions_iterator)->is_oneway() ? "MessageType.ONEWAY" : "MessageType.CALL") << ", seqid);" << endl
            << indent() << endl
            << indent() << "var args = new " << argsname << "();" << endl;

        t_struct* arg_struct = (*functions_iterator)->get_arglist();
        prepare_member_name_mapping(arg_struct);
        const vector<t_field*>& fields = arg_struct->get_members();
        vector<t_field*>::const_iterator fld_iter;

        for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter)
        {
            out << indent() << "args." << prop_name(*fld_iter) << " = " << normalize_name((*fld_iter)->get_name()) << ";" << endl;
        }

        out << indent() << endl
            << indent() << "args.write(OutputProtocol);" << endl
            << indent() << "output_protocol.write_message_end();" << endl
            << indent() << "output_protocol.transport.flush();" << endl;

        if (!(*functions_iterator)->is_oneway())
        {
            string resultname = (*functions_iterator)->get_name() + "Result";
            t_struct noargs(program_);
            t_struct* xs = (*functions_iterator)->get_xceptions();
            prepare_member_name_mapping(xs, xs->get_members(), resultname);

            out << indent() << endl
                << indent() << "string name;" << endl
                << indent() << "MessageType message_type;" << endl
                << indent() << "int32 seqid;" << endl
                << indent() << "input_protcol.read_message_begin(name, message_type, seqid);" << endl
                << indent() << "if (message_type == MessageType.EXCEPTION)" << endl
                << indent() << "{" << endl;
            indent_up();

            out << indent() << "var x = await TApplicationException.ReadAsync(InputProtocol, cancellationToken);" << endl
                << indent() << "await InputProtocol.ReadMessageEndAsync(cancellationToken);" << endl
                << indent() << "throw x;" << endl;
            indent_down();

            out << indent() << "}" << endl
                << endl
                << indent() << "var result = new " << resultname << "();" << endl
                << indent() << "await result.ReadAsync(InputProtocol, cancellationToken);" << endl
                << indent() << "await InputProtocol.ReadMessageEndAsync(cancellationToken);" << endl;

            if (!(*functions_iterator)->get_returntype()->is_void())
            {
                out << indent() << "if (result.__isset.success)" << endl
                    << indent() << "{" << endl;
                indent_up();
                out << indent() << "return result.Success;" << endl;
                indent_down();
                out << indent() << "}" << endl;
            }

            const vector<t_field*>& xceptions = xs->get_members();
            vector<t_field*>::const_iterator x_iter;
            for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter)
            {
                out << indent() << "if (result.__isset." << normalize_name((*x_iter)->get_name()) << ")" << endl
                    << indent() << "{" << endl;
                indent_up();
                out << indent() << "throw result." << prop_name(*x_iter) << ";" << endl;
                indent_down();
                out << indent() << "}" << endl;
            }

            if ((*functions_iterator)->get_returntype()->is_void())
            {
                out << indent() << "return;" << endl;
            }
            else
            {
                out << indent() << "throw new ApplicationExceptionError.MISSING_RESULT(\""
                    << function_name << " failed: unknown result\");" << endl;
            }

            cleanup_member_name_mapping((*functions_iterator)->get_xceptions());
            indent_down();
            out << indent() << "}" << endl << endl;
        }
        else
        {
            indent_down();
            out << indent() << "}" << endl;
        }
    }

    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_service_server(ostream& out, t_service* tservice)
{
    vector<t_function*> functions = tservice->get_functions();
    vector<t_function*>::iterator f_iter;

    string extends = "";
    if (tservice->get_extends() != NULL)
    {
        extends = type_name(tservice->get_extends());
    }

    out << indent() << "public class Processor : " << extends << endl
        << indent() << "{" << endl;

    indent_up();

    out << indent() << "private IAsync _iAsync;" << endl
        << endl
        << indent() << "public AsyncProcessor(IAsync iAsync)";

    if (!extends.empty())
    {
        out << " : base(iAsync)";
    }

    out << endl
        << indent() << "{" << endl;
    indent_up();

    out << indent() << "if (iAsync == null) throw new ArgumentNullException(nameof(iAsync));" << endl
        << endl
        << indent() << "_iAsync = iAsync;" << endl;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        string function_name = (*f_iter)->get_name();
        out << indent() << "processMap_[\"" << correct_function_name_for_async(function_name) << "\"] = " << function_name << "_ProcessAsync;" << endl;
    }

    indent_down();
    out << indent() << "}" << endl
        << endl;

    if (extends.empty())
    {
        out << indent() << "protected delegate Task ProcessFunction(int seqid, TProtocol iprot, TProtocol oprot, CancellationToken cancellationToken);" << endl;
    }

    if (extends.empty())
    {
        out << indent() << "protected Dictionary<string, ProcessFunction> processMap_ = new Dictionary<string, ProcessFunction>();" << endl;
    }

    out << endl;

    if (extends.empty())
    {
        out << indent() << "public async Task<bool> ProcessAsync(TProtocol iprot, TProtocol oprot)" << endl
            << indent() << "{" << endl;
        indent_up();
        out << indent() << "return await ProcessAsync(iprot, oprot, CancellationToken.None);" << endl;
        indent_down();
        out << indent() << "}" << endl << endl;

        out << indent() << "public async Task<bool> ProcessAsync(TProtocol iprot, TProtocol oprot, CancellationToken cancellationToken)" << endl;
    }
    else
    {
        out << indent() << "public new async Task<bool> ProcessAsync(TProtocol iprot, TProtocol oprot)" << endl
            << indent() << "{" << endl;
        indent_up();
        out << indent() << "return await ProcessAsync(iprot, oprot, CancellationToken.None);" << endl;
        indent_down();
        out << indent() << "}" << endl << endl;

        out << indent() << "public new async Task<bool> ProcessAsync(TProtocol iprot, TProtocol oprot, CancellationToken cancellationToken)" << endl;
    }

    out << indent() << "{" << endl;
    indent_up();
    out << indent() << "try" << endl
        << indent() << "{" << endl;
    indent_up();
    out << indent() << "var msg = await iprot.ReadMessageBeginAsync(cancellationToken);" << endl
        << endl
        << indent() << "ProcessFunction fn;" << endl
        << indent() << "processMap_.TryGetValue(msg.Name, out fn);" << endl
        << endl
        << indent() << "if (fn == null)" << endl
        << indent() << "{" << endl;
    indent_up();
    out << indent() << "await TProtocolUtil.SkipAsync(iprot, TType.Struct, cancellationToken);" << endl
        << indent() << "await iprot.ReadMessageEndAsync(cancellationToken);" << endl
        << indent() << "var x = new TApplicationException (TApplicationException.ExceptionType.UnknownMethod, \"Invalid method name: '\" + msg.Name + \"'\");" << endl
        << indent() << "await oprot.WriteMessageBeginAsync(new TMessage(msg.Name, TMessageType.Exception, msg.SeqID), cancellationToken);" << endl
        << indent() << "await x.WriteAsync(oprot, cancellationToken);" << endl
        << indent() << "await oprot.WriteMessageEndAsync(cancellationToken);" << endl
        << indent() << "await oprot.Transport.FlushAsync(cancellationToken);" << endl
        << indent() << "return true;" << endl;
    indent_down();
    out << indent() << "}" << endl
        << endl
        << indent() << "await fn(msg.SeqID, iprot, oprot, cancellationToken);" << endl
        << endl;
    indent_down();
    out << indent() << "}" << endl;
    out << indent() << "catch (IOException)" << endl
        << indent() << "{" << endl;
    indent_up();
    out << indent() << "return false;" << endl;
    indent_down();
    out << indent() << "}" << endl
        << endl
        << indent() << "return true;" << endl;
    indent_down();
    out << indent() << "}" << endl << endl;

    for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter)
    {
        generate_process_function_async(out, tservice, *f_iter);
    }

    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_function_helpers(ostream& out, t_function* tfunction)
{
    if (tfunction->is_oneway())
    {
        return;
    }

    t_struct result(program_, tfunction->get_name() + "_result");
    t_field success(tfunction->get_returntype(), "success", 0);
    if (!tfunction->get_returntype()->is_void())
    {
        result.append(&success);
    }

    t_struct* xs = tfunction->get_xceptions();
    const vector<t_field*>& fields = xs->get_members();
    vector<t_field*>::const_iterator f_iter;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        result.append(*f_iter);
    }

    generate_vala_struct_definition(out, &result, false, true, true);
}

void t_vala_generator::generate_process_function_async(ostream& out, t_service* tservice, t_function* tfunction)
{
    (void)tservice;
    out << indent() << "public async Task " << tfunction->get_name()
        << "_ProcessAsync(int seqid, TProtocol iprot, TProtocol oprot, CancellationToken cancellationToken)" << endl
        << indent() << "{" << endl;
    indent_up();

    string argsname = tfunction->get_name() + "Args";
    string resultname = tfunction->get_name() + "Result";

    out << indent() << "var args = new " << argsname << "();" << endl
        << indent() << "await args.ReadAsync(iprot, cancellationToken);" << endl
        << indent() << "await iprot.ReadMessageEndAsync(cancellationToken);" << endl;

    if (!tfunction->is_oneway())
    {
        out << indent() << "var result = new " << resultname << "();" << endl;
    }

    out << indent() << "try" << endl
        << indent() << "{" << endl;
    indent_up();

    t_struct* xs = tfunction->get_xceptions();
    const vector<t_field*>& xceptions = xs->get_members();

    if (xceptions.size() > 0)
    {
        out << indent() << "try" << endl
            << indent() << "{" << endl;
        indent_up();
    }

    t_struct* arg_struct = tfunction->get_arglist();
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator f_iter;

    out << indent();
    if (!tfunction->is_oneway() && !tfunction->get_returntype()->is_void())
    {
        out << "result.Success = ";
    }

    out << "await _iAsync." << normalize_name(tfunction->get_name()) << "Async(";

    bool first = true;
    prepare_member_name_mapping(arg_struct);
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            out << ", ";
        }

        out << "args." << prop_name(*f_iter);
    }

    cleanup_member_name_mapping(arg_struct);

    if (!first)
    {
        out << ", ";
    }

    out << "cancellationToken);" << endl;

    vector<t_field*>::const_iterator x_iter;

    prepare_member_name_mapping(xs, xs->get_members(), resultname);
    if (xceptions.size() > 0)
    {
        indent_down();
        out << indent() << "}" << endl;

        for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter)
        {
            out << indent() << "catch (" << type_name((*x_iter)->get_type()) << " " << (*x_iter)->get_name() << ")" << endl
                << indent() << "{" << endl;

            if (!tfunction->is_oneway())
            {
                indent_up();
                out << indent() << "result." << prop_name(*x_iter) << " = " << (*x_iter)->get_name() << ";" << endl;
                indent_down();
            }
            out << indent() << "}" << endl;
        }
    }

    if (!tfunction->is_oneway())
    {
        out << indent() << "await oprot.WriteMessageBeginAsync(new TMessage(\""
                << correct_function_name_for_async(tfunction->get_name()) << "\", TMessageType.Reply, seqid), cancellationToken); " << endl
            << indent() << "await result.WriteAsync(oprot, cancellationToken);" << endl;
    }
    indent_down();

    cleanup_member_name_mapping(xs);

    out << indent() << "}" << endl
        << indent() << "catch (TTransportException)" << endl
        << indent() << "{" << endl
        << indent() << "  throw;" << endl
        << indent() << "}" << endl
        << indent() << "catch (Exception ex)" << endl
        << indent() << "{" << endl;
    indent_up();

    out << indent() << "Console.Error.WriteLine(\"Error occurred in processor:\");" << endl
        << indent() << "Console.Error.WriteLine(ex.ToString());" << endl;

    if (tfunction->is_oneway())
    {
        indent_down();
        out << indent() << "}" << endl;
    }
    else
    {
        out << indent() << "var x = new TApplicationException(TApplicationException.ExceptionType.InternalError,\" Internal error.\");" << endl
            << indent() << "await oprot.WriteMessageBeginAsync(new TMessage(\"" << correct_function_name_for_async(tfunction->get_name())
            << "\", TMessageType.Exception, seqid), cancellationToken);" << endl
            << indent() << "await x.WriteAsync(oprot, cancellationToken);" << endl;
        indent_down();

        out << indent() << "}" << endl
            << indent() << "await oprot.WriteMessageEndAsync(cancellationToken);" << endl
            << indent() << "await oprot.Transport.FlushAsync(cancellationToken);" << endl;
    }

    indent_down();
    out << indent() << "}" << endl << endl;
}

void t_vala_generator::generate_deserialize_field(ostream& out, t_field* tfield, string prefix, bool is_propertyless)
{
    t_type* type = tfield->get_type();
    while (type->is_typedef())
    {
        type = static_cast<t_typedef*>(type)->get_type();
    }

    if (type->is_void())
    {
        throw "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " + prefix + tfield->get_name();
    }

    string name = prefix + (is_propertyless ? "" : prop_name(tfield));

    if (type->is_struct() || type->is_xception())
    {
        generate_deserialize_struct(out, static_cast<t_struct*>(type), name);
    }
    else if (type->is_container())
    {
        generate_deserialize_container(out, type, name);
    }
    else if (type->is_base_type() || type->is_enum())
    {
        out << indent() << name << " = ";

        if (type->is_enum())
        {
            out << "(" << type_name(type) << ")";
        }

        out << "protocol";

        if (type->is_base_type())
        {
            t_base_type::t_base tbase = static_cast<t_base_type*>(type)->get_base();
            switch (tbase)
            {
            case t_base_type::TYPE_VOID:
                throw "compiler error: cannot serialize void field in a struct: " + name;
                break;
            case t_base_type::TYPE_STRING:
                if (type->is_binary())
                {
                    out << "read_binary(buf, len);";
                }
                else
                {
                    out << "read_string(str);";
                }
                break;
            case t_base_type::TYPE_BOOL:
                out << "read_bool(value);";
                break;
            case t_base_type::TYPE_I8:
                out << "read_byte(value);";
                break;
            case t_base_type::TYPE_I16:
                out << "read_i16(value);";
                break;
            case t_base_type::TYPE_I32:
                out << "read_i32(value);";
                break;
            case t_base_type::TYPE_I64:
                out << "read_i64(value);";
                break;
            case t_base_type::TYPE_DOUBLE:
                out << "read_double(value);";
                break;
            default:
                throw "compiler error: no Vala name for base type " + t_base_type::t_base_name(tbase);
            }
        }
        else if (type->is_enum())
        {
            out << "read_i32(value);";
        }
        out << endl;
    }
    else
    {
        printf("DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n", tfield->get_name().c_str(), type_name(type).c_str());
    }
}

void t_vala_generator::generate_deserialize_struct(ostream& out, t_struct* tstruct, string prefix)
{
    out << indent() << prefix << " = new " << type_name(tstruct) << "();" << endl
            << indent() << "await " << prefix << ".ReadAsync(iprot, cancellationToken);" << endl;
}

void t_vala_generator::generate_deserialize_container(ostream& out, t_type* ttype, string prefix)
{
    out << indent() << "{" << endl;
    indent_up();

    string obj;

    if (ttype->is_map())
    {
        obj = tmp("_map");
    }
    else if (ttype->is_set())
    {
        obj = tmp("_set");
    }
    else if (ttype->is_list())
    {
        obj = tmp("_list");
    }

    if (ttype->is_map())
    {
        out << indent() << "protocol.read_map_begin(key_type, value_type, size);" << endl;
    }
    else if (ttype->is_set())
    {
        out << indent() << "protocol.read_set_begin(element_type, size);" << endl;
    }
    else if (ttype->is_list())
    {
        out << indent() << "protocol.read_list_begin(element_type, size);" << endl;
    }

    out << indent() << prefix << " = new " << type_name(ttype) << "(" << obj << ".Count);" << endl;
    string i = tmp("_i");
    out << indent() << "for(int " << i << " = 0; " << i << " < " << obj << ".Count; ++" << i << ")" << endl
        << indent() << "{" << endl;
    indent_up();

    if (ttype->is_map())
    {
        generate_deserialize_map_element(out, static_cast<t_map*>(ttype), prefix);
    }
    else if (ttype->is_set())
    {
        generate_deserialize_set_element(out, static_cast<t_set*>(ttype), prefix);
    }
    else if (ttype->is_list())
    {
        generate_deserialize_list_element(out, static_cast<t_list*>(ttype), prefix);
    }

    indent_down();
    out << indent() << "}" << endl;

    if (ttype->is_map())
    {
        out << indent() << "protocol.read_map_end();" << endl;
    }
    else if (ttype->is_set())
    {
        out << indent() << "protocol.read_set_end();" << endl;
    }
    else if (ttype->is_list())
    {
        out << indent() << "protocol.read_list_end();" << endl;
    }

    indent_down();
    out << indent() << "}" << endl;
}

void t_vala_generator::generate_deserialize_map_element(ostream& out, t_map* tmap, string prefix)
{
    string key = tmp("_key");
    string val = tmp("_val");

    t_field fkey(tmap->get_key_type(), key);
    t_field fval(tmap->get_val_type(), val);

    out << indent() << declare_field(&fkey) << endl;
    out << indent() << declare_field(&fval) << endl;

    generate_deserialize_field(out, &fkey);
    generate_deserialize_field(out, &fval);

    out << indent() << prefix << "[" << key << "] = " << val << ";" << endl;
}

void t_vala_generator::generate_deserialize_set_element(ostream& out, t_set* tset, string prefix)
{
    string elem = tmp("_elem");
    t_field felem(tset->get_elem_type(), elem);

    out << indent() << declare_field(&felem) << endl;

    generate_deserialize_field(out, &felem);

    out << indent() << prefix << ".Add(" << elem << ");" << endl;
}

void t_vala_generator::generate_deserialize_list_element(ostream& out, t_list* tlist, string prefix)
{
    string elem = tmp("_elem");
    t_field felem(tlist->get_elem_type(), elem);

    out << indent() << declare_field(&felem) << endl;

    generate_deserialize_field(out, &felem);

    out << indent() << prefix << ".Add(" << elem << ");" << endl;
}

void t_vala_generator::generate_serialize_field(ostream& out, t_field* tfield, string prefix, bool is_propertyless)
{
    t_type* type = tfield->get_type();
    while (type->is_typedef())
    {
        type = static_cast<t_typedef*>(type)->get_type();
    }

    string name = prefix + (is_propertyless ? "" : prop_name(tfield));

    if (type->is_void())
    {
        throw "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + name;
    }

    if (type->is_struct() || type->is_xception())
    {
        generate_serialize_struct(out, static_cast<t_struct*>(type), name);
    }
    else if (type->is_container())
    {
        generate_serialize_container(out, type, name);
    }
    else if (type->is_base_type() || type->is_enum())
    {
        out << indent() << "protocol.";

        string nullable_name = name;

        if (type->is_base_type())
        {
            t_base_type::t_base tbase = static_cast<t_base_type*>(type)->get_base();
            switch (tbase)
            {
            case t_base_type::TYPE_VOID:
                throw "compiler error: cannot serialize void field in a struct: " + name;
            case t_base_type::TYPE_STRING:
                if (type->is_binary())
                {
                    out << "write_binary(";
                }
                else
                {
                    out << "write_string(";
                }
                out << name << ");";
                break;
            case t_base_type::TYPE_BOOL:
                out << "write_bool(" << nullable_name << ");";
                break;
            case t_base_type::TYPE_I8:
                out << "write_byte(" << nullable_name << ");";
                break;
            case t_base_type::TYPE_I16:
                out << "write_i16(" << nullable_name << ");";
                break;
            case t_base_type::TYPE_I32:
                out << "write_i32(" << nullable_name << ", cancellationToken);";
                break;
            case t_base_type::TYPE_I64:
                out << "write_i64(" << nullable_name << ");";
                break;
            case t_base_type::TYPE_DOUBLE:
                out << "write_double(" << nullable_name << ");";
                break;
            default:
                throw "compiler error: no Vala name for base type " + t_base_type::t_base_name(tbase);
            }
        }
        else if (type->is_enum())
        {
            out << "write_i32(" << nullable_name << ");";
        }
        out << endl;
    }
    else
    {
        printf("DO NOT KNOW HOW TO SERIALIZE '%s%s' TYPE '%s'\n", prefix.c_str(), tfield->get_name().c_str(), type_name(type).c_str());
    }
}

void t_vala_generator::generate_serialize_struct(ostream& out, t_struct* tstruct, string prefix)
{
    (void)tstruct;
    out << indent() << prefix << ".WriteAsync(oprot, cancellationToken);" << endl;
}

void t_vala_generator::generate_serialize_container(ostream& out, t_type* ttype, string prefix)
{
    out << indent() << "{" << endl;
    indent_up();

    if (ttype->is_map())
    {
        out << indent() << "protocol.write_map_begin(new HashMap(" << type_to_enum(static_cast<t_map*>(ttype)->get_key_type())
            << ", " << type_to_enum(static_cast<t_map*>(ttype)->get_val_type()) << ", " << prefix
            << ".Count));" << endl;
    }
    else if (ttype->is_set())
    {
        out << indent() << "protocol.write_set_begin(new HashSet(" << type_to_enum(static_cast<t_set*>(ttype)->get_elem_type())
            << ", " << prefix << ".Count));" << endl;
    }
    else if (ttype->is_list())
    {
        out << indent() << "protocol.write_list_begin(new ArrayList("
            << type_to_enum(static_cast<t_list*>(ttype)->get_elem_type()) << ", " << prefix << ".Count));"
            << endl;
    }

    string iter = tmp("_iter");
    if (ttype->is_map())
    {
        out << indent() << "foreach (" << type_name(static_cast<t_map*>(ttype)->get_key_type()) << " " << iter
            << " in " << prefix << ".Keys)";
    }
    else if (ttype->is_set())
    {
        out << indent() << "foreach (" << type_name(static_cast<t_set*>(ttype)->get_elem_type()) << " " << iter
            << " in " << prefix << ")";
    }
    else if (ttype->is_list())
    {
        out << indent() << "foreach (" << type_name(static_cast<t_list*>(ttype)->get_elem_type()) << " " << iter
            << " in " << prefix << ")";
    }

    out << endl;
    out << indent() << "{" << endl;
    indent_up();

    if (ttype->is_map())
    {
        generate_serialize_map_element(out, static_cast<t_map*>(ttype), iter, prefix);
    }
    else if (ttype->is_set())
    {
        generate_serialize_set_element(out, static_cast<t_set*>(ttype), iter);
    }
    else if (ttype->is_list())
    {
        generate_serialize_list_element(out, static_cast<t_list*>(ttype), iter);
    }

    indent_down();
    out << indent() << "}" << endl;

    if (ttype->is_map())
    {
        out << indent() << "protocol.write_map_end();" << endl;
    }
    else if (ttype->is_set())
    {
        out << indent() << "protocol.write_set_end();" << endl;
    }
    else if (ttype->is_list())
    {
        out << indent() << "protocol.write_list_end();" << endl;
    }

    indent_down();
    out << indent() << "}" << endl;
}

void t_vala_generator::generate_serialize_map_element(ostream& out, t_map* tmap, string iter, string map)
{
    t_field kfield(tmap->get_key_type(), iter);
    generate_serialize_field(out, &kfield, "");
    t_field vfield(tmap->get_val_type(), map + "[" + iter + "]");
    generate_serialize_field(out, &vfield, "");
}

void t_vala_generator::generate_serialize_set_element(ostream& out, t_set* tset, string iter)
{
    t_field efield(tset->get_elem_type(), iter);
    generate_serialize_field(out, &efield, "");
}

void t_vala_generator::generate_serialize_list_element(ostream& out, t_list* tlist, string iter)
{
    t_field efield(tlist->get_elem_type(), iter);
    generate_serialize_field(out, &efield, "");
}

void t_vala_generator::generate_property(ostream& out, t_field* tfield, bool isPublic, bool generateIsset)
{
    generate_vala_property(out, tfield, isPublic, generateIsset, "_");
}

void t_vala_generator::generate_vala_property(ostream& out, t_field* tfield, bool isPublic, bool generateIsset, string fieldPrefix)
{
    bool is_required = field_is_required(tfield);
    if (is_required)
    {
        out << indent() << (isPublic ? "public " : "private ") << type_name(tfield->get_type()) << " " << prop_name(tfield) << " { get; set; }" << endl;
    }
    else
    {
        out << indent() << (isPublic ? "public " : "private ")  << type_name(tfield->get_type()) << " " << prop_name(tfield) << endl
            << indent() << "{" << endl;
        indent_up();

        out << indent() << "get" << endl
            << indent() << "{" << endl;
        indent_up();

        bool use_nullable = false;

        out << indent() << "return " << fieldPrefix + tfield->get_name() << ";" << endl;
        indent_down();
        out << indent() << "}" << endl
            << indent() << "set" << endl
            << indent() << "{" << endl;
        indent_up();

        if (use_nullable)
        {
            if (generateIsset)
            {
                out << indent() << "__isset." << normalize_name(tfield->get_name()) << " = value.HasValue;" << endl;
            }
            out << indent() << "if (value.HasValue) this." << fieldPrefix + tfield->get_name() << " = value.Value;" << endl;
        }
        else
        {
            if (generateIsset)
            {
                out << indent() << "__isset." << normalize_name(tfield->get_name()) << " = true;" << endl;
            }
            out << indent() << "this." << fieldPrefix + tfield->get_name() << " = value;" << endl;
        }

        indent_down();
        out << indent() << "}" << endl;
        indent_down();
        out << indent() << "}" << endl;
    }
    out << endl;
}

string t_vala_generator::make_valid_vala_identifier(string const& fromName)
{
    string str = fromName;
    if (str.empty())
    {
        return str;
    }

    // tests rely on this
    assert(('A' < 'Z') && ('a' < 'z') && ('0' < '9'));

    // if the first letter is a number, we add an additional underscore in front of it
    char c = str.at(0);
    if (('0' <= c) && (c <= '9'))
    {
        str = "_" + str;
    }

    // following chars: letter, number or underscore
    for (size_t i = 0; i < str.size(); ++i)
    {
        c = str.at(i);
        if (('A' > c || c > 'Z') && ('a' > c || c > 'z') && ('0' > c || c > '9') && '_' != c)
        {
            str.replace(i, 1, "_");
        }
    }

    return str;
}

void t_vala_generator::cleanup_member_name_mapping(void* scope)
{
    if (member_mapping_scopes.empty())
    {
        throw "internal error: cleanup_member_name_mapping() no scope active";
    }

    member_mapping_scope& active = member_mapping_scopes.back();
    if (active.scope_member != scope)
    {
        throw "internal error: cleanup_member_name_mapping() called for wrong struct";
    }

    member_mapping_scopes.pop_back();
}

string t_vala_generator::get_mapped_member_name(string name)
{
    if (!member_mapping_scopes.empty())
    {
        member_mapping_scope& active = member_mapping_scopes.back();
        map<string, string>::iterator iter = active.mapping_table.find(name);
        if (active.mapping_table.end() != iter)
        {
            return iter->second;
        }
    }

    pverbose("no mapping for member %s\n", name.c_str());
    return name;
}

void t_vala_generator::prepare_member_name_mapping(t_struct* tstruct)
{
    prepare_member_name_mapping(tstruct, tstruct->get_members(), tstruct->get_name());
}

void t_vala_generator::prepare_member_name_mapping(void* scope, const vector<t_field*>& members, const string& structname)
{
    // begin new scope
    member_mapping_scopes.emplace_back();
    member_mapping_scope& active = member_mapping_scopes.back();
    active.scope_member = scope;

    // current Vala generator policy:
    // - prop names are always rendered with an Uppercase first letter
    // - struct names are used as given
    std::set<string> used_member_names;
    vector<t_field*>::const_iterator iter;

    // prevent name conflicts with struct (CS0542 error)
    used_member_names.insert(structname);

    // prevent name conflicts with known methods (THRIFT-2942)
    used_member_names.insert("Read");
    used_member_names.insert("Write");

    for (iter = members.begin(); iter != members.end(); ++iter)
    {
        string oldname = (*iter)->get_name();
        string newname = prop_name(*iter, true);
        while (true)
        {
            // new name conflicts with another member
            if (used_member_names.find(newname) != used_member_names.end())
            {
                pverbose("struct %s: member %s conflicts with another member\n", structname.c_str(), newname.c_str());
                newname += '_';
                continue;
            }

            // add always, this helps us to detect edge cases like
            // different spellings ("foo" and "Foo") within the same struct
            pverbose("struct %s: member mapping %s => %s\n", structname.c_str(), oldname.c_str(), newname.c_str());
            active.mapping_table[oldname] = newname;
            used_member_names.insert(newname);
            break;
        }
    }
}


string t_vala_generator::convert_to_pascal_case(const string& str) {
  string out;
  bool must_capitalize = true;
  bool first_character = true;
  for (auto it = str.begin(); it != str.end(); ++it) {
    if (std::isalnum(*it)) {
      if (must_capitalize) {
        out.append(1, (char)::toupper(*it));
        must_capitalize = false;
      } else {
        out.append(1, *it);
      }
    } else {
      if (first_character) //this is a private variable and should not be PascalCased
        return str;
      must_capitalize = true;
    }
    first_character = false;
  }
  return out;
}


string t_vala_generator::prop_name(t_field* tfield, bool suppress_mapping) {
  string name(tfield->get_name());
  if (suppress_mapping) {
    name[0] = toupper(name[0]);
    if (use_pascal_case_properties)
      name = t_vala_generator::convert_to_pascal_case(name);
  } else {
    name = get_mapped_member_name(name);
  }

  return name;
}

string t_vala_generator::type_name(t_type* ttype)
{
    while (ttype->is_typedef())
    {
        ttype = static_cast<t_typedef*>(ttype)->get_type();
    }

    if (ttype->is_base_type())
    {
        return base_type_name(static_cast<t_base_type*>(ttype));
    }

    if (ttype->is_map())
    {
        t_map* tmap = static_cast<t_map*>(ttype);
        return "HashMap<" + type_name(tmap->get_key_type()) + ", " + type_name(tmap->get_val_type()) + ">";
    }

    if (ttype->is_set())
    {
        t_set* tset = static_cast<t_set*>(ttype);
        return "HashSet<" + type_name(tset->get_elem_type()) + ">";
    }

    if (ttype->is_list())
    {
        t_list* tlist = static_cast<t_list*>(ttype);
        return "ArrayList<" + type_name(tlist->get_elem_type()) + ">";
    }

    t_program* program = ttype->get_program();
    if (program != NULL && program != program_)
    {
        string ns = program->get_namespace("vala");
        if (!ns.empty())
        {
            return ns + "." + normalize_name(ttype->get_name());
        }
    }

    return normalize_name(ttype->get_name());
}

string t_vala_generator::base_type_name(t_base_type* tbase)
{
    switch (tbase->get_base())
    {
    case t_base_type::TYPE_VOID:
        return "void";
    case t_base_type::TYPE_STRING:
        {
            if (tbase->is_binary())
            {
                return "byte[]";
            }
            return "string";
        }
    case t_base_type::TYPE_BOOL:
        return "bool";
    case t_base_type::TYPE_I8:
        return "int8";
    case t_base_type::TYPE_I16:
        return "int16";
    case t_base_type::TYPE_I32:
        return "int32";
    case t_base_type::TYPE_I64:
        return "int64";
    case t_base_type::TYPE_DOUBLE:
        return "double";
    default:
        throw "compiler error: no Vala name for base type " + t_base_type::t_base_name(tbase->get_base());
    }
}

string t_vala_generator::declare_field(t_field* tfield, bool init, string prefix)
{
    string result = type_name(tfield->get_type()) + " " + prefix + tfield->get_name();
    if (init)
    {
        t_type* ttype = tfield->get_type();
        while (ttype->is_typedef())
        {
            ttype = static_cast<t_typedef*>(ttype)->get_type();
        }
        if (ttype->is_base_type() && field_has_default(tfield))
        {
            std::ofstream dummy;
            result += " = " + render_const_value(dummy, tfield->get_name(), ttype, tfield->get_value());
        }
        else if (ttype->is_base_type())
        {
            t_base_type::t_base tbase = static_cast<t_base_type*>(ttype)->get_base();
            switch (tbase)
            {
            case t_base_type::TYPE_VOID:
                throw "NO T_VOID CONSTRUCT";
            case t_base_type::TYPE_STRING:
                result += " = null";
                break;
            case t_base_type::TYPE_BOOL:
                result += " = false";
                break;
            case t_base_type::TYPE_I8:
            case t_base_type::TYPE_I16:
            case t_base_type::TYPE_I32:
            case t_base_type::TYPE_I64:
                result += " = 0";
                break;
            case t_base_type::TYPE_DOUBLE:
                result += " = (double)0";
                break;
            }
        }
        else if (ttype->is_enum())
        {
            result += " = (" + type_name(ttype) + ")0";
        }
        else if (ttype->is_container())
        {
            result += " = new " + type_name(ttype) + "()";
        }
        else
        {
            result += " = new " + type_name(ttype) + "()";
        }
    }
    return result + ";";
}

string t_vala_generator::function_signature(t_function* tfunction, string prefix)
{
    t_type* ttype = tfunction->get_returntype();
    return type_name(ttype) + " " + normalize_name(prefix + tfunction->get_name()) + "(" + argument_list(tfunction->get_arglist()) + ")";
}

string t_vala_generator::function_signature_async(t_function* tfunction, string prefix)
{
    t_type* ttype = tfunction->get_returntype();

    string result = type_name(ttype) + " " + normalize_name(prefix + tfunction->get_name()) + "(";
    string args = argument_list(tfunction->get_arglist());
    result += args;

    return result;
}

string t_vala_generator::argument_list(t_struct* tstruct)
{
    string result = "";
    const vector<t_field*>& fields = tstruct->get_members();
    vector<t_field*>::const_iterator f_iter;
    bool first = true;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            result += ", ";
        }
        result += type_name((*f_iter)->get_type()) + " " + normalize_name((*f_iter)->get_name());
    }
    return result;
}

string t_vala_generator::type_to_enum(t_type* type)
{
    while (type->is_typedef())
    {
        type = static_cast<t_typedef*>(type)->get_type();
    }

    if (type->is_base_type())
    {
        t_base_type::t_base tbase = static_cast<t_base_type*>(type)->get_base();
        switch (tbase)
        {
        case t_base_type::TYPE_VOID:
            throw "Type.VOID";
        case t_base_type::TYPE_STRING:
            return "Type.STRING";
        case t_base_type::TYPE_BOOL:
            return "Type.BOOL";
        case t_base_type::TYPE_I8:
            return "Type.BYTE";
        case t_base_type::TYPE_I16:
            return "Type.I16";
        case t_base_type::TYPE_I32:
            return "Type.I32";
        case t_base_type::TYPE_I64:
            return "Type.I64";
        case t_base_type::TYPE_DOUBLE:
            return "Type.DOUBLE";
        }
    }
    else if (type->is_enum())
    {
        return "Type.I32";
    }
    else if (type->is_struct() || type->is_xception())
    {
        return "Type.STRUCT";
    }
    else if (type->is_map())
    {
        return "Type.MAP";
    }
    else if (type->is_set())
    {
        return "Type.SET";
    }
    else if (type->is_list())
    {
        return "Type.LIST";
    }

    throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}

void t_vala_generator::generate_vala_docstring_comment(ostream& out, string contents)
{
    docstring_comment(out, "/// <summary>" + endl, "/// ", contents, "/// </summary>" + endl);
}

void t_vala_generator::generate_vala_doc(ostream& out, t_field* field)
{
    if (field->get_type()->is_enum())
    {
        string combined_message = field->get_doc() + endl + "<seealso cref=\"" + get_enum_class_name(field->get_type()) + "\"/>";
        generate_vala_docstring_comment(out, combined_message);
    }
    else
    {
        generate_vala_doc(out, static_cast<t_doc*>(field));
    }
}

void t_vala_generator::generate_vala_doc(ostream& out, t_doc* tdoc)
{
    if (tdoc->has_doc())
    {
        generate_vala_docstring_comment(out, tdoc->get_doc());
    }
}

void t_vala_generator::generate_vala_doc(ostream& out, t_function* tfunction)
{
    if (tfunction->has_doc())
    {
        stringstream ps;
        const vector<t_field*>& fields = tfunction->get_arglist()->get_members();
        vector<t_field*>::const_iterator p_iter;
        for (p_iter = fields.begin(); p_iter != fields.end(); ++p_iter)
        {
            t_field* p = *p_iter;
            ps << endl << "<param name=\"" << p->get_name() << "\">";
            if (p->has_doc())
            {
                string str = p->get_doc();
                str.erase(remove(str.begin(), str.end(), '\n'), str.end());
                ps << str;
            }
            ps << "</param>";
        }

        docstring_comment(out,
                                   "",
                                   "/// ",
                                   "<summary>" + endl + tfunction->get_doc() + "</summary>" + ps.str(),
                                   "");
    }
}

void t_vala_generator::docstring_comment(ostream& out, const string& comment_start, const string& line_prefix, const string& contents, const string& comment_end)
{
    if (comment_start != "")
    {
        out << indent() << comment_start;
    }

    stringstream docs(contents, std::ios_base::in);

    while (!(docs.eof() || docs.fail()))
    {
        char line[1024];
        docs.getline(line, 1024);

        // Just prnt a newline when the line & prefix are empty.
        if (strlen(line) == 0 && line_prefix == "" && !docs.eof())
        {
            out << endl;
        }
        else if (strlen(line) > 0 || !docs.eof())
        { // skip the empty last line
            out << indent() << line_prefix << line << endl;
        }
    }
    if (comment_end != "")
    {
        out << indent() << comment_end;
    }
}

string t_vala_generator::get_enum_class_name(t_type* type)
{
    string package = "";
    t_program* program = type->get_program();
    if (program != NULL && program != program_)
    {
        package = program->get_namespace("vala") + ".";
    }
    return package + type->get_name();
}

THRIFT_REGISTER_GENERATOR(
    vala,
    "Vala",
    "    pascal:          Generate Pascal Case property names according to Microsoft naming convention.\n"
)
