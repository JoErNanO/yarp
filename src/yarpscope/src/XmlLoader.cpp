/*
 * Copyright (C) 2012  iCub Facility, Istituto Italiano di Tecnologia
 * Author: Daniele E. Domenichelli <daniele.domenichelli@iit.it>
 *
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */


#include "XmlLoader.h"
#include "PortReader.h"
#include "PlotManager.h"
#include "Debug.h"

#include <yarp/os/ContactStyle.h>

#include <tinyxml.h>

#include <glibmm/ustring.h>
#include <stdexcept>

#include <sstream>

namespace {
    static const int default_portscope_rows = 1;
    static const int default_portscope_columns = 1;
    static const Glib::ustring default_portscope_carrier = "mcast";
    static bool default_portscope_persistent = true;
    static const int default_plot_gridx = -1;
    static const int default_plot_gridy = -1;
    static const int default_plot_hspan = 1;
    static const int default_plot_vspan = 1;
    static const float default_plot_minval = -100.;
    static const float default_plot_maxval = 100.;
    static const int default_plot_size = 201;
    static const bool default_plot_autorescale = false;
    static const bool default_plot_realtime = false;
    static const bool default_plot_triggermode = false;
    static const int default_graph_size = 1;
    static const Glib::ustring default_graph_type = "lines";
}

// FIXME check if rows and columns are used, or if the table is resized
//       automatically
// FIXME set default plot bgcolor
// FIXME set default graph color


