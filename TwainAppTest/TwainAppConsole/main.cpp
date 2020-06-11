#include <QCoreApplication>
#include "mytestapp.h"
#include <iostream>
using namespace std;

// Global Variables
MyTestApp * gpTwainApplicationCMD;  /**< The main application */

/**
* Display exit message.
* @param[in] _sig not used.
*/
void onSigINT(int _sig)
{
    UNUSEDARG(_sig);
    cout << "\nGoodbye!" << endl;
    exit(0);
}

/**
* Negotiate a capabilities between the app and the DS
* @param[in] _pCap the capabilities to negotiate
*/
void negotiate_CAP(const pTW_CAPABILITY _pCap)
{
    string input;

    // -Setting one cap could change another cap so always refresh the caps
    // before working with another one.
    // -Another method of doing this is to let the DS worry about the state
    // of the caps instead of keeping a copy in the app like I'm doing.
    gpTwainApplicationCMD->initCaps();

    for (;;)
    {
        if ((TWON_ENUMERATION == _pCap->ConType) ||
            (TWON_ONEVALUE == _pCap->ConType))
        {
            //TW_MEMREF pVal = _DSM_LockMemory(_pCap->hContainer);
            TW_MEMREF pVal = gpTwainApplicationCMD->S_DSM_LockMemory(_pCap->hContainer);
            // print the caps current value
            if (TWON_ENUMERATION == _pCap->ConType)
            {
                gpTwainApplicationCMD->print_ICAP(_pCap->Cap, (pTW_ENUMERATION)(pVal));
            }
            else // TWON_ONEVALUE
            {
                gpTwainApplicationCMD->print_ICAP(_pCap->Cap, (pTW_ONEVALUE)(pVal));
            }

            cout << "\nset cap# > ";
            cin >> input;
            cout << endl;

            if ("q" == input)
            {
                gpTwainApplicationCMD->S_DSM_UnlockMemory(_pCap->hContainer);
                break;
            }
            else
            {
                int n = atoi(input.c_str());
                TW_UINT16  valUInt16 = 0;
                pTW_FIX32  pValFix32 = { 0 };
                pTW_FRAME  pValFrame = { 0 };

                // print the caps current value
                if (TWON_ENUMERATION == _pCap->ConType)
                {
                    switch (((pTW_ENUMERATION)pVal)->ItemType)
                    {
                    case TWTY_UINT16:
                        valUInt16 = ((pTW_UINT16)(&((pTW_ENUMERATION)pVal)->ItemList))[n];
                        break;

                    case TWTY_FIX32:
                        pValFix32 = &((pTW_ENUMERATION_FIX32)pVal)->ItemList[n];
                        break;

                    case TWTY_FRAME:
                        pValFrame = &((pTW_ENUMERATION_FRAME)pVal)->ItemList[n];
                        break;
                    }

                    switch (_pCap->Cap)
                    {
                    case ICAP_PIXELTYPE:
                        gpTwainApplicationCMD->set_ICAP_PIXELTYPE(valUInt16);
                        break;

                    case ICAP_BITDEPTH:
                        gpTwainApplicationCMD->set_ICAP_BITDEPTH(valUInt16);
                        break;

                    case ICAP_UNITS:
                        gpTwainApplicationCMD->set_ICAP_UNITS(valUInt16);
                        break;

                    case ICAP_XFERMECH:
                        gpTwainApplicationCMD->set_ICAP_XFERMECH(valUInt16);
                        break;

                    case ICAP_XRESOLUTION:
                    case ICAP_YRESOLUTION:
                        gpTwainApplicationCMD->set_ICAP_RESOLUTION(_pCap->Cap, pValFix32);
                        break;

                    case ICAP_FRAMES:
                        gpTwainApplicationCMD->set_ICAP_FRAMES(pValFrame);
                        break;

                    default:
                        cerr << "Unsupported capability" << endl;
                        break;
                    }
                }
            }
            gpTwainApplicationCMD->S_DSM_UnlockMemory(_pCap->hContainer);
        }
        else
        {
            cerr << "Unsupported capability" << endl;
            break;
        }
    }

    return;
}

