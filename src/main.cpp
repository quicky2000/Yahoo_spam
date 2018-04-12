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
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

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

//------------------------------------------------------------------------------
void dump_string_in_file(const std::string & p_string,
                         const std::string & p_file_name
                        )
{
    std::ofstream l_ofstream;
    l_ofstream.open(p_file_name);
    if(!l_ofstream.is_open())
    {
        throw quicky_exception::quicky_runtime_exception("Unable to write file \""+ p_file_name + "\"", __LINE__, __FILE__);
    }
    l_ofstream << p_string << std::endl;
    l_ofstream.close();

}

//------------------------------------------------------------------------------
void remove_extra_line_return(std::string & p_text,
                             const std::string & p_line_return
                             )
{
    size_t l_pos = 0;
    while(std::string::npos != (l_pos = p_text.find(p_line_return, l_pos)))
    {
        if(l_pos > 0 && p_text[l_pos -1] != '>')
        {
            p_text.erase(l_pos,p_line_return.size());
        }
        else
        {
            ++l_pos;
        }
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
            if('<' != l_line[0] || '>' != l_line[l_line.size() -1])
            {
                throw quicky_exception::quicky_logic_exception("Malformed Beacon : \"" + l_line + "\"", __LINE__, __FILE__);
            }
            p_beacons.push_back(l_line);
        }
    }
}

