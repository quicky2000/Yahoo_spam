/*    This file is part of Yahoo_spam
      Copyright (C) 2018  Julien Thevenon ( julien_thevenon at yahoo.fr )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/
#include "url_reader.h"
#include "quicky_exception.h"
#include "xmlParser.h"
#include <iostream>
#include <fstream>
#include <vector>

/**
 * Convert a swtring to string
 * @param p_wstring wstring to convert
 * @return string
 */
std::string to_string(const std::wstring & p_wstring)
{
    std::string l_string(p_wstring.size(), ' ');
    wcstombs(&l_string[0], p_wstring.c_str(), p_wstring.size());
    return l_string;
}

/**
 * Convert a string to wstring
 * @param p_string string to convert
 * @return wstring
 */
std::wstring to_wstring(const std::string & p_string)
{
    std::wstring l_wstring(p_string.size(), L' ');
    mbstowcs(&l_wstring[0],p_string.c_str(),p_string.size());
    return l_wstring;
}

void printNode(XMLNode p_node, unsigned int p_level=0)
{
    std::string l_prefix(p_level, ' ');
    std::cout << l_prefix << "Name : \"";
    std::wcout << (p_node.getName() ? p_node.getName() : L"") << "\"" << std::endl;
    std::cout << l_prefix << "Child number : " << p_node.nChildNode() << std::endl;
    std::cout << l_prefix << "Attribute number : " << p_node.nAttribute() << std::endl;
    std::cout << l_prefix << "Text number : " << p_node.nText() << std::endl;
    std::cout << l_prefix << "Clear number : " << p_node.nClear() << std::endl;
    for (unsigned int l_index = 0;
         l_index < p_node.nAttribute();
         ++l_index
            )
    {
        std::cout << l_prefix << "Attribute[" << l_index << "] = (\"";
        std::wcout << p_node.getAttributeName(l_index);
        std::cout << "\",\"";
        std::wcout << p_node.getAttributeValue(l_index);
        std::cout << "\")" << std::endl;
    }
    for (unsigned int l_index = 0;
         l_index < p_node.nText();
         ++l_index
            )
    {
        std::cout << l_prefix << "Text[" << l_index << "] = \"";
        std::wcout << p_node.getText(l_index);
        std::cout << "\"" << std::endl;
    }
    for (unsigned int l_index = 0;
         l_index < p_node.nChildNode();
         ++l_index
            )
    {
        std::cout << l_prefix << "Child[" << l_index << "] :" << std::endl;
        printNode(p_node.getChildNode(l_index), p_level + 1);
    }

}

//------------------------------------------------------------------------------
void remove_beacon_text(const std::string & p_beacon, std::string & p_text)
{
    size_t l_pos = 0;

    while((l_pos = p_text.find("<" + p_beacon, l_pos)) != std::string::npos)
    {
        if (std::string::npos != l_pos)
        {
            l_pos = p_text.find(">",
                                l_pos
                               );
            if (std::string::npos != l_pos)
            {
                size_t l_pos_end = p_text.find("</" + p_beacon + ">",
                                               l_pos + 1
                                              );
                if (std::string::npos != l_pos_end)
                {
                    ++l_pos;
                    p_text = p_text.replace(l_pos,
                                            l_pos_end - l_pos,
                                            ""
                                           );
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void trim(std::string & p_string)
{
    if("" == p_string) return;
    size_t l_pos = p_string.find_first_not_of(" \t");
    if(std::string::npos != l_pos)
    {
        p_string.erase(0,l_pos);
    }
    l_pos = p_string.size();
    bool l_erase = false;
    while(l_pos && (' ' == p_string[l_pos - 1] || '\t' == p_string[l_pos -1]))
    {
        --l_pos;
        l_erase = true;
    }
    if(l_erase)
    {
        p_string.erase(l_pos);
    }
}

//------------------------------------------------------------------------------
void extract_input_beacons(std::istream & p_stream,
                           std::vector<std::string> & p_beacons)
{
    std::string l_line;
    while(!p_stream.eof())
    {
        std::getline(p_stream,l_line);
        size_t l_pos = l_line.find("<input");
        if(std::string::npos != l_pos)
        {
            trim(l_line);
            p_beacons.push_back(l_line);
        }
    }
}

//------------------------------------------------------------------------------
int
main(int argc,
     char **argv
    )
{
    try
    {
        const std::string l_file_name("URL_content.html");
        quicky_url_reader::url_reader & l_instance = quicky_url_reader::url_reader::instance();
        l_instance.dump_url("https://login.yahoo.com",
                            l_file_name
                           );
        std::ifstream l_ifstream;
        l_ifstream.open(l_file_name.c_str());
        if(!l_ifstream.is_open())
        {
            throw quicky_exception::quicky_runtime_exception("Unable to read file \""+ l_file_name + "\"", __LINE__, __FILE__);
        }
        std::vector<std::string> l_inputs;
        extract_input_beacons(l_ifstream, l_inputs);
        l_ifstream.close();

        for(auto l_iter: l_inputs)
        {
            std::cout << l_iter << std::endl;
        }
        std::cout << "Done" << std::endl;
    }
    catch (quicky_exception::quicky_runtime_exception & e)
    {
        std::cout << "ERROR : " << e.what() << " at " << e.get_file() << ":" << e.get_line() << std::endl;
        return (-1);
    }
    catch (quicky_exception::quicky_logic_exception & e)
    {
        std::cout << "ERROR : " << e.what() << " at " << e.get_file() << ":" << e.get_line() << std::endl;
        return (-1);
    }
    return 0;

}
//EOF