/**
* drives main capabilities menu.  Negotiate all capabilities
*/
void negotiateCaps()
{
    // If the app is not in state 4, don't even bother showing this menu.
    if (gpTwainApplicationCMD->m_DSMState < 4)
    {
        cerr << "\nNeed to open a source first\n" << endl;
        return;
    }

    string input;

    // Loop forever until either SIGINT is heard or user types done to go back
    // to the main menu.
    for (;;)
    {
        gpTwainApplicationCMD->printMainCaps();
        cout << "\n(h for help) > ";
        cin >> input;
        cout << endl;

        if ("q" == input)
        {
            break;
        }
        else if ("h" == input)
        {
            gpTwainApplicationCMD->printMainCaps();
        }
        else if (input >= "1" && input <= "100")
        {
            int n = atoi(input.c_str());

            switch (n)
            {
            case 1:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_XFERMECH));
                break;
            case 2:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_PIXELTYPE));
                break;
            case 3:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_BITDEPTH));
                break;
            case 4:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_XRESOLUTION));
                break;
            case 5:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_YRESOLUTION));
                break;
            case 6:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_FRAMES));
                break;
            case 7:
                negotiate_CAP(&(gpTwainApplicationCMD->m_ICAP_UNITS));
                break;
//            case 8:/**/

//                break;
            default:
                break;
            }         
        }
        else
        {
            gpTwainApplicationCMD->printMainCaps();
        }
    }

    return;
}

/**
* Enables the source. The source will let us know when it is ready to scan by
* calling our registered callback function.
*/
void EnableDS()
{
    gpTwainApplicationCMD->m_DSMessage = 0;
#ifdef TWNDS_OS_LINUX

    int test;
    sem_getvalue(&(gpTwainApplicationCMD->m_TwainEvent), &test);
    while (test < 0)
    {
        sem_post(&(gpTwainApplicationCMD->m_TwainEvent));    // Event semaphore Handle
        sem_getvalue(&(gpTwainApplicationCMD->m_TwainEvent), &test);
    }
    while (test > 0)
    {
        sem_wait(&(gpTwainApplicationCMD->m_TwainEvent)); // event semaphore handle
        sem_getvalue(&(gpTwainApplicationCMD->m_TwainEvent), &test);
    }

#endif
    // -Enable the data source. This puts us in state 5 which means that we
    // have to wait for the data source to tell us to move to state 6 and
    // start the transfer.  Once in state 5, no more set ops can be done on the
    // caps, only get ops.
    // -The scan will not start until the source calls the callback function
    // that was registered earlier.
#ifdef TWNDS_OS_WIN
    if (!gpTwainApplicationCMD->enableDS(GetDesktopWindow(), FALSE))
#else
    if (!gpTwainApplicationCMD->enableDS(0, TRUE))
#endif
    {
        return;
    }

#ifdef TWNDS_OS_WIN
    // now we have to wait until we hear something back from the DS.
    while (!gpTwainApplicationCMD->m_DSMessage)
    {
        TW_EVENT twEvent = { 0 };

        // If we are using callbacks, there is nothing to do here except sleep
        // and wait for our callback from the DS.  If we are not using them,
        // then we have to poll the DSM.

        // Pumping messages is for Windows only
        MSG Msg;
        if (!GetMessage((LPMSG)&Msg, nullptr, 0, 0))
        {
            break;//WM_QUIT
        }
        twEvent.pEvent = (TW_MEMREF)&Msg;

        twEvent.TWMessage = MSG_NULL;
        TW_UINT16  twRC = TWRC_NOTDSEVENT;
        twRC = gpTwainApplicationCMD->DSM_Entry(
            DG_CONTROL,
            DAT_EVENT,
            MSG_PROCESSEVENT,
            (TW_MEMREF)&twEvent);

        if (!gpTwainApplicationCMD->gUSE_CALLBACKS && twRC == TWRC_DSEVENT)
        {
            // check for message from Source
            switch (twEvent.TWMessage)
            {
            case MSG_XFERREADY:
            case MSG_CLOSEDSREQ:
            case MSG_CLOSEDSOK:
            case MSG_NULL:
                gpTwainApplicationCMD->m_DSMessage = twEvent.TWMessage;
                break;

            default:
                cerr << "\nError - Unknown message in MSG_PROCESSEVENT loop\n" << endl;
                break;
            }
        }
        if (twRC != TWRC_DSEVENT)
        {
            TranslateMessage((LPMSG)&Msg);
            DispatchMessage((LPMSG)&Msg);
        }
    }
#elif defined(TWNDS_OS_LINUX)
    // Wait for the event be signaled
    sem_wait(&(gpTwainApplicationCMD->m_TwainEvent)); // event semaphore handle
                              // Indefinite wait
#endif

  // At this point the source has sent us a callback saying that it is ready to
  // transfer the image.

    if (gpTwainApplicationCMD->m_DSMessage == MSG_XFERREADY)
    {
        // move to state 6 as a result of the data source. We can start a scan now.
        gpTwainApplicationCMD->m_DSMState = 6;

        gpTwainApplicationCMD->startScan();
    }

    // Scan is done, disable the ds, thus moving us back to state 4 where we
    // can negotiate caps again.
    gpTwainApplicationCMD->disableDS();

    return;
}

