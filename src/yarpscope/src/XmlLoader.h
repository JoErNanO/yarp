/*
 * Copyright (C) 2012  iCub Facility, Istituto Italiano di Tecnologia
 * Author: Daniele E. Domenichelli <daniele.domenichelli@iit.it>
 *
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */


#ifndef YARPSCOPE_XMLLOADER_H
#define YARPSCOPE_XMLLOADER_H

#include <string>

namespace Glib {
class ustring;
}

namespace YarpScope
{

class XmlLoader
{
public:
    explicit XmlLoader(const Glib::ustring &filename);
    virtual ~XmlLoader();

private:
    /**
     * Converts the given string into an integer.
     * Provides a safe alternative to atoi and the like which upon error fail with <i>undefined behaviour</i>.
     *
     * \param i_str The string to convert
     * \param o_val The converted integer
     * \return True upon success
     */
    bool string2int(const std::string &i_str, int &o_val);

}; // class XmlLoader

} // namespace YarpScope


#endif // YARPSCOPE_XMLLOADER_H
