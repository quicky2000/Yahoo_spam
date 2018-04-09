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
int
main(int argc,
     char **argv
    )
{
    try
    {
        quicky_url_reader::url_reader & l_instance = quicky_url_reader::url_reader::instance();
        std::string l_page_content;
        l_instance.dump_url("https://login.yahoo.com",
                            l_page_content
                           );
        std::cout << "Page content :" << std::endl << l_page_content;
        std::string l_modified_page_content = l_page_content;

        remove_beacon_text("script", l_modified_page_content);
        remove_beacon_text("style", l_modified_page_content);

        std::ofstream l_ofstream;
        l_ofstream.open("URL_content.txt");
        l_ofstream << l_page_content << std::endl;
        l_ofstream.close();
        l_ofstream.open("URL_modified_content.txt");
        l_ofstream << l_modified_page_content << std::endl;
        l_ofstream.close();

        //l_page_content = "<!DOCTYPE html><html class=\"no-js\"></html>";
        XMLResults l_result;
        std::wstring l_wstring(l_page_content.size(), L' ');
        mbstowcs(&l_wstring[0],l_page_content.c_str(),l_page_content.size());
        XMLNode l_node = XMLNode::parseString(l_wstring.c_str(),
                                              NULL, //L"html",
                                              &l_result
                                             );
        if (eXMLErrorNone == l_result.error)
        {
            std::cout << "Parsing OK" << std::endl;
        } else
        {
            std::wcout << XMLNode::getError(l_result.error) << std::endl;
        }
        printNode(l_node);
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
