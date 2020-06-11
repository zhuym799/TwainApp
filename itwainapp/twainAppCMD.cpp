﻿/***************************************************************************
* Copyright ?2007 TWAIN Working Group:
*   Adobe Systems Incorporated, AnyDoc Software Inc., Eastman Kodak Company,
*   Fujitsu Computer Products of America, JFL Peripheral Solutions Inc.,
*   Ricoh Corporation, and Xerox Corporation.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the TWAIN Working Group nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY TWAIN Working Group ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL TWAIN Working Group BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************/

/**
 * @file TwainAppCMD.cpp
 * TWAIN Application.
 * A TWAIN Application communicates with the DSM to acquire images.
 * The goal of the application is to acquire data from a Source.
 * However, applications cannot contact the Source directly.  All requests for
 * data, capability information, error information, etc. must be handled
 * Through the Source Manager.
 * @author JFL Peripheral Solutions Inc.
 * @date April 2007
 */

#ifdef _WINDOWS
#include "stdafx.h"
#endif
//#include "main.h"

#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <stdio.h>

#include "TwainAppCMD.h"
#include "common/CTiffWriter.h"
#include "common/TwainString.h"

using namespace std;


//////////////////////////////////////////////////////////////////////////////
TwainAppCMD::TwainAppCMD(HWND parent /*=NULL*/) :TwainApp(parent)
{

#ifdef TWNDS_OS_LINUX
    // Initialize event semaphore
    sem_init(&m_TwainEvent,   // handle to the event semaphore
             0,     // not shared
             0);    // initially set to non signaled state
#endif
}

//////////////////////////////////////////////////////////////////////////////
TwainAppCMD::~TwainAppCMD()
{
#ifdef TWNDS_OS_LINUX
    // now destroy the event semaphore
    sem_destroy(&m_TwainEvent);   // Event semaphore handle
#endif
}

////////////////////////////////////////////////////////////////////////////////
//void TwainAppCMD::fillIdentity(TW_IDENTITY& _identity)
//{
//	TwainApp::fillIdentity(_identity);
//	SSTRCPY(_identity.Manufacturer, sizeof(_identity.Manufacturer), "TWAIN Working Group");
//	SSTRCPY(_identity.ProductFamily, sizeof(_identity.ProductFamily), "Sample TWAIN App");
//	SSTRCPY(_identity.ProductName, sizeof(_identity.ProductName), "Command line TWAIN app");

//	return;
//}

////////////////////////////////////////////////////////////////////////////////
//void TwainAppCMD::printIdentStruct(const TW_IDENTITY& _ident)
//{
//	PrintCMDMessage("\n Id: %u\n", _ident.Id);
//	PrintCMDMessage("Version: %u.%u\n", _ident.Version.MajorNum, _ident.Version.MinorNum);
//	PrintCMDMessage("SupportedGroups: %u\n", _ident.SupportedGroups);
//	PrintCMDMessage("Manufacturer: %s\n", _ident.Manufacturer);
//	PrintCMDMessage("ProductFamily: %s\n", _ident.ProductFamily);
//	PrintCMDMessage("ProductName: %s\n", _ident.ProductName);

//	return;
//}

//////////////////////////////////////////////////////////////////////////////
//void TwainAppCMD::printIdentityStruct(const TW_UINT32 _identityID)
//{
//    for (unsigned int x = 0; x < (m_DataSources).size(); ++x)
//	{
//        if (_identityID == (m_DataSources)[x].Id)
//		{
//            printIdentStruct((m_DataSources)[x]);
//			break;
//		}
//	}

//	return;
//}

//////////////////////////////////////////////////////////////////////////////
//void TwainAppCMD::printAvailableDataSources()
//{
//	if (m_DSMState < 3)
//	{
//		PrintCMDMessage("The DSM has not been opened yet, please open it first\n");
//		return;
//	}

//	// print the Id and name of each available source
//    for (unsigned int x = 0; x < (m_DataSources).size(); ++x)
//	{
//        PrintCMDMessage("%d: %.33s by %.33s\n", (m_DataSources)[x].Id, (m_DataSources)[x].ProductName, (m_DataSources)[x].Manufacturer);
//	}