YarpScope::XmlLoader::XmlLoader(const Glib::ustring& filename)
{
    using std::string;

    YarpScope::PortReader &portReader = YarpScope::PortReader::instance();
    YarpScope::PlotManager &plotManager = YarpScope::PlotManager::instance();


    TiXmlDocument doc(filename.c_str());
    if (!doc.LoadFile()) {
        fatal() << "Syntax error while loading" << filename << "at line" << doc.ErrorRow() << ":" << doc.ErrorDesc();
    }

    TiXmlElement *rootElem = doc.RootElement();
    if (!rootElem) {
        fatal() << "Syntax error while loading" << filename << ". No root element.";
    }

    if (Glib::ustring(rootElem->Value()).compare("portscope") != 0) {
        fatal() << "Syntax error while loading" << filename << ". Root element should be \"portscope\", found" << rootElem->Value();
    }

    int portscope_rows, portscope_columns;
    Glib::ustring portscope_carrier;
    bool portscope_persistent;

    if (rootElem->QueryIntAttribute("rows", &portscope_rows) != TIXML_SUCCESS || portscope_rows < 1) {
        portscope_rows = default_portscope_rows;
    }

    if (rootElem->QueryIntAttribute("columns", &portscope_columns) != TIXML_SUCCESS || portscope_columns < 1) {
        portscope_columns = default_portscope_columns;
    }

    if (portscope_rows != 1 || portscope_columns != 1) {
        plotManager.setupTable(portscope_rows, portscope_columns);
    }


    if (const char *t = rootElem->Attribute("carrier")) {
        portscope_carrier = t;
    } else {
        portscope_carrier = default_portscope_carrier;
    }

    if (rootElem->QueryBoolAttribute("persistent", &portscope_persistent) != TIXML_SUCCESS) {
        portscope_persistent = default_portscope_persistent;
    }

    for (TiXmlElement *plotElem = rootElem->FirstChildElement(); plotElem != 0; plotElem = plotElem->NextSiblingElement()) {
        if (Glib::ustring(plotElem->Value()).compare("plot") != 0) {
            fatal() << "Syntax error while loading" << filename << ". Expected \"plot\", found" << plotElem->Value();
        }

        Glib::ustring plot_title, plot_bgcolor;
        int plot_gridx, plot_gridy, plot_hspan, plot_vspan, plot_size;
        float plot_minval, plot_maxval;
        bool plot_autorescale, plot_realtime, plot_triggermode;
        if (const char *t = plotElem->Attribute("title")) {
            plot_title = t;
        }
        if (plotElem->QueryIntAttribute("gridx", &plot_gridx) != TIXML_SUCCESS || plot_gridx < 0) {
            plot_gridx = default_plot_gridx;
        }
        if (plotElem->QueryIntAttribute("gridy", &plot_gridy) != TIXML_SUCCESS || plot_gridy < 0) {
            plot_gridy = default_plot_gridy;
        }
        if (plotElem->QueryIntAttribute("hspan", &plot_hspan) != TIXML_SUCCESS || plot_hspan <= 0) {
            plot_hspan = default_plot_hspan;
        }
        if (plotElem->QueryIntAttribute("vspan", &plot_vspan) != TIXML_SUCCESS || plot_vspan <= 0) {
            plot_vspan = default_plot_vspan;
        }
        if (plotElem->QueryFloatAttribute("minval", &plot_minval) != TIXML_SUCCESS) {
            plot_minval = default_plot_minval;
        }
        if (plotElem->QueryFloatAttribute("maxval", &plot_maxval) != TIXML_SUCCESS) {
            plot_maxval = default_plot_maxval;
        }
        if (plotElem->QueryIntAttribute("size", &plot_size) != TIXML_SUCCESS || plot_size <= 0) {
            plot_size = default_plot_size;
        }
        if (const char *t = plotElem->Attribute("bgcolor")) {
            plot_bgcolor = t;
        }
        if (plotElem->QueryBoolAttribute("autorescale", &plot_autorescale) != TIXML_SUCCESS) {
            plot_autorescale = default_plot_autorescale;
        }
        // TODO enable realtime mode
        if (plotElem->QueryBoolAttribute("realtime", &plot_realtime) != TIXML_SUCCESS) {
            plot_realtime = default_plot_realtime;
        }
        // TODO enable trigger mode
        if (plotElem->QueryBoolAttribute("triggermode", &plot_triggermode) != TIXML_SUCCESS) {
            plot_triggermode = default_plot_triggermode;
        }


        int plotIndex = plotManager.addPlot(plot_title, plot_gridx, plot_gridy, plot_hspan, plot_vspan, plot_minval, plot_maxval, plot_size, plot_bgcolor, plot_autorescale);

        for (TiXmlElement* graphElem = plotElem->FirstChildElement(); graphElem != 0; graphElem = graphElem->NextSiblingElement()) {
            if (Glib::ustring(graphElem->Value()).compare("graph") != 0) {
                fatal() << "Syntax error while loading" << filename << ". Expected \"graph\", found" << graphElem->Value();
            }

            Glib::ustring graph_remote, graph_title, graph_color, graph_type;
            if (const char *t = graphElem->Attribute("remote")) {
                graph_remote = t;
            } else {
                fatal() << "Syntax error while loading" << filename << ". \"remote\" attribute not found in element \"graph\"";
            }

            // Graph data indexes to plot
            string graph_indexes;
            if (graphElem->QueryStringAttribute("index", &graph_indexes) != TIXML_SUCCESS) {
                fatal() << "Syntax error while loading" << filename << ". \"index\" attribute not found in element \"graph\"";
            }

            if (const char *t = graphElem->Attribute("title")) {
                graph_title = t;
            }
            if (const char *t = graphElem->Attribute("color")) {
                graph_color = t;
            }
            if (const char *t = graphElem->Attribute("type")) {
                graph_type = t;
            } else {
                graph_type = default_graph_type;
            }
            int graph_size;
            if (graphElem->QueryIntAttribute("size", &graph_size) != TIXML_SUCCESS || graph_size <= 0) {
                graph_size = default_graph_size;
            }

            // Split indexes string
            string delimiters = "-";
            int start = 0;
            int end = 0;
            if (graph_indexes.size() > 0) {
                // Find delimiter "-"
                size_t curPos = graph_indexes.find_first_of(delimiters);
                std::cout << "Start position " << curPos << "\n";
                if (curPos == string::npos) {
                    // Only one index specified
                    if (string2int(graph_indexes, start)) {
                        end = start;
                    } else {
                        std::ostringstream err;
                        err << "Syntax error while loading" << filename << ". Invalid \"index\" specified " << graph_indexes;
                        fatal() << err.str().c_str();
                    }
                } else {
                    // Index range specified
                    string tmpIndexS = graph_indexes.substr(0, curPos);
                    string tmpIndexE = graph_indexes.substr(curPos+1, string::npos);
                    // Convert start index
                    if (!string2int(tmpIndexS, start)) {
                        std::ostringstream err;
                        err << "Syntax error while loading" << filename << ". Invalid starting index \"" << tmpIndexS << "\" specified in the \"index\" attribute.";
                        fatal() << err.str().c_str();
                    }
                    // Convert end index
                    if (!string2int(tmpIndexE, end)) {
                        std::ostringstream err;
                        err << "Syntax error while loading" << filename << ". Invalid end index \"" << tmpIndexE << "\" specified in the \"index\" attribute.";
                        fatal() << err.str().c_str();
                    }

                    // Check for correct index range
                    if (start <= end) {
                        std::cout << "Plotting data with start index " << start << " and end index " << end << "\n";
                    } else {
                        std::ostringstream err;
                        err << "Syntax error while loading" << filename << ". Start index \"" << tmpIndexS << "\" specified in the \"index\" attribute should be smaller than the end index \"" << tmpIndexE << "\".";
                        fatal() << err.str().c_str();
                    }
                }
            }

            // Add plots
            for (int i = start; i <= end; ++i) {
                portReader.acquireData(graph_remote, i, "", portscope_carrier, portscope_persistent);
                plotManager.addGraph(plotIndex, graph_remote, i, graph_title, graph_color, graph_type, graph_size);
            }
        }
    }

    portReader.toggleAcquire(true);
}

YarpScope::XmlLoader::~XmlLoader()
{
}


/* *********************************************************************************************************************** */
/* ******* Convert the given string into an integer.                        ********************************************** */
bool YarpScope::XmlLoader::string2int(const std::string &i_str, int &o_val) {
    bool ok = false;
    std::istringstream iss(i_str);

    iss >> std::ws >> o_val >> std::ws;

    if(iss.eof()) {
        ok = true;
    }

    return ok;
}
/* *********************************************************************************************************************** */