/**
* Callback funtion for DS.  This is a callback function that will be called by
* the source when it is ready for the application to start a scan. This
* callback needs to be registered with the DSM before it can be called.
* It is important that the application returns right away after recieving this
* message.  Set a flag and return.  Do not process the callback in this function.
*/
#ifdef TWH_CMP_MSC
TW_UINT16 FAR PASCAL
#else
FAR PASCAL TW_UINT16
#endif
DSMCallbackFunc(pTW_IDENTITY _pOrigin,
    pTW_IDENTITY _pDest,
    TW_UINT32    _DG,
    TW_UINT16    _DAT,
    TW_UINT16    _MSG,
    TW_MEMREF    _pData)
{
    UNUSEDARG(_pDest);
    UNUSEDARG(_DG);
    UNUSEDARG(_DAT);
    UNUSEDARG(_pData);

    TW_UINT16 twrc = TWRC_SUCCESS;

    // we are only waiting for callbacks from our datasource, so validate
    // that the originator.
    if (nullptr == _pOrigin ||
        _pOrigin->Id != gpTwainApplicationCMD->getDataSource()->Id)
    {
        return TWRC_FAILURE;
    }
    switch (_MSG)
    {
    case MSG_XFERREADY:
    case MSG_CLOSEDSREQ:
    case MSG_CLOSEDSOK:
    case MSG_NULL:
        gpTwainApplicationCMD->m_DSMessage = _MSG;
        // now signal the event semaphore
#ifdef TWNDS_OS_LINUX
        {
            int test = 12345;
            sem_post(&(gpTwainApplicationCMD->m_TwainEvent));    // Event semaphore Handle
        }
#endif
        break;

    default:
        cerr << "Error - Unknown message in callback routine" << endl;
        twrc = TWRC_FAILURE;
        break;
    }

    return twrc;
}


//////////////////////////////////////////////////////////////////////////////
//void improveImg()
//{
//	CCadreArchImprove::improve("D://H005.jpg");
//}


int cmd_on_Twain(int argc, char *argv[])
{
    UNUSEDARG(argc);
    UNUSEDARG(argv);
    int ret = EXIT_SUCCESS;

    // Instantiate the TWAIN application CMD class
    HWND parentWindow = nullptr;

#if defined (TWH_CMP_MSC) || defined(TWNDS_OS_WIN)
    parentWindow = GetConsoleWindow();
#endif
    gpTwainApplicationCMD = new MyTestApp(parentWindow);

    // setup a signal handler for SIGINT that will allow the program to stop
    signal(SIGINT, &onSigINT);

    string input;

    gpTwainApplicationCMD->printOptions();

    // start the main event loop
    for (;;)
    {
        cout << "\n(h for help) > ";
        cin >> input;
        cout << endl;

        if ("q" == input)
        {
            break;
        }
        else if ("h" == input)
        {
            gpTwainApplicationCMD->printOptions();
        }
        else if ("cdsm" == input)
        {
            gpTwainApplicationCMD->connectDSM();
        }
        else if ("xdsm" == input)
        {
            gpTwainApplicationCMD->disconnectDSM();
        }
        else if ("lds" == input)
        {
            gpTwainApplicationCMD->printAvailableDataSources();
        }
        else if ("pds" == input.substr(0, 3))
        {
            gpTwainApplicationCMD->printIdentityStruct(atoi(input.substr(3, input.length() - 3).c_str()));
        }
        else if ("cds" == input.substr(0, 3))
        {
            gpTwainApplicationCMD->loadDS(atoi(input.substr(3, input.length() - 3).c_str()));
        }
        else if ("xds" == input)
        {
            gpTwainApplicationCMD->unloadDS();
        }
        else if ("caps" == input)
        {
            if (gpTwainApplicationCMD->m_DSMState < 3)
            {
                cout << "\nYou need to select a source first!" << endl;
            }
            else
            {
                negotiateCaps();
                gpTwainApplicationCMD->printOptions();
            }
        }
        else if ("scan" == input)
        {
            EnableDS();
        }
        else
        {
            // default action
            gpTwainApplicationCMD->printOptions();
        }
    }

    gpTwainApplicationCMD->exit();
    delete gpTwainApplicationCMD;
    gpTwainApplicationCMD = nullptr;

    return ret;
}

/**
* Display the last windows error messages.
*/
void printWindowsErrorMessage()
{
    LPVOID lpMsgBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, nullptr);

    cerr << "Error: id [" << dw << "] msg [" << lpMsgBuf << "]" << endl;

    LocalFree(lpMsgBuf);
}



int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);
//    return a.exec();

    cmd_on_Twain(argc, argv);
}