//	return;
//}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::loadDS(const TW_UINT32 _dsID)
{
    TwainApp::loadDS(_dsID);

    if (m_DSMState == 4)
    {
        initCaps();
    }
}
//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::unloadDS()
{
    uninitCaps();

    TwainApp::unloadDS();
}
//////////////////////////////////////////////////////////////////////////////
map<TW_UINT16,TW_UINT16> TwainAppCMD::initCaps()
{    
    map<TW_UINT16,   TW_UINT16> result;

    if (m_DSMState < 3)
    {
        PrintCMDMessage("The DSM needs to be opened first.\n");
        return result;
    }
    else if (m_DSMState < 4)
    {
        PrintCMDMessage("A Data Source needs to be opened first.\n");
        return result;
    }

    TW_UINT16 twcc;

    // get the default pixel type
    memset(&m_CAP_XFERCOUNT, 0, sizeof(TW_CAPABILITY));
    m_CAP_XFERCOUNT.Cap = CAP_XFERCOUNT;
    twcc = get_CAP(m_CAP_XFERCOUNT);
    result[CAP_XFERCOUNT] =twcc;

    memset(&m_ICAP_PIXELTYPE, 0, sizeof(TW_CAPABILITY));
    m_ICAP_PIXELTYPE.Cap = ICAP_PIXELTYPE;
    twcc = get_CAP(m_ICAP_PIXELTYPE);
    result[ICAP_PIXELTYPE] = twcc;


    memset(&m_ICAP_XFERMECH, 0, sizeof(TW_CAPABILITY));
    m_ICAP_XFERMECH.Cap = ICAP_XFERMECH;
    twcc =get_CAP(m_ICAP_XFERMECH);
    result[ICAP_XFERMECH] = twcc;

    memset(&m_ICAP_IMAGEFILEFORMAT, 0, sizeof(TW_CAPABILITY));
    m_ICAP_IMAGEFILEFORMAT.Cap = ICAP_IMAGEFILEFORMAT;
    twcc =get_CAP(m_ICAP_IMAGEFILEFORMAT);
    result[ICAP_IMAGEFILEFORMAT] = twcc;

    memset(&m_ICAP_COMPRESSION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_COMPRESSION.Cap = ICAP_COMPRESSION;
    twcc =get_CAP(m_ICAP_COMPRESSION);
    result[ICAP_COMPRESSION] = twcc;

    memset(&m_ICAP_JPEGQUALITY,0,sizeof(TW_CAPABILITY));
    m_ICAP_JPEGQUALITY.Cap = ICAP_JPEGQUALITY;
    twcc = get_CAP(m_ICAP_JPEGQUALITY);
    result[ICAP_JPEGQUALITY] = twcc;

    memset(&m_ICAP_UNITS, 0, sizeof(TW_CAPABILITY));
    m_ICAP_UNITS.Cap = ICAP_UNITS;
    twcc =get_CAP(m_ICAP_UNITS);
    result[ICAP_UNITS] = twcc;

    memset(&m_ICAP_BITDEPTH, 0, sizeof(TW_CAPABILITY));
    m_ICAP_BITDEPTH.Cap = ICAP_BITDEPTH;
    twcc =get_CAP(m_ICAP_BITDEPTH);
    result[ICAP_BITDEPTH] = twcc;

    memset(&m_ICAP_XRESOLUTION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_XRESOLUTION.Cap = ICAP_XRESOLUTION;
    twcc =get_CAP(m_ICAP_XRESOLUTION);
    result[ICAP_XRESOLUTION] = twcc;

    memset(&m_ICAP_YRESOLUTION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_YRESOLUTION.Cap = ICAP_YRESOLUTION;
    twcc =get_CAP(m_ICAP_YRESOLUTION);
    result[ICAP_YRESOLUTION] = twcc;

    memset(&m_ICAP_FRAMES, 0, sizeof(TW_CAPABILITY));
    m_ICAP_FRAMES.Cap = ICAP_FRAMES;
    twcc =get_CAP(m_ICAP_FRAMES);
    result[ICAP_FRAMES] = twcc;

    /* Capabilities added by JiaFang in Jan 2019 */
    memset(&m_ICAP_AUTODISCARDBLANKPAGES, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTODISCARDBLANKPAGES.Cap = ICAP_AUTODISCARDBLANKPAGES;
    twcc =get_CAP(m_ICAP_AUTODISCARDBLANKPAGES);
    result[ICAP_AUTODISCARDBLANKPAGES] = twcc;

    memset(&m_CAP_AUTOMATICSENSEMEDIUM, 0, sizeof(TW_CAPABILITY));
    m_CAP_AUTOMATICSENSEMEDIUM.Cap = CAP_AUTOMATICSENSEMEDIUM;
    twcc =get_CAP(m_CAP_AUTOMATICSENSEMEDIUM);
    result[CAP_AUTOMATICSENSEMEDIUM] = twcc;

    memset(&m_CAP_FEEDERENABLED, 0, sizeof(TW_CAPABILITY));
    m_CAP_FEEDERENABLED.Cap = CAP_FEEDERENABLED;
    twcc =get_CAP(m_CAP_FEEDERENABLED);
    result[CAP_FEEDERENABLED] = twcc;

    memset(&m_CAP_PAPERDETECTABLE, 0, sizeof(TW_CAPABILITY));
    m_CAP_PAPERDETECTABLE.Cap = CAP_PAPERDETECTABLE;
    twcc =get_CAP(m_CAP_PAPERDETECTABLE);
    result[CAP_PAPERDETECTABLE] = twcc;

    memset(&m_CAP_FEEDERLOADED, 0, sizeof(TW_CAPABILITY));
    m_CAP_FEEDERLOADED.Cap = CAP_FEEDERLOADED;
    twcc =get_CAP(m_CAP_FEEDERLOADED);
    result[CAP_FEEDERLOADED] = twcc;

    memset(&m_CAP_AUTOMATICCAPTURE, 0, sizeof(TW_CAPABILITY));
    m_CAP_AUTOMATICCAPTURE.Cap = CAP_AUTOMATICCAPTURE;
    twcc =get_CAP(m_CAP_AUTOMATICCAPTURE);
    result[CAP_AUTOMATICCAPTURE] = twcc;

    memset(&m_CAP_TIMEBEFOREFIRSTCAPTURE, 0, sizeof(TW_CAPABILITY));
    m_CAP_TIMEBEFOREFIRSTCAPTURE.Cap = CAP_TIMEBEFOREFIRSTCAPTURE;
    twcc =get_CAP(m_CAP_TIMEBEFOREFIRSTCAPTURE);
    result[CAP_TIMEBEFOREFIRSTCAPTURE] = twcc;

    memset(&m_CAP_TIMEBETWEENCAPTURES, 0, sizeof(TW_CAPABILITY));
    m_CAP_TIMEBETWEENCAPTURES.Cap = CAP_TIMEBETWEENCAPTURES;
    twcc =get_CAP(m_CAP_TIMEBETWEENCAPTURES);
    result[CAP_TIMEBETWEENCAPTURES] = twcc;

    memset(&m_CAP_DUPLEX, 0, sizeof(TW_CAPABILITY));
    m_CAP_DUPLEX.Cap = CAP_DUPLEX;
    twcc =get_CAP(m_CAP_DUPLEX);
    result[CAP_DUPLEX] = twcc;

    memset(&m_CAP_DUPLEXENABLED, 0, sizeof(TW_CAPABILITY));
    m_CAP_DUPLEXENABLED.Cap = CAP_DUPLEXENABLED;
    twcc =get_CAP(m_CAP_DUPLEXENABLED);
    result[CAP_DUPLEXENABLED] = twcc;

    memset(&m_ICAP_AUTOMATICCOLORENABLED, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOMATICCOLORENABLED.Cap = ICAP_AUTOMATICCOLORENABLED;
    twcc =get_CAP(m_ICAP_AUTOMATICCOLORENABLED);
    result[ICAP_AUTOMATICCOLORENABLED] = twcc;

    memset(&m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE.Cap = ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE;
    twcc =get_CAP(m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE);
    result[ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE] = twcc;

    memset(&m_ICAP_AUTOBRIGHT, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOBRIGHT.Cap = ICAP_AUTOBRIGHT;
    twcc =get_CAP(m_ICAP_AUTOBRIGHT);
    result[ICAP_AUTOBRIGHT] = twcc;

    memset(&m_ICAP_BRIGHTNESS, 0, sizeof(TW_CAPABILITY));
    m_ICAP_BRIGHTNESS.Cap = ICAP_BRIGHTNESS;
    twcc =get_CAP(m_ICAP_BRIGHTNESS);
    result[ICAP_BRIGHTNESS] = twcc;

    memset(&m_ICAP_CONTRAST, 0, sizeof(TW_CAPABILITY));
    m_ICAP_CONTRAST.Cap = ICAP_CONTRAST;
    twcc =get_CAP(m_ICAP_CONTRAST);
    result[ICAP_CONTRAST] = twcc;

    memset(&m_ICAP_UNDEFINEDIMAGESIZE, 0, sizeof(TW_CAPABILITY));
    m_ICAP_UNDEFINEDIMAGESIZE.Cap = ICAP_UNDEFINEDIMAGESIZE;
    twcc =get_CAP(m_ICAP_UNDEFINEDIMAGESIZE);
    result[ICAP_UNDEFINEDIMAGESIZE] = twcc;

    memset(&m_ICAP_AUTOMATICBORDERDETECTION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOMATICBORDERDETECTION.Cap = ICAP_AUTOMATICBORDERDETECTION;
    twcc =get_CAP(m_ICAP_AUTOMATICBORDERDETECTION);
    result[ICAP_AUTOMATICBORDERDETECTION] = twcc;

    memset(&m_ICAP_AUTOMATICROTATE, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOMATICROTATE.Cap = ICAP_AUTOMATICROTATE;
    twcc =get_CAP(m_ICAP_AUTOMATICROTATE);
    result[ICAP_AUTOMATICROTATE] = twcc;

    memset(&m_ICAP_AUTOMATICDESKEW, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOMATICDESKEW.Cap = ICAP_AUTOMATICDESKEW;
    twcc =get_CAP(m_ICAP_AUTOMATICDESKEW);
    result[ICAP_AUTOMATICDESKEW] = twcc;

    memset(&m_ICAP_AUTOSIZE, 0, sizeof(TW_CAPABILITY));
    m_ICAP_AUTOSIZE.Cap = ICAP_AUTOSIZE;
    twcc =get_CAP(m_ICAP_AUTOSIZE);
    result[ICAP_AUTOSIZE] = twcc;

    memset(&m_ICAP_SUPPORTEDSIZES, 0, sizeof(TW_CAPABILITY));
    m_ICAP_SUPPORTEDSIZES.Cap = ICAP_SUPPORTEDSIZES;
    twcc =get_CAP(m_ICAP_SUPPORTEDSIZES);
    result[ICAP_SUPPORTEDSIZES] = twcc;

    memset(&m_ICAP_MAXFRAMES, 0, sizeof(TW_CAPABILITY));
    m_ICAP_MAXFRAMES.Cap = ICAP_MAXFRAMES;
    twcc =get_CAP(m_ICAP_MAXFRAMES);
    result[ICAP_MAXFRAMES] = twcc;

    memset(&m_ICAP_ORIENTATION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_ORIENTATION.Cap = ICAP_ORIENTATION;
    twcc =get_CAP(m_ICAP_ORIENTATION);
    result[ICAP_ORIENTATION] = twcc;

    memset(&m_ICAP_ROTATION, 0, sizeof(TW_CAPABILITY));
    m_ICAP_ROTATION.Cap = ICAP_ROTATION;
    twcc =get_CAP(m_ICAP_ROTATION);
    result[ICAP_ROTATION] = twcc;
    return result;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::uninitCaps()
{
    if (m_DSMState < 3)
    {
        PrintCMDMessage("The DSM needs to be opened first.\n");
        return;
    }
    else if (m_DSMState < 4)
    {
        PrintCMDMessage("A Data Source needs to be opened first.\n");
        return;
    }

    if (m_CAP_XFERCOUNT.hContainer)
    {
        _DSM_Free(m_CAP_XFERCOUNT.hContainer);
        m_CAP_XFERCOUNT.hContainer = nullptr;
    }
    if (m_ICAP_PIXELTYPE.hContainer)
    {
        _DSM_Free(m_ICAP_PIXELTYPE.hContainer);
        m_ICAP_PIXELTYPE.hContainer = nullptr;
    }
    if (m_ICAP_XFERMECH.hContainer)
    {
        _DSM_Free(m_ICAP_XFERMECH.hContainer);
        m_ICAP_XFERMECH.hContainer = nullptr;
    }
    if (m_ICAP_IMAGEFILEFORMAT.hContainer)
    {
        _DSM_Free(m_ICAP_IMAGEFILEFORMAT.hContainer);
        m_ICAP_IMAGEFILEFORMAT.hContainer = nullptr;
    }
    if (m_ICAP_COMPRESSION.hContainer)
    {
        _DSM_Free(m_ICAP_COMPRESSION.hContainer);
        m_ICAP_COMPRESSION.hContainer = nullptr;
    }
    if (m_ICAP_UNITS.hContainer)
    {
        _DSM_Free(m_ICAP_UNITS.hContainer);
        m_ICAP_UNITS.hContainer = nullptr;
    }
    if (m_ICAP_BITDEPTH.hContainer)
    {
        _DSM_Free(m_ICAP_BITDEPTH.hContainer);
        m_ICAP_BITDEPTH.hContainer = nullptr;
    }
    if (m_ICAP_XRESOLUTION.hContainer)
    {
        _DSM_Free(m_ICAP_XRESOLUTION.hContainer);
        m_ICAP_XRESOLUTION.hContainer = nullptr;
    }
    if (m_ICAP_YRESOLUTION.hContainer)
    {
        _DSM_Free(m_ICAP_YRESOLUTION.hContainer);
        m_ICAP_YRESOLUTION.hContainer = nullptr;
    }
    if (m_ICAP_FRAMES.hContainer)
    {
        _DSM_Free(m_ICAP_FRAMES.hContainer);
        m_ICAP_FRAMES.hContainer = nullptr;
    }

    /* Capabilities added by JiaFang in Jan 2019 */
    if (m_ICAP_AUTODISCARDBLANKPAGES.hContainer)
    {
        _DSM_Free(m_ICAP_AUTODISCARDBLANKPAGES.hContainer);
        m_ICAP_AUTODISCARDBLANKPAGES.hContainer = nullptr;
    }
    if (m_CAP_AUTOMATICSENSEMEDIUM.hContainer)
    {
        _DSM_Free(m_CAP_AUTOMATICSENSEMEDIUM.hContainer);
        m_CAP_AUTOMATICSENSEMEDIUM.hContainer = nullptr;
    }

    if (m_CAP_FEEDERENABLED.hContainer)
    {
        _DSM_Free(m_CAP_FEEDERENABLED.hContainer);
        m_CAP_FEEDERENABLED.hContainer = nullptr;
    }
    if (m_CAP_PAPERDETECTABLE.hContainer)
    {
        _DSM_Free(m_CAP_PAPERDETECTABLE.hContainer);
        m_CAP_PAPERDETECTABLE.hContainer = nullptr;
    }
    if (m_CAP_FEEDERLOADED.hContainer)
    {
        _DSM_Free(m_CAP_FEEDERLOADED.hContainer);
        m_CAP_FEEDERLOADED.hContainer = nullptr;
    }

    if (m_CAP_AUTOMATICCAPTURE.hContainer)
    {
        _DSM_Free(m_CAP_AUTOMATICCAPTURE.hContainer);
        m_CAP_AUTOMATICCAPTURE.hContainer = nullptr;
    }
    if (m_CAP_TIMEBEFOREFIRSTCAPTURE.hContainer)
    {
        _DSM_Free(m_CAP_TIMEBEFOREFIRSTCAPTURE.hContainer);
        m_CAP_TIMEBEFOREFIRSTCAPTURE.hContainer = nullptr;
    }
    if (m_CAP_TIMEBETWEENCAPTURES.hContainer)
    {
        _DSM_Free(m_CAP_TIMEBETWEENCAPTURES.hContainer);
        m_CAP_TIMEBETWEENCAPTURES.hContainer = nullptr;
    }

    if (m_CAP_DUPLEX.hContainer)
    {
        _DSM_Free(m_CAP_DUPLEX.hContainer);
        m_CAP_DUPLEX.hContainer = nullptr;
    }
    if (m_CAP_DUPLEXENABLED.hContainer)
    {
        _DSM_Free(m_CAP_DUPLEXENABLED.hContainer);
        m_CAP_DUPLEXENABLED.hContainer = nullptr;
    }

    if (m_ICAP_AUTOMATICCOLORENABLED.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOMATICCOLORENABLED.hContainer);
        m_ICAP_AUTOMATICCOLORENABLED.hContainer = nullptr;
    }
    if (m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE.hContainer);
        m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE.hContainer = nullptr;
    }

    if (m_ICAP_AUTOBRIGHT.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOBRIGHT.hContainer);
        m_ICAP_AUTOBRIGHT.hContainer = nullptr;
    }
    if (m_ICAP_BRIGHTNESS.hContainer)
    {
        _DSM_Free(m_ICAP_BRIGHTNESS.hContainer);
        m_ICAP_BRIGHTNESS.hContainer = nullptr;
    }
    if (m_ICAP_CONTRAST.hContainer)
    {
        _DSM_Free(m_ICAP_CONTRAST.hContainer);
        m_ICAP_CONTRAST.hContainer = nullptr;
    }
    if (m_ICAP_UNDEFINEDIMAGESIZE.hContainer)
    {
        _DSM_Free(m_ICAP_UNDEFINEDIMAGESIZE.hContainer);
        m_ICAP_UNDEFINEDIMAGESIZE.hContainer = nullptr;
    }
    if (m_ICAP_AUTOMATICBORDERDETECTION.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOMATICBORDERDETECTION.hContainer);
        m_ICAP_AUTOMATICBORDERDETECTION.hContainer = nullptr;
    }
    if (m_ICAP_AUTOMATICROTATE.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOMATICROTATE.hContainer);
        m_ICAP_AUTOMATICROTATE.hContainer = nullptr;
    }
    if (m_ICAP_AUTOMATICDESKEW.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOMATICDESKEW.hContainer);
        m_ICAP_AUTOMATICDESKEW.hContainer = nullptr;
    }
    if (m_ICAP_AUTOSIZE.hContainer)
    {
        _DSM_Free(m_ICAP_AUTOSIZE.hContainer);
        m_ICAP_AUTOSIZE.hContainer = nullptr;
    }


    if (m_ICAP_SUPPORTEDSIZES.hContainer)
    {
        _DSM_Free(m_ICAP_SUPPORTEDSIZES.hContainer);
        m_ICAP_SUPPORTEDSIZES.hContainer = nullptr;
    }
    if (m_ICAP_MAXFRAMES.hContainer)
    {
        _DSM_Free(m_ICAP_MAXFRAMES.hContainer);
        m_ICAP_MAXFRAMES.hContainer = nullptr;
    }

    if (m_ICAP_ORIENTATION.hContainer)
    {
        _DSM_Free(m_ICAP_ORIENTATION.hContainer);
        m_ICAP_ORIENTATION.hContainer = nullptr;
    }
    if (m_ICAP_ROTATION.hContainer)
    {
        _DSM_Free(m_ICAP_ROTATION.hContainer);
        m_ICAP_ROTATION.hContainer = nullptr;
    }
    return;
}
//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::startScan()
{
    if (m_DSMState != 6)
    {
        printError(m_pDataSource, "A scan cannot be initiated unless we are in state 6");
        return;
    }

    TW_UINT16 mech;
    if (!getICAP_XFERMECH(mech))
    {
        printError(m_pDataSource, "Error: could not get the transfer mechanism");
        return;
    }

    switch (mech)
    {
    case TWSX_NATIVE:
        initiateTransfer_Native();
        break;

    case TWSX_FILE:
    {
        TW_UINT16 fileformat = TWFF_TIFF;
        if (!getICAP_IMAGEFILEFORMAT(fileformat))
        {
            // Default back to TIFF
            fileformat = TWFF_TIFF;
        }
        initiateTransfer_File(fileformat);
    }
        break;

    case TWSX_MEMORY:
        initiateTransfer_Memory();
        break;
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_CAP_XFERCOUNT(const TW_INT16 _count, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_XFERCOUNT, _count, TWTY_INT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_XFERCOUNT))
        {
            TW_INT16 count;
            if (getCAP_XFERCOUNT(count) &&
                    count == _count)
            {
                PrintCMDMessage("Capability successfully set!\n");
            }
        }
    }

    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_UNITS(const TW_UINT16 _val, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_UNITS, _val, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_UNITS))
        {
            if (TWON_ENUMERATION == m_ICAP_UNITS.ConType &&
                    nullptr != m_ICAP_UNITS.hContainer)
            {
                pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(m_ICAP_UNITS.hContainer);

                if (_val == pCapPT->ItemList[pCapPT->CurrentIndex])
                {
                    PrintCMDMessage("Capability successfully set!\n");

                    // successfully setting this cap means that we have to re-obtain the X/Y resolutions as well
                    get_CAP(m_ICAP_XRESOLUTION);
                    get_CAP(m_ICAP_YRESOLUTION);
                }
                _DSM_UnlockMemory(m_ICAP_UNITS.hContainer);
            }
        }

    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_PIXELTYPE(const TW_UINT16 _pt, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_PIXELTYPE, _pt, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_PIXELTYPE))
        {
            if (TWON_ENUMERATION == m_ICAP_PIXELTYPE.ConType &&
                    nullptr != m_ICAP_PIXELTYPE.hContainer)
            {
                pTW_ENUMERATION pCapPT = (pTW_ENUMERATION)_DSM_LockMemory(m_ICAP_PIXELTYPE.hContainer);

                if (_pt == ((TW_UINT16*)(&pCapPT->ItemList))[pCapPT->CurrentIndex])
                {
                    PrintCMDMessage("Capability successfully set!\n");
                }
                _DSM_UnlockMemory(m_ICAP_PIXELTYPE.hContainer);
            }
        }

        get_CAP(m_ICAP_BITDEPTH);

    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_RESOLUTION(const TW_UINT16 _ICAP, const pTW_FIX32 _pVal, bool reget/*=false*/)
{
    if ((ICAP_XRESOLUTION != _ICAP) &&
            (ICAP_YRESOLUTION != _ICAP))
    {
        printError(m_pDataSource, "Invalid resolution passed in! Resolution set failed.");
        return;
    }

    set_CapabilityOneValue(_ICAP, _pVal);

    // Get the new RESOLUTION caps values to see if the set was successfull.
    if (reget)
    {
        get_CAP(m_ICAP_XRESOLUTION);
        get_CAP(m_ICAP_YRESOLUTION);

        pTW_CAPABILITY pCapRes = nullptr;

        if (ICAP_XRESOLUTION == _ICAP)
        {
            pCapRes = &m_ICAP_XRESOLUTION;
        }
        else
        {
            pCapRes = &m_ICAP_YRESOLUTION;
        }

        // check ICAP_XRESOLUTION
        if (TWON_ENUMERATION == pCapRes->ConType &&
                nullptr != pCapRes->hContainer)
        {
            pTW_ENUMERATION_FIX32 pdat = (pTW_ENUMERATION_FIX32)pCapRes->hContainer;

            if (TWTY_FIX32 == pdat->ItemType &&
                    _pVal->Whole == pdat->ItemList[pdat->CurrentIndex].Whole &&
                    _pVal->Frac == pdat->ItemList[pdat->CurrentIndex].Frac)
            {
                PrintCMDMessage("Resolution successfully set!\n");
            }
        }
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_FRAMES(const pTW_FRAME _pFrame, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_FRAMES, _pFrame);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_FRAMES))
        {
            if (TWON_ENUMERATION == m_ICAP_FRAMES.ConType &&
                    nullptr != m_ICAP_FRAMES.hContainer)
            {
                pTW_ENUMERATION_FRAME pCapPT = (pTW_ENUMERATION_FRAME)_DSM_LockMemory(m_ICAP_FRAMES.hContainer);

                pTW_FRAME ptframe = &pCapPT->ItemList[pCapPT->CurrentIndex];

                if ((_pFrame->Bottom == ptframe->Bottom) &&
                        (_pFrame->Top == ptframe->Top) &&
                        (_pFrame->Left == ptframe->Left) &&
                        (_pFrame->Right == ptframe->Right))
                {
                    PrintCMDMessage("Frames successfully set!\n");
                }
                _DSM_UnlockMemory(m_ICAP_FRAMES.hContainer);
            }
        }
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_XFERMECH(const TW_UINT16 _mech, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_XFERMECH, _mech, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_XFERMECH))
        {
            TW_UINT16 mech;
            if (getICAP_XFERMECH(mech) &&
                    mech == _mech)
            {
                PrintCMDMessage("XferMech successfully set!\n");
            }
        }

        // Update compression and FileFormat after xfer is set
        get_CAP(m_ICAP_COMPRESSION);
        get_CAP(m_ICAP_IMAGEFILEFORMAT);

    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_IMAGEFILEFORMAT(const TW_UINT16 _fileformat, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_IMAGEFILEFORMAT, _fileformat, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_IMAGEFILEFORMAT))
        {
            TW_UINT16 fileformat;
            if (getICAP_IMAGEFILEFORMAT(fileformat) &&
                    fileformat == _fileformat)
            {
                PrintCMDMessage("ImageFileFormat successfully set!\n");
            }
        }

        // Update compression after xfer is set
        get_CAP(m_ICAP_COMPRESSION);

    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_COMPRESSION(const TW_UINT16 _comp, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_COMPRESSION, _comp, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_COMPRESSION))
        {
            TW_UINT16 comp;
            if (getICAP_COMPRESSION(comp) &&
                    comp == _comp)
            {
                PrintCMDMessage("Compression successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_JPEGQUALITY(const TW_INT16 _quality, bool reget)
{
    set_CapabilityOneValue(ICAP_JPEGQUALITY,_quality,TWTY_INT16);
    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_JPEGQUALITY))
        {
            TW_INT16 quality;
            if (getICAP_JPEGQUALITY(quality) && quality == _quality)
            {
                PrintCMDMessage("JEEG quality successfully set!\n");
            }
        }
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
void TwainAppCMD::set_ICAP_BITDEPTH(const TW_UINT16 _nVal, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_BITDEPTH, _nVal, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_BITDEPTH))
        {
            TW_UINT16 val;
            if (getICAP_BITDEPTH(val) &&
                    val == _nVal)
            {
                PrintCMDMessage("BitDepth successfully set!\n");
            }
        }
    }

    return;
}

