//---------------------------------------------------------------------------//
//                        GUILoadThread.cpp -
//  Class describing the thread that performs the loading of a simulation
//                           -------------------
//  project              : SUMO - Simulation of Urban MObility
//  begin                : Sept 2002
//  copyright            : (C) 2002 by Daniel Krajzewicz
//  organisation         : IVF/DLR http://ivf.dlr.de
//  email                : Daniel.Krajzewicz@dlr.de
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//---------------------------------------------------------------------------//
namespace
{
    const char rcsid[] =
    "$Id$";
}
// $Log$
// Revision 1.13  2003/07/22 14:56:46  dkrajzew
// changes due to new detector handling
//
// Revision 1.12  2003/07/07 08:09:38  dkrajzew
// Some further error checking was added and the usage of the SystemFrame was refined
//
// Revision 1.11  2003/06/24 08:09:28  dkrajzew
// implemented SystemFrame and applied the changes to all applications
//
// Revision 1.10  2003/06/19 10:56:03  dkrajzew
// user information about simulation ending added; the gui may shutdown on end and be started with a simulation now;
//
// Revision 1.9  2003/06/18 11:04:53  dkrajzew
// new error processing adapted
//
// Revision 1.8  2003/06/06 11:12:37  dkrajzew
// deletion of singletons changed/added
//
// Revision 1.7  2003/06/06 10:33:47  dkrajzew
// changes due to moving the popup-menus into a subfolder
//
// Revision 1.6  2003/06/05 06:26:16  dkrajzew
// first tries to build under linux: warnings removed; Makefiles added
//
// Revision 1.5  2003/03/20 16:17:52  dkrajzew
// windows eol removed
//
// Revision 1.4  2003/03/12 16:55:18  dkrajzew
// centering of objects debugged
//
// Revision 1.3  2003/02/07 10:34:14  dkrajzew
// files updated
//
//


/* =========================================================================
 * included modules
 * ======================================================================= */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <sumo_version.h>
#include <qthread.h>
#include <iostream>
#include <guisim/GUINet.h>
#include <guinetload/GUINetBuilder.h>
#include <microsim/MSDetectorSubSys.h>
#include <utils/common/UtilExceptions.h>
#include <utils/xml/XMLBuildingExceptions.h>
#include <utils/options/OptionsCont.h>
#include <utils/options/Option.h>
#include <utils/options/OptionsIO.h>
#include <utils/options/OptionsSubSys.h>
#include <utils/common/MsgHandler.h>
#include <sumo_only/SUMOFrame.h>
#include <helpers/SingletonDictionary.h>
#include "QSimulationLoadedEvent.h"
#include "GUIApplicationWindow.h"
#include "GUILoadThread.h"
#include "QMessageEvent.h"
#include "QSimulationEndedEvent.h"


#include <ctime>
/* =========================================================================
 * used namespaces
 * ======================================================================= */
using namespace std;


/* =========================================================================
 * member method definitions
 * ======================================================================= */
GUILoadThread::GUILoadThread(GUIApplicationWindow *mw)
    : _parent(mw)
{
    MsgHandler::getErrorInstance()->addRetriever(this);
}


GUILoadThread::~GUILoadThread()
{
}


void
GUILoadThread::init(const string &file)
{
    _file = file;
}


void GUILoadThread::run()
{
    GUINet *net = 0;
    std::ostream *craw = 0;
    int simStartTime = 0;
    int simEndTime = 0;

    OptionsSubSys::close();
    if(!OptionsSubSys::guiInit(SUMOFrame::fillOptions, _file)) {
        // ok, the options could not be set
        QThread::postEvent( _parent,
            new QSimulationLoadedEvent(net, craw, simStartTime, simEndTime,
            string(_file)) );
        return;
    }
    // retrieve the options
    OptionsCont &oc = OptionsSubSys::getOptions();
    if(!SUMOFrame::checkOptions(oc)) {
        // the options are not valid
        QThread::postEvent( _parent,
            new QSimulationLoadedEvent(net, craw, simStartTime, simEndTime,
            string(_file)) );
        return;
    }
    // try to load
    try {
//        MSDetectorSubSys::deleteDictionariesAndContents();
        MsgHandler::getErrorInstance()->clear();
        MsgHandler::getWarningInstance()->clear();
        MsgHandler::getMessageInstance()->clear();
//        MSDetectorSubSys::createDictionaries();
//        OptionsIO::loadConfiguration(oc);
        GUINetBuilder builder(oc);
        net = builder.buildGUINet();
        if(net!=0) {
            SUMOFrame::postbuild(*net);
            simStartTime = oc.getInt("b");
            simEndTime = oc.getInt("e");
            craw = SUMOFrame::buildRawOutputStream(oc);
        }
    } catch (UtilException &e) {
        delete net;
        delete craw;
        MSNet::clearAll();
        net = 0;
        craw = 0;
    } catch (XMLBuildingException &e) {
        delete net;
        delete craw;
        MSNet::clearAll();
        net = 0;
        craw = 0;
    }
    QThread::postEvent( _parent,
        new QSimulationLoadedEvent(net, craw, simStartTime, simEndTime,
        string(_file)) );
}

void
GUILoadThread::inform(const std::string &msg)
{
    QThread::postEvent( _parent,
        new QMessageEvent(MsgHandler::MT_ERROR, msg));
}


/**************** DO NOT DEFINE ANYTHING AFTER THE INCLUDE *****************/
//#ifdef DISABLE_INLINE
//#include "GUILoadThread.icc"
//#endif

// Local Variables:
// mode:C++
// End:


