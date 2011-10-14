/*
 *  Yarp Modules Manager
 *  Copyright: 2011 (C) Robotics, Brain and Cognitive Sciences - Italian Institute of Technology (IIT)
 *  Authors: Ali Paikan <ali.paikan@iit.it>
 * 
 *  Copy Policy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 *
 */

#ifndef _APPLICATION_WINDOW_H_
#define _APPLICATION_WINDOW_H_

#include <gtkmm.h>
#include "safe_manager.h"
#include <vector>

#include <yarp/os/Network.h>
#include <yarp/os/Property.h>
#include <yarp/os/ConstString.h>
#include <yarp/os/Time.h>

class MainWindow;

//Tree model columns:
class ModuleModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	ModuleModelColumns() { 
		add(m_col_refPix);
		add(m_col_name); 
		add(m_col_id);
		add(m_col_status); 
		add(m_col_host); 
		add(m_col_param); 
		add(m_col_stdio); 
		add(m_col_wdir); 
		add(m_col_env); 
		add(m_col_color);
		}
	Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_col_refPix;
	Gtk::TreeModelColumn<int> m_col_id;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
	Gtk::TreeModelColumn<Glib::ustring> m_col_status;
	Gtk::TreeModelColumn<Glib::ustring> m_col_host;
	Gtk::TreeModelColumn<Glib::ustring> m_col_param;
	Gtk::TreeModelColumn<Glib::ustring> m_col_stdio;
	Gtk::TreeModelColumn<Glib::ustring> m_col_wdir;
	Gtk::TreeModelColumn<Glib::ustring> m_col_env;
	Gtk::TreeModelColumn<Gdk::Color> m_col_color;

};



class ConnectionModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	ConnectionModelColumns() { 
		add(m_col_refPix); 
		add(m_col_type);
		add(m_col_id); 
		add(m_col_status);
		add(m_col_from); 
		add(m_col_to); 
		add(m_col_carrier); 
		add(m_col_color);
		}
	Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_col_refPix;
	Gtk::TreeModelColumn<int> m_col_id;
	Gtk::TreeModelColumn<Glib::ustring> m_col_type;
	Gtk::TreeModelColumn<Glib::ustring> m_col_status;
	Gtk::TreeModelColumn<Glib::ustring> m_col_from;
	Gtk::TreeModelColumn<Glib::ustring> m_col_to;
	Gtk::TreeModelColumn<Glib::ustring> m_col_carrier;
	Gtk::TreeModelColumn<Gdk::Color> m_col_color;

};


class ResourceModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

	ResourceModelColumns() { 
		add(m_col_refPix); 
		add(m_col_res); 
		add(m_col_id); 
		add(m_col_status);
		add(m_col_color);
	}

	Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_col_refPix;
	Gtk::TreeModelColumn<int> m_col_id;
	Gtk::TreeModelColumn<Glib::ustring> m_col_res;
	Gtk::TreeModelColumn<Glib::ustring> m_col_status;
	Gtk::TreeModelColumn<Gdk::Color> m_col_color;
};




class ApplicationWindow: public Gtk::ScrolledWindow, ApplicationEvent
{
public:
	ApplicationWindow(const char* szAppName, Manager* lazy, 
		yarp::os::Property* config, MainWindow* parent);
	virtual ~ApplicationWindow();
	const char* getApplicationName(void) { return m_strAppName.c_str(); }

	bool onClose(void);
	bool onRun(void);
	bool onStop(void);
	bool onKill(void);
	bool onConnect(void);
	bool onDisconnect(void);
	bool onRefresh(void);
	bool onSelectAll(void);
	
	void onTabCloseRequest();
	void releaseApplication(void);


	Glib::RefPtr<Gtk::TreeStore> m_refTreeModModel;
	Glib::RefPtr<Gtk::TreeStore> m_refTreeConModel;
	Glib::RefPtr<Gtk::TreeStore> m_refTreeResModel;
	ModuleModelColumns m_modColumns;
	ConnectionModelColumns m_conColumns;
	ResourceModelColumns m_resColumns;

	void onModStart(int which);
	void onModStop(int which);
	void onConConnect(int which);
	void onConDisconnect(int which);
	void onResAvailable(int which);
	void onResUnAvailable(int which);
	void onError(void);


protected:
		
	void selectedModuleCallback(const Gtk::TreeModel::iterator& iter);
	void selectedConnectionCallback(const Gtk::TreeModel::iterator& iter);
	void selectedResourceCallback(const Gtk::TreeModel::iterator& iter);
	void onModuleTreeButtonPressed(GdkEventButton* event);
	void onConnectionTreeButtonPressed(GdkEventButton* event);
	void onResourceTreeButtonPressed(GdkEventButton* event);
	
	void onPMenuRun() { onRun(); }
	void onPMenuStop() { onStop(); }
	void onPMenuKill() { onKill(); }
	void onPMenuConnect() { onConnect(); }
	void onPMenuDisconnect() { onDisconnect(); }
	void onPMenuRefresh() { onRefresh(); }
	void onPMenuSelectAll() { onSelectAll(); }

	Gtk::VPaned m_VPaned;
	Gtk::HPaned m_HPaned;
	Gtk::TreeView m_TreeModView;
	Gtk::TreeView m_TreeConView;
	Gtk::TreeView m_TreeResView;
	Gtk::TreeModel::Row m_modRow;
	Gtk::TreeModel::Row m_conRow;
	Gtk::TreeModel::Row m_resRow;
	Glib::RefPtr<Gtk::TreeSelection> m_refTreeModSelection;
	Glib::RefPtr<Gtk::TreeSelection> m_refTreeConSelection;
	Glib::RefPtr<Gtk::TreeSelection> m_refTreeResSelection;
	Glib::RefPtr<Gtk::UIManager> m_refUIManager;
	Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;

private:
	bool m_bShouldRun;
	SafeManager manager;
	MainWindow* m_pParent;
	string m_strAppName;
	yarp::os::Property* m_pConfig;
	std::vector<int> m_ModuleIDs;
	std::vector<int> m_ConnectionIDs;
	std::vector<int> m_ResourceIDs;

	Glib::RefPtr<Gdk::Pixbuf> m_refPixSuspended;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixRunning;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixWaiting;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixConnected;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixDisconnected;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixAvailable;
	Glib::RefPtr<Gdk::Pixbuf> m_refPixUnAvailable;

	void createWidgets(void);
	void setupSignals(void);
		
	bool getModRowByID(int id, Gtk::TreeModel::Row* row );
	bool getConRowByID(int id, Gtk::TreeModel::Row* row );
	bool getResRowByID(int id, Gtk::TreeModel::Row* row );
	void setCellsEditable(void);

	void prepareManagerFrom(Manager* lazy, const char* szName);
	void reportErrors(void);
	
	void doClose(void);
	void doRun(void);
	void doStop(void);
	void doKill(void);
	void doConnect(void);
	void doDisconnect(void);
	void doRefresh(void);

};

#endif //_APPLICATION_WINDOW_H_