void TwainAppCMD::set_ICAP_AUTODISCARDBLANKPAGES(const TW_INT32 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTODISCARDBLANKPAGES, _value, TWTY_INT32);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTODISCARDBLANKPAGES))
        {
            TW_INT32 val;
            if (getICAP_AUTODISCARDBLANKPAGES(val) && val == _value)
            {
                PrintCMDMessage("AutoDiscardBlankPages successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_AUTOMATICSENSEMEDIUM(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_AUTOMATICSENSEMEDIUM, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_AUTOMATICSENSEMEDIUM))
        {
            TW_BOOL val;
            if (getCAP_AUTOMATICSENSEMEDIUM(val) && val == _flag)
            {
                PrintCMDMessage("AutomaticSenseMedium successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_FEEDERENABLED(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_FEEDERENABLED, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_FEEDERENABLED))
        {
            TW_BOOL val;
            if (getCAP_FEEDERENABLED(val) && val == _flag)
            {
                PrintCMDMessage("FeederEnabled successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_AUTOSCAN(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_AUTOSCAN, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_AUTOSCAN))
        {
            TW_BOOL val;
            if (getCAP_AUTOSCAN(val) && val == _flag)
            {
                PrintCMDMessage("AutoScan successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_AUTOMATICCAPTURE(const TW_INT32 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_AUTOMATICCAPTURE, _value, TWTY_INT32);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_AUTOMATICCAPTURE))
        {
            TW_INT32 val;
            if (getCAP_AUTOMATICCAPTURE(val) && val == _value)
            {
                PrintCMDMessage("AutomaticCapture successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_TIMEBEFOREFIRSTCAPTURE(const TW_INT32 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_TIMEBEFOREFIRSTCAPTURE, _value, TWTY_INT32);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_TIMEBEFOREFIRSTCAPTURE))
        {
            TW_INT32 val;
            if (getCAP_TIMEBEFOREFIRSTCAPTURE(val) && val == _value)
            {
                PrintCMDMessage("TimeBeforeFirstCapture successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_TIMEBETWEENCAPTURES(const TW_INT32 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_TIMEBETWEENCAPTURES, _value, TWTY_INT32);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_TIMEBETWEENCAPTURES))
        {
            TW_INT32 val;
            if (getCAP_TIMEBETWEENCAPTURES(val) && val == _value)
            {
                PrintCMDMessage("TimeBetweenCaptures successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_CAP_DUPLEXENABLED(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(CAP_DUPLEXENABLED, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_CAP_DUPLEXENABLED))
        {
            TW_BOOL val;
            if (getCAP_DUPLEXENABLED(val) && val == _flag)
            {
                PrintCMDMessage("DuplexEnabled successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOMATICCOLORENABLED(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOMATICCOLORENABLED, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOMATICCOLORENABLED))
        {
            TW_BOOL val;
            if (getICAP_AUTOMATICCOLORENABLED(val) && val == _flag)
            {
                PrintCMDMessage("AutomaticColorEnabled successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE(const TW_UINT16 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE, _value, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE))
        {
            TW_UINT16 val;
            if (getICAP_AUTOMATICCOLORNONCOLORPIXELTYPE(val) && val == _value)
            {
                PrintCMDMessage("AutomaticColorNoncolorPixelType successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOBRIGHT(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOBRIGHT, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOBRIGHT))
        {
            TW_BOOL val;
            if (getICAP_AUTOBRIGHT(val) && val == _flag)
            {
                PrintCMDMessage("AutoBright successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_BRIGHTNESS(const pTW_FIX32 _pVal, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_BRIGHTNESS, _pVal);

    // Get the new BRIGHTNESS values to see if the set was successfull.
    if (reget)
    {
        get_CAP(m_ICAP_BRIGHTNESS);
        pTW_CAPABILITY pCapBrightness = &m_ICAP_BRIGHTNESS;

        if (TWON_ONEVALUE == pCapBrightness->ConType && nullptr != pCapBrightness->hContainer)
        {
            pTW_ONEVALUE_FIX32 pdat = (pTW_ONEVALUE_FIX32)pCapBrightness->hContainer;

            if (TWTY_FIX32 == pdat->ItemType &&
                    _pVal->Whole == pdat->Item.Whole &&
                    _pVal->Frac == pdat->Item.Frac)
            {
                PrintCMDMessage("Brightness successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_CONTRAST(const pTW_FIX32 _pVal, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_CONTRAST, _pVal);

    // Get the new Contrast values to see if the set was successfull.
    if (reget)
    {
        get_CAP(m_ICAP_CONTRAST);
        pTW_CAPABILITY pCapContrast = &m_ICAP_CONTRAST;

        if (TWON_ONEVALUE == pCapContrast->ConType && nullptr != pCapContrast->hContainer)
        {
            pTW_ONEVALUE_FIX32 pdat = static_cast<pTW_ONEVALUE_FIX32>(pCapContrast->hContainer);

            if (TWTY_FIX32 == pdat->ItemType &&
                    _pVal->Whole == pdat->Item.Whole &&
                    _pVal->Frac == pdat->Item.Frac)
            {
                PrintCMDMessage("Contrast successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_UNDEFINEDIMAGESIZE(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_UNDEFINEDIMAGESIZE, _flag, TWTY_BOOL);
    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_UNDEFINEDIMAGESIZE))
        {
            TW_BOOL val;
            if (getICAP_UNDEFINEDIMAGESIZE(val) && val == _flag)
            {
                PrintCMDMessage("UndefinedImageSize successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOMATICBORDERDETECTION(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOMATICBORDERDETECTION, _flag, TWTY_BOOL);
    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOMATICBORDERDETECTION))
        {
            TW_BOOL val;
            if (getICAP_AUTOMATICBORDERDETECTION(val) && val == _flag)
            {
                PrintCMDMessage("AutomaticBorderDetection successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOMATICROTATE(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOMATICROTATE, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOMATICROTATE))
        {
            TW_BOOL val;
            if (getICAP_AUTOMATICROTATE(val) && val == _flag)
            {
                PrintCMDMessage("AutomaticRotate successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOMATICDESKEW(const TW_BOOL _flag, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOMATICDESKEW, _flag, TWTY_BOOL);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOMATICDESKEW))
        {
            TW_BOOL val;
            if (getICAP_AUTOMATICDESKEW(val) && val == _flag)
            {
                PrintCMDMessage("AutomaticDeskew successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_AUTOSIZE(const TW_UINT16 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_AUTOSIZE, _value, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_AUTOSIZE))
        {
            TW_UINT16 val;
            if (getICAP_AUTOSIZE(val) && val == _value)
            {
                PrintCMDMessage("AutoSize successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_SUPPORTEDSIZES(const TW_UINT16 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_SUPPORTEDSIZES, _value, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_SUPPORTEDSIZES))
        {
            TW_UINT16 val;
            if (getICAP_SUPPORTEDSIZES(val) && val == _value)
            {
                PrintCMDMessage("SupportedSizes successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_MAXFRAMES(const TW_UINT16 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_MAXFRAMES, _value, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_MAXFRAMES))
        {
            TW_UINT16 val;
            if (getICAP_MAXFRAMES(val) && val == _value)
            {
                PrintCMDMessage("MaxFrames successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_ORIENTATION(const TW_UINT16 _value, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_ORIENTATION, _value, TWTY_UINT16);

    // now that we have set it, re-get it to ensure it was set
    if (reget)
    {
        if (TWCC_SUCCESS == get_CAP(m_ICAP_ORIENTATION))
        {
            TW_UINT16 val;
            if (getICAP_ORIENTATION(val) && val == _value)
            {
                PrintCMDMessage("Orientation successfully set!\n");
            }
        }
    }
    return;
}

void TwainAppCMD::set_ICAP_ROTATION(const pTW_FIX32 _pVal, bool reget/*=false*/)
{
    set_CapabilityOneValue(ICAP_ROTATION, _pVal);

    // Get the new Rotation value to see if the set was successfull.
    if (reget)
    {
        get_CAP(m_ICAP_ROTATION);
        pTW_CAPABILITY pRotation = &m_ICAP_ROTATION;

        if (TWON_ONEVALUE == pRotation->ConType && nullptr != pRotation->hContainer)
        {
            pTW_ONEVALUE_FIX32 pdat = (pTW_ONEVALUE_FIX32)pRotation->hContainer;

            if (TWTY_FIX32 == pdat->ItemType &&
                    _pVal->Whole == pdat->Item.Whole &&
                    _pVal->Frac == pdat->Item.Frac)
            {
                PrintCMDMessage("Rotation successfully set!\n");
            }
        }
    }
    return;
}

//////////////////////////////////////////////////////////////////////////////
bool TwainAppCMD::getICAP_UNITS(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_UNITS, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_XFERCOUNT(TW_INT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_XFERCOUNT, val);
    _val = (TW_INT16)val;
    return rtn;
}

bool TwainAppCMD::getICAP_XFERMECH(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_XFERMECH, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_PIXELTYPE(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_PIXELTYPE, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_BITDEPTH(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_BITDEPTH, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_IMAGEFILEFORMAT(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_IMAGEFILEFORMAT, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_COMPRESSION(TW_UINT16& _val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_COMPRESSION, val);
    _val = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_JPEGQUALITY(TW_INT16 &_val)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_JPEGQUALITY, val);
    _val = (TW_INT16)val;
    return rtn;
}

bool TwainAppCMD::getICAP_XRESOLUTION(TW_FIX32& _xres)
{
    return getCurrent(&m_ICAP_XRESOLUTION, _xres);
}

bool TwainAppCMD::getICAP_YRESOLUTION(TW_FIX32& _yres)
{
    return getCurrent(&m_ICAP_YRESOLUTION, _yres);
}

/********************* below: Capabilities added by JiaFang in Jan 2019 ********************************/

bool TwainAppCMD::getICAP_AUTODISCARDBLANKPAGES(TW_INT32 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTODISCARDBLANKPAGES, val);
    _value = static_cast<TW_INT32>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_AUTOMATICSENSEMEDIUM(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_AUTOMATICSENSEMEDIUM, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_FEEDERENABLED(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_FEEDERENABLED, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_PAPERDETECTABLE(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_PAPERDETECTABLE, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_FEEDERLOADED(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_FEEDERLOADED, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_AUTOSCAN(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_AUTOSCAN, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_AUTOMATICCAPTURE(TW_INT32 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_AUTOMATICCAPTURE, val);
    _value = static_cast<TW_INT32>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_TIMEBEFOREFIRSTCAPTURE(TW_INT32 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_TIMEBEFOREFIRSTCAPTURE, val);
    _value = static_cast<TW_INT32>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_TIMEBETWEENCAPTURES(TW_INT32 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_TIMEBETWEENCAPTURES, val);
    _value = static_cast<TW_INT32>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_DUPLEX(TW_UINT16 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_DUPLEX, val);
    _value = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getCAP_DUPLEXENABLED(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_CAP_DUPLEXENABLED, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOMATICCOLORENABLED(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOMATICCOLORENABLED, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOMATICCOLORNONCOLORPIXELTYPE(TW_UINT16 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOMATICCOLORNONCOLORPIXELTYPE, val);
    _value = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOBRIGHT(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOBRIGHT, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_BRIGHTNESS(TW_FIX32 & _value)
{
    return getCurrent(&m_ICAP_BRIGHTNESS, _value);
}

bool TwainAppCMD::getICAP_CONTRAST(TW_FIX32 & _value)
{
    return getCurrent(&m_ICAP_CONTRAST, _value);
}

bool TwainAppCMD::getICAP_UNDEFINEDIMAGESIZE(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_UNDEFINEDIMAGESIZE, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOMATICBORDERDETECTION(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOMATICBORDERDETECTION, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOMATICROTATE(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOMATICROTATE, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOMATICDESKEW(TW_BOOL & _flag)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOMATICDESKEW, val);
    _flag = static_cast<TW_BOOL>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_AUTOSIZE(TW_UINT16 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_AUTOSIZE, val);
    _value = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_SUPPORTEDSIZES(TW_UINT16 & _value)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_SUPPORTEDSIZES, val);
    _value = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_FRAMES(TW_FRAME &_frame)
{
    TW_FRAME f;
    bool rtn = getCurrent(&m_ICAP_FRAMES, f);
    _frame = static_cast<TW_FRAME>(f);
    return rtn;
}

bool TwainAppCMD::getICAP_MAXFRAMES(TW_UINT16 & _num)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_MAXFRAMES, val);
    _num = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_ORIENTATION(TW_UINT16 & _orientation)
{
    TW_UINT32 val;
    bool rtn = getCurrent(&m_ICAP_ORIENTATION, val);
    _orientation = static_cast<TW_UINT16>(val);
    return rtn;
}

bool TwainAppCMD::getICAP_ROTATION(TW_FIX32 & _degree)
{
    return getCurrent(&m_ICAP_ROTATION, _degree);
}