//------------------------------------------------------------------------------
void extract_attributes(const std::string & p_string,
                        std::map<std::string,std::string> & p_attributes)
{
    // Search for first blank character separating beacon from attribute name
    size_t l_pos = p_string.find_first_of(" \t");
    if(std::string::npos == l_pos)
    {
        throw quicky_exception::quicky_logic_exception("Missing space after beacon name : \"" + p_string + "\"", __LINE__, __FILE__);
    }

    size_t l_pos_start = l_pos;
    while(std::string::npos != (l_pos = p_string.find('=', l_pos_start)))
    {
        std::string l_attribute_name = p_string.substr(l_pos_start, l_pos - l_pos_start);
        trim(l_attribute_name);

        l_pos_start = p_string.find('"', l_pos);
        if(std::string::npos == l_pos_start)
        {
            throw quicky_exception::quicky_logic_exception("Missing '\"' character starting value of attribute \"" + l_attribute_name + " in \"" + p_string + "\"", __LINE__, __FILE__);
        }
        ++l_pos_start;
        l_pos = p_string.find('"', l_pos_start);
        if(std::string::npos == l_pos)
        {
            throw quicky_exception::quicky_logic_exception("Missing '\"' character closing value of attribute \"" + l_attribute_name + " in \"" + p_string + "\"", __LINE__, __FILE__);
        }
        std::string l_value = p_string.substr(l_pos_start, l_pos - l_pos_start);
        p_attributes.insert(std::map<std::string,std::string>::value_type(l_attribute_name, l_value));
        l_pos_start = l_pos + 1;
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
        std::string l_url_content;
        quicky_url_reader::url_reader & l_instance = quicky_url_reader::url_reader::instance();
        std::string l_cookie_file_name = "cookie.txt";
        l_instance.set_cookie_file(l_cookie_file_name);

        // Define headers like seen in Firefox
        l_instance.add_http_headers("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
//        l_instance.add_http_headers("Accept-Encoding","gzip, deflate, br");
        l_instance.add_http_headers("Accept-Language","en-US,en;q=0.5");
        l_instance.add_http_headers("Connection","keep-alive");
        l_instance.add_http_headers("DNT","1");
        l_instance.add_http_headers("Host","login.yahoo.com");
        l_instance.add_http_headers("Upgrade-Insecure-Requests","1");
        l_instance.add_http_headers("User-Agent","Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:52.0) Gecko/20100101 Firefox/52.0");

        l_instance.dump_url("https://login.yahoo.com",
                            l_url_content
                           );

        dump_string_in_file(l_url_content, "URL_content.html");

        std::string l_modified_content = l_url_content;
        remove_beacon_text("script", l_modified_content);
        remove_beacon_text("style", l_modified_content);

        std::string l_file_name("URL_modified_content.html");
        dump_string_in_file(l_modified_content, l_file_name);

        remove_extra_line_return(l_modified_content,"\r\n");
        remove_extra_line_return(l_modified_content,"\n");

        l_file_name = "URL_modified_content2.html";
        dump_string_in_file(l_modified_content, l_file_name);

        std::ifstream l_ifstream;
        l_ifstream.open(l_file_name.c_str());
        if(!l_ifstream.is_open())
        {
            throw quicky_exception::quicky_runtime_exception("Unable to read file \""+ l_file_name + "\"", __LINE__, __FILE__);
        }

        // Extract inputs
        std::vector<std::string> l_inputs;
        extract_input_beacons(l_ifstream, l_inputs);
        l_ifstream.close();

        std::map<std::string,std::string> l_interesting_inputs = {
                {"acrumb",""},
                {"sessionIndex",""}
        };

        // Extrac attibrutes for each input
        for(auto l_iter: l_inputs)
        {
            std::map<std::string,std::string> l_attributes;
            extract_attributes(l_iter, l_attributes);

            // Search for name attribute
            auto l_attribute_iter = l_attributes.find("name");
            if(l_attributes.end() == l_attribute_iter)
            {
                throw quicky_exception::quicky_logic_exception("name attribute is missing for input \"" + l_iter + "\"", __LINE__, __FILE__);
            }
            std::string l_name = l_attribute_iter->second;

            //Search if this an interesting input
            auto l_input_iter = l_interesting_inputs.find(l_name);
            if(l_interesting_inputs.end() != l_input_iter)
            {
                // Search value attribute
                l_attribute_iter = l_attributes.find("value");
                if(l_attributes.end() == l_attribute_iter)
                {
                    throw quicky_exception::quicky_logic_exception("value attribute is missing for input \"" + l_name +"\" : \"" + l_iter + "\"", __LINE__, __FILE__);
                }
                l_interesting_inputs[l_name] = l_attribute_iter->second;
            }
        }

        // Display value of interesting inputs
        for (auto l_iter: l_interesting_inputs)
        {
            std::cout << "\t" << l_iter.first << "=\"" << l_iter.second << "\"" << std::endl;
            if("" == l_iter.second)
            {
                throw quicky_exception::quicky_logic_exception("Input \"" + l_iter.first +"\" is missing", __LINE__, __FILE__);
            }
        }

        // Add harcorded inputs
        std::vector<std::pair<std::string,std::string> > l_param_list;
        l_param_list.push_back(std::pair<std::string,std::string>("authMecanism","primary"));
        l_param_list.push_back(std::pair<std::string,std::string>("display","login"));
        l_param_list.push_back(std::pair<std::string,std::string>("yid","julien_thevenon"));
        l_param_list.push_back(std::pair<std::string,std::string>("done","https://www.yahoo.com/"));
        l_param_list.push_back(std::pair<std::string,std::string>("sessionIndex",l_interesting_inputs["sessionIndex"]));
        l_param_list.push_back(std::pair<std::string,std::string>("acrumb",l_interesting_inputs["acrumb"]));

        std::string l_params;
        for(auto l_iter: l_param_list)
        {
            l_instance.add_parameter(l_params, l_iter.first, l_iter.second);
        }
        //sleep(10);
        std::string l_step2_url = "https://login.yahoo.com/account/challenge/password?" + l_params;
        std::cout << "URL = \""  << l_step2_url << "\"" << std::endl;

        l_instance.add_http_headers("Referer","https://login.yahoo.com/");

        // Parse cookie file
        std::string l_cookie_content;
        std::ifstream l_cookie_file;
        l_cookie_file.open(l_cookie_file_name);
        if(!l_cookie_file.is_open())
        {
            throw quicky_exception::quicky_logic_exception("Unable to read " + l_cookie_file_name, __LINE__, __FILE__);
        }
        while(!l_cookie_file.eof())
        {
            std::string l_line;
            std::getline(l_cookie_file,l_line);
            std::string l_truncated;
            if("" != l_line && std::string::npos != l_line.find("yahoo"))
            {
                size_t l_pos = 0;
                for(unsigned int l_index = 0; l_index < 5; ++l_index)
                {
                    l_pos = l_line.find('\t', l_pos);
                    if(std::string::npos == l_pos)
                    {
                        throw quicky_exception::quicky_logic_exception("Not enough TAB in cookie file line \"" + l_line + "\"", __LINE__, __FILE__);
                    }
                    ++l_pos;
                }
                l_truncated = l_line.substr(l_pos);
                std::cout << "\"" << l_truncated << "\"" << std::endl;
                l_pos = l_truncated.find('\t');
                if(std::string::npos == l_pos)
                {
                    throw quicky_exception::quicky_logic_exception("Missing TAB in cookie file line \"" + l_line + "\"", __LINE__, __FILE__);
                }
                std::string l_name = l_truncated.substr(0,l_pos);
                std::string l_value = l_truncated.substr(l_pos + 1);
                if("" != l_cookie_content)
                {
                    l_cookie_content += "; ";
                }
                l_cookie_content += l_name + "=" + l_value;
            }
        }
        l_cookie_file.close();

        std::cout << "Cookie : " << l_cookie_content << std::endl;
        //l_instance.add_http_headers("Cookie",l_cookie_content);

        l_instance.dump_url(l_step2_url,
                            l_url_content
                           );
        dump_string_in_file(l_url_content, "URL_login.html");
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
