#ifndef MYTESTAPP_H
#define MYTESTAPP_H
#include "twainAppInterface.h"

class MyTestApp:public TwainAppInterface
{
public:
    MyTestApp(HWND parent = nullptr);
    ~MyTestApp();

    virtual TwainApp::DSMCallback* getDSMCallbackFunction();

        //virtual void printErrorText(const string text);
        //virtual void printCommonText(const string text);

        /**
    * Fills an identity structure with this applications identity details.
    * Initialize the Apps identity so the DSM can uniquely identify it.
    * @param[out] _identity a structure that will get filled with Applications details
    */
        virtual void fillIdentity(TW_IDENTITY& _identity);

        /**
        * Returns an error message that can be used when a type is of an unexpected type.
        * @param[in] _unExpected the expected type
        * @param[in] _unReceived the received type
        * @return the error string
        */
        virtual string getErrorString_UnexpectedType(const TW_UINT16 _unExpected, const TW_UINT16 _unReceived);

        /**
        * prints the main menu
        */
        virtual void printOptions();

        /**
        * prints the main capabilities menu
        */
        virtual void printMainCaps();

        /**
        * prints the ICAP_xxxxx menu for type TW_ONEVALUE
        * @param[in] _unCap The cap to print
        * @param[in] _pCap pointer to the TW_ONEVALUE returned by the source for this cap
        */
        virtual void print_ICAP(const TW_UINT16 _unCap, pTW_ONEVALUE _pCap);

        /**
        * @overload void print_ICAP(const TW_UINT16 _unCap, pTW_ENUMERATION _pCap)
        * prints the ICAP_xxxxx menu for type TW_ENUMERATION
        * @param[in] _unCap The cap to print
        * @param[in] _pCap pointer to the TW_ENUMERATION returned by the source for this cap
        */
        virtual void print_ICAP(const TW_UINT16 _unCap, pTW_ENUMERATION _pCap);

};

#endif // MYTESTAPP_H
