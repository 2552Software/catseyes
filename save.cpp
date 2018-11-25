// install.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Mincore.lib")
#pragma comment(lib, "comsuppw.lib")

// 
//   FUNCTION: AutoWrap(int, VARIANT*, IDispatch*, LPOLESTR, int,...) 
// 
//   PURPOSE: Automation helper function. It simplifies most of the low-level  
//      details involved with using IDispatch directly. Feel free to use it  
//      in your own implementations. One caveat is that if you pass multiple  
//      parameters, they need to be passed in reverse-order. 
// 
//   PARAMETERS: 
//      * autoType - Could be one of these values: DISPATCH_PROPERTYGET,  
//      DISPATCH_PROPERTYPUT, DISPATCH_PROPERTYPUTREF, DISPATCH_METHOD. 
//      * pvResult - Holds the return value in a VARIANT. 
//      * pDisp - The IDispatch interface. 
//      * ptName - The property/method name exposed by the interface. 
//      * cArgs - The count of the arguments. 
// 
//   RETURN VALUE: An HRESULT value indicating whether the function succeeds  
//      or not.  
// 
//   EXAMPLE:  
//      AutoWrap(DISPATCH_METHOD, NULL, pDisp, L"call", 2, parm[1], parm[0]); 
// 
HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...)
{
    // Begin variable-argument list 
    va_list marker;
    va_start(marker, cArgs);

    if (!pDisp) {
        _putws(L"NULL IDispatch passed to AutoWrap()");
        return E_INVALIDARG;
    }

    // Variables used 
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;

    // Get DISPID for name passed 
    hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if (FAILED(hr)) {
        if (hr == RPC_E_SERVERCALL_RETRYLATER) {
            wprintf(L"PowerPoint busy try again\n");
        }
        else if (hr == DISP_E_UNKNOWNNAME) {
            wprintf(L"unknown name IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx\n", ptName, hr);
        }
        else {
            wprintf(L"IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx\n", ptName, hr);
        }
        return hr;
    }

    // Allocate memory for arguments 
    VARIANT *pArgs = new VARIANT[cArgs + 1];
    // Extract arguments... 
    for (int i = 0; i < cArgs; i++) {
        pArgs[i] = va_arg(marker, VARIANT);
    }

    // Build DISPPARAMS 
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts 
    if (autoType & DISPATCH_PROPERTYPUT) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    // Make the call 
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
    if (FAILED(hr)) {
        if (hr == DISP_E_EXCEPTION) {
            wprintf(L"IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx DISP_E_EXCEPTION, really bad\n", ptName, dispID, hr);
        }
        else {
            wprintf(L"IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx\n", ptName, dispID, hr);
        }
        return hr;
    }

    // End variable-argument section 
    va_end(marker);

    delete[] pArgs;

    return hr;
}

DWORD FindProcessId(const std::wstring& processName) {

    PROCESSENTRY32W processInfo;
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (processesSnapshot == INVALID_HANDLE_VALUE)
        return 0;

    Process32FirstW(processesSnapshot, &processInfo);
    std::wstring next(processInfo.szExeFile);
    _putws(processInfo.szExeFile);
    if (next.find(processName) != std::string::npos) {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32NextW(processesSnapshot, &processInfo)) {
        _putws(processInfo.szExeFile);
        std::wstring next(processInfo.szExeFile);
        if (next.find(processName) != std::string::npos) {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);
    return 0;
}

void justEcho(const _bstr_t target, const std::wstring& name, _bstr_t& bs) {
    std::wstring s = (wchar_t*)target;
    wprintf_s(L"target %s, %s = %s\n", s.c_str(), name.c_str(), static_cast<wchar_t*>(bs));
}

void echoElement(IUIAutomationElement* element, const _bstr_t target, BOOL& b, _bstr_t& cls, _bstr_t& name, _bstr_t& id) {
    if (element) {
        element->get_CurrentIsControlElement(&b);
        b = true;//bugbug just fo rnow
        element->get_CurrentClassName(cls.GetAddress());
        justEcho(target, L"cls", cls);
        element->get_CurrentName(name.GetAddress());
        justEcho(target, L"name", name);
        _bstr_t status;
        element->get_CurrentItemStatus(status.GetAddress());
        justEcho(target, L"status", status);
        _bstr_t desc;
        element->get_CurrentLocalizedControlType(desc.GetAddress());
        justEcho(target, L"desc", desc);
        element->get_CurrentAutomationId(id.GetAddress());
        justEcho(target, L"id", id);
    }
}

bool set(IUIAutomationElement* pNode) {
    if (pNode) {
        CComPtr<IUIAutomationTogglePattern> pToggle;
        pNode->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern), (void **)&pToggle);
        if (pToggle) {
            ToggleState state;
            pToggle->get_CurrentToggleState(&state);
            if (state != ToggleState_On) {
                pToggle->Toggle(); 
            }
            return true;
        }
    }
    else {
        wprintf_s(L"insert null\n");
    }
    return false;
}
bool insert(IUIAutomationElement* pNode, const std::wstring& data) {
    if (pNode) {
        BOOL enabled;
        pNode->get_CurrentIsEnabled(&enabled);
        if (!enabled) {
            // set error
            return false;
        }
        pNode->get_CurrentIsKeyboardFocusable(&enabled);
        if (enabled) {
            CComPtr<IUIAutomationValuePattern> pval;
            pNode->GetCurrentPatternAs(UIA_ValuePatternId, __uuidof(IUIAutomationValuePattern), (void **)&pval);
            pNode->SetFocus();
            if (pval) {
                _bstr_t val(data.c_str());
                pval->SetValue(val.GetBSTR());
                return true;
            }
        }
    }
    else {
        wprintf_s(L"insert null\n");
    }
    return false;
}
bool invoke(IUIAutomationElement* pNode) {
    if (pNode) {
        CComPtr<IUIAutomationInvokePattern> pInvoke;
        pNode->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void **)&pInvoke);
        if (pInvoke) {
            pInvoke->Invoke();
            return true;
        }
    }
    else {
        wprintf_s(L"invoke null\n");
    }
    return false;
}
bool select(IUIAutomationElement* pNode) {
    if (pNode) {
        CComPtr<IUIAutomationSelectionItemPattern> pSelect;
        pNode->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void **)&pSelect);
        if (pSelect) {
            pSelect->Select();
            return true;
        }
    }
    else {
        wprintf_s(L"select null\n");
    }
    return false;
}

bool findDescendantByClass(IUIAutomation* pAutomation, IUIAutomationElement * pSender, _variant_t className){
    if (!pSender)   {
        return false;
    }
    CComPtr<IUIAutomationCondition> pCondition;
    pAutomation->CreatePropertyCondition(UIA_ClassNamePropertyId, className, &pCondition);
    CComPtr<IUIAutomationElementArray> all;
    pSender->FindAll(TreeScope_Subtree, pCondition, &all);
    CComPtr<IUIAutomationElement> ret;
    if (all) {
        int len;
        all->get_Length(&len);
        for (int i = 0; i < len; ++i) {
            CComPtr<IUIAutomationElement> pFound;
            all->GetElement(i, &pFound);
            _bstr_t name;
            pFound->get_CurrentName(name.GetAddress());
            std::wstring test = (wchar_t*)name;
            if (test.find(L"Don't allow any web add-ins to start.") != std::string::npos) {
                select(pFound); // I assume we need to restore this if its not set
            }
            if (test.find(L"Contiq") != std::string::npos) {
                select(pFound); // other selected items will be deleted also unless we save and re-select
            }
        }
    }
    return false;
}
// learning tool
void tryall(IUIAutomation* pAutomation, IUIAutomationElement * pSender, _variant_t target) {
    CComPtr<IUIAutomationCondition> pCondition;
    pAutomation->CreatePropertyCondition(UIA_NamePropertyId, target, &pCondition);
    CComPtr<IUIAutomationElementArray> all;
    pSender->FindAll(TreeScope_Subtree, pCondition, &all);
    CComPtr<IUIAutomationElement> ret;
    if (all) {
        int len;
        all->get_Length(&len);
        for (int i = 0; i < len; ++i) {
            CComPtr<IUIAutomationElement> pFound;
            all->GetElement(i, &pFound);
            _bstr_t cls;
            HRESULT hr = pFound->get_CurrentClassName(cls.GetAddress());
            if (!IS_ERROR(hr)) {
                pFound->SetFocus();
                select(pFound);
                invoke(pFound);
            }
            _bstr_t n;
            hr = pFound->get_CurrentName(n.GetAddress());
            if (hr == UIA_E_ELEMENTNOTAVAILABLE) {
                wprintf(L">> UIA_E_ELEMENTNOTAVAILABLE\n");
            }
        }
    }

}

CComPtr<IUIAutomationElement> find(IUIAutomation* pAutomation, IUIAutomationElement * pSender, _variant_t target, const std::wstring& targetClass) {
    CComPtr<IUIAutomationCondition> pCondition;
    pAutomation->CreatePropertyCondition(UIA_NamePropertyId, target, &pCondition);
    CComPtr<IUIAutomationElementArray> all;
    pSender->FindAll(TreeScope_Subtree, pCondition, &all);
    CComPtr<IUIAutomationElement> ret;
    if (all) {
        int len;
        all->get_Length(&len);
        for (int i = 0; i < len; ++i) {
            CComPtr<IUIAutomationElement> pFound;
            all->GetElement(i, &pFound);
            _bstr_t cls;
            HRESULT hr = pFound->get_CurrentClassName(cls.GetAddress());
            if (!IS_ERROR(hr)) {
                std::wstring test = (wchar_t*)cls;
                if (targetClass == test || targetClass.length() == 0) {
                    ret = pFound; // loop to see all values when debugging
                }
            }
        }
    }
    if (!ret) {
        wprintf_s(L"not found %s\n", targetClass.c_str());
    }
    return ret; // not found
}

class UICommand {
public:
    enum CommandType { Invoke, Select, Insert, EnableToggle, ExpandCollapse, Parse, ListItem };
    UICommand( const _bstr_t targetIn, CommandType cmdIn) { cmd = cmdIn; target = targetIn; }
    UICommand(const _bstr_t targetIn) { cmd = Invoke; target = targetIn; }
    UICommand(const _bstr_t targetIn, const std::wstring& dataIn, CommandType cmdIn = Invoke) { target = targetIn; data = dataIn;  cmd = cmdIn; }
    _bstr_t target;
    CommandType cmd;
    std::wstring data;
    DWORD sleep = 100UL;
};

DWORD WINAPI AutomatePowerPointByCOMAPI(LPVOID lpParam);
void postError(const std::wstring& error, DWORD e) {
    MessageBoxW(nullptr, error.c_str(), L"Contiq Error!\0", MB_ICONEXCLAMATION | MB_OK);
}
void postMessage(const std::wstring& message) {
    MessageBoxW(nullptr, message.c_str(), L"Contiq Message\0", MB_ICONINFORMATION | MB_OK);
}
int askQuestion(const std::wstring& question) {
    return MessageBoxW(nullptr, question.c_str(), L"Contiq Question\0", MB_ICONQUESTION | MB_YESNO);
}
std::wstring getShareName() {
    wchar_t  infoBuf[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD  bufCharCount = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerNameW(infoBuf, &bufCharCount);
    std::wstring share;
    share = L"\\\\";
    share += infoBuf;
    share += L"\\Contiq";
    return share;
}
class EventHandler : public IUIAutomationEventHandler {
private:
    LONG _refCount;
    std::wstring caption;
    std::wstring CheckName;
public:
    int _eventCount;
    DWORD pid;
    IUIAutomation* pAutomation;
    IDispatch* app;
    _variant_t presentation;
    IUIAutomationElement* pRoot;
    HANDLE event;
    int type;

    // Constructor.
    EventHandler(IUIAutomation* au, IUIAutomationElement* ru, HANDLE e, int t) :
        _refCount(1), _eventCount(0), pid(0UL), pAutomation(au), pRoot(ru), event(e), type(t){
    }

    // IUnknown methods.
    ULONG STDMETHODCALLTYPE AddRef()
    {
        ULONG ret = InterlockedIncrement(&_refCount);
        return ret;
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ret = InterlockedDecrement(&_refCount);
        if (ret == 0)
        {
            delete this;
            return 0;
        }
        return ret;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
    {
        if (riid == __uuidof(IUnknown))
            *ppInterface = static_cast<IUIAutomationEventHandler*>(this);
        else if (riid == __uuidof(IUIAutomationEventHandler))
            *ppInterface = static_cast<IUIAutomationEventHandler*>(this);
        else
        {
            *ppInterface = NULL;
            return E_NOINTERFACE;
        }
        this->AddRef();
        return S_OK;
    }
    bool isMe(IUIAutomationElement * pSender) {
        int  pidCurrent=0;
        HRESULT hr = pSender->get_CurrentProcessId(&pidCurrent);
        return pid && pidCurrent == pid;
    }
    // IUIAutomationEventHandler methods
    HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID)    {
        _eventCount++;
        std::wstring name;
        if (isMe(pSender)) {
            _bstr_t senderName;
            pSender->get_CurrentName(senderName.GetAddress());
            if (senderName.length() > 0) {
                name = (wchar_t*)senderName;
            }
        }
        /*
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trusted Add-in Catalogs"))); // button
        parse(parm->automation, parent, UICommand(_bstr_t(L"Catalog Url"), share, UICommand::Insert)); // edit field
        parse(parm->automation, parent, UICommand(_bstr_t(L"Add catalog"))); // button
        parse(parm->automation, parent, UICommand(_bstr_t(L"Show in Menu"), UICommand::EnableToggle));
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // knock down any possible error screen 
        parse(parm->automation, parent, UICommand(_bstr_t(L"Show in Menu"), UICommand::EnableToggle)); // make sure its set if there was a knock down
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // OK - but only tthis OK --- pass in somehow
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // 2end OK 
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // 2end OK 
        round 2
        parse(parm->automation, parent, UICommand(_bstr_t(L"No"))); // first dlg box
        parse(parm->automation, parent, UICommand(_bstr_t(L"Ribbon Tabs"), UICommand::Select)); //  NetUIRibbonTab
        parse(parm->automation, parent, UICommand(_bstr_t(L"Insert"), UICommand::Select));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Lower Ribbon"), UICommand::Select)); //  group
        parse(parm->automation, parent, UICommand(_bstr_t(L"Store"), UICommand::Invoke));
        //IUIAutomationElement*parent2 = GetTopLevelWindowByName(L"Office Add-ins");
        parse(parm->automation, parent, UICommand(_bstr_t(L"SHARED FOLDER"), UICommand::Invoke));

        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq"), UICommand::ListItem));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq Companion Beta"), UICommand::ListItem));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq Companion Beta.Use arrow keys to navigate the items
        */
        switch (eventID)        {
        case UIA_MenuOpenedEventId:
            if (isMe(pSender) && type == 0) {
                //http://source.roslyn.io/#Microsoft.VisualStudio.IntegrationTest.Utilities/AutomationElementExtensions.cs,1a83d951b02044f1,references
                //http://source.roslyn.io/#Microsoft.VisualStudio.IntegrationTest.Utilities/ScreenshotService.cs
                if (name == L"File") {
                    invoke(find(pAutomation, pSender, _variant_t(L"Options"), L"NetUIOutSpaceButton"));
                }
            }
            break;
        case UIA_MenuModeStartEventId:
            wprintf(L">> Event UIA_MenuModeStartEventId Received! (count: %d)\n", _eventCount);
            break;
        case UIA_ToolTipOpenedEventId:
            if (isMe(pSender)) {
                int i = 0;
            }
            break;
        case UIA_Window_WindowOpenedEventId:
            if (isMe(pSender)) {
                if (name.find(L"Office Add-ins") != std::string::npos) {
                    CheckName = L"Office Add-ins";
                    invoke(find(pAutomation, pSender, _variant_t(L"SHARED FOLDER"), L""));
                    tryall(pAutomation, pSender, _variant_t(L"Contiq Companion Beta"));
                    tryall(pAutomation, pSender, _variant_t(L"Contiq"));
                    select(find(pAutomation, pSender, _variant_t(L"Contiq Companion Beta"), L""));
                    while (1) {
                        Sleep(500UL);
                        if (CheckName.length() > 0) {
                            CComPtr<IUIAutomationElement> found = find(pAutomation, pSender, _variant_t(CheckName.c_str()), L"");
                            if (!found) {
                                CheckName.clear();
                                select(find(pAutomation, pRoot, _variant_t(L"Ribbon Tabs"), L""));
                                select(find(pAutomation, pRoot, _variant_t(L"Home"), L""));
                                select(find(pAutomation, pRoot, _variant_t(L"Contiq"), L""));
                                break;
                            }
                        }
                    }

                }
                if (name.find(L" - PowerPoint") != std::string::npos) {
                    invoke(find(pAutomation, pSender, _variant_t(L"No"), L"Button")); // clear dlg (are there more)? bugbug
                    if (type == 0) {
                        invoke(find(pAutomation, pSender, _variant_t(L"File Tab"), L"NetUIRibbonTab"));
                    }
                    else if (type == 1) {
                        select(find(pAutomation, pSender, _variant_t(L"Ribbon Tabs"), L"NetUIPanViewer"));
                        Sleep(500UL);
                        select(find(pAutomation, pSender, _variant_t(L"Insert"), L"NetUIRibbonTab"));
                        Sleep(500UL);
                        select(find(pAutomation, pSender, _variant_t(L"Lower Ribbon"), L""));
                        Sleep(500UL);
                        invoke(find(pAutomation, pSender, _variant_t(L"Store"), L""));
                    }
                }
                else if (name == L"Manage Add-in Catalogs" && type == 0) {
                    invoke(find(pAutomation, pSender, _variant_t(L"OK"), L""));
                }
                else if (name == L"Get Started With Contiq!" && type == 1) {
                    invoke(find(pAutomation, pSender, _variant_t(L"OK"), L""));
                }
                else if (name == L"Trust Center" && type == 0) {
                    if (invoke(find(pAutomation, pSender, _variant_t(L"Trusted Add-in Catalogs"), L"NetUIListViewItem"))) {
                        Sleep(500UL);
                    }
                    if (insert(find(pAutomation, pSender, _variant_t(L"Catalog Url"), L"NetUITextbox"), getShareName())) {
                        Sleep(500UL);
                        if (invoke(find(pAutomation, pSender, _variant_t(L"Add catalog"), L"NetUIButton"))) {
                            Sleep(500UL);
                            if (set(find(pAutomation, pSender, _variant_t(L"Show in Menu"), L"NetUICheckbox"))) {
                            }
                            Sleep(500UL);
                            invoke(find(pAutomation, pSender, _variant_t(L"OK"), L"NetUIButton"));
                            Sleep(500UL);
                            invoke(find(pAutomation, pSender, _variant_t(L"OK"), L"NetUIButton"));
                            Sleep(500UL);
                            invoke(find(pAutomation, pSender, _variant_t(L"OK"), L"NetUIButton"));
                            if (presentation.vt != VT_EMPTY) {
                                AutoWrap(DISPATCH_METHOD, NULL, presentation, (LPOLESTR)L"Close", 0);
                            }
                            pid = 0UL;
                            if (app) {
                                CComPtr<IUIAutomationElement> found = find(pAutomation, pRoot, _variant_t(L"PowerPoint"), L"PPTFrameClass");
                                if (found) {
                                    invoke(find(pAutomation, found, _variant_t(L"Close"), L""));
                                }
                                //AutoWrap(DISPATCH_METHOD, NULL, app, (LPOLESTR)L"Quit", 0);
                                wprintf(L"-Removing Event Handlers.\n");
                                // Remove event handlers etc
                                if (pAutomation) {
                                    pAutomation->RemoveAllEventHandlers();
                                }
                                app->Release();
                                SetEvent(event); // stop thread
                            }
                        }
                    }
                }
                else if (name == L"PowerPoint Options" && type == 0) {
                    if (select(find(pAutomation, pSender, _variant_t(L"Trust Center"), L"NetUIListViewItem"))) {
                        Sleep(500UL);
                    }
                    if (invoke(find(pAutomation, pSender, _variant_t(L"Trust Center Settings..."), L"NetUIButton"))) {
                        Sleep(500UL);
                    }
                }
                wprintf(L"ME!>> Event UIA_Window_WindowOpenedEventId Received! (count: %d)\n", _eventCount);
            }
            wprintf(L">> Event WindowOpened Received! (count: %d)\n", _eventCount);
            break;
        case UIA_NotificationEventId:
            break;
        case UIA_SystemAlertEventId:
            break;
        case UIA_Window_WindowClosedEventId:
            if (isMe(pSender)) {
                wprintf(L"ME!>> Event WindowClosed Received! (count: %d)\n", _eventCount);
            }
            wprintf(L">> Event WindowClosed Received! (count: %d)\n", _eventCount);
            break;
        default:
            wprintf(L">> Event (%d) Received! (count: %d)\n", eventID, _eventCount);
            break;
        }
        return S_OK;
    }
}; 
void createSlide(EventHandler*pEvents) {
    if (!pEvents) {
        return;
    }
    // Create a new Presentation. (i.e. Application.Presentations.Add) 
    _variant_t result;
    _variant_t pres;
    HRESULT hr;
    hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pEvents->app, (LPOLESTR)L"Presentations", 0);
    if (result.vt != VT_EMPTY) {
        pres = result.pdispVal;
    }
    else {
        wprintf(L"Presentations w/err 0x%08lx\n", hr);
        return;
    }
    // Call Presentations.Add to create a new presentation
    if (pres.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_METHOD, result.GetAddress(), pres, (LPOLESTR)L"Add", 0);
        if (result.vt != VT_EMPTY) {
            pEvents->presentation = result.pdispVal;
            _putws(L"A new presentation is created");
        }
    }
    else {
        wprintf(L"Add Presentations w/err 0x%08lx\n", hr);
        return;
    }

    /////////////////////////////////////////////////////////////////////////
    // Insert a new Slide and add some text to it. 
    //  

    _variant_t slides;
    if (pEvents->presentation.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pEvents->presentation, (LPOLESTR)L"Slides", 0);
        if (result.vt != VT_EMPTY) {
            slides = result.pdispVal;
        }
    }
    else {
        wprintf(L"Slides w/err 0x%08lx\n", hr);
        return;
    }
    _variant_t slide;
    _variant_t vtIndex(1);
    _variant_t vtLayout(2);
    if (slides.vt != VT_EMPTY) {
        _putws(L"Insert a slide");

        result.Clear();
        // If there are more than 1 parameters passed, they MUST be pass in  
        // reversed order. Otherwise, you may get the error 0x80020009. 
        AutoWrap(DISPATCH_METHOD, result.GetAddress(), slides, (LPOLESTR)L"Add", 2, vtLayout, vtIndex);
        if (result.vt != VT_EMPTY) {
            slide = result.pdispVal;
        }
    }
    else {
        wprintf(L"Add Slides w/err 0x%08lx\n", hr);
        return;
    }

    // Add some texts to the slide 
    _putws(L"Add some texts");

    _variant_t shapes;
    if (slide.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), slide, (LPOLESTR)L"Shapes", 0);
        if (result.vt != VT_EMPTY) {
            shapes = result.pdispVal;
        }
    }
    else {
        wprintf(L"Shapes w/err 0x%08lx\n", hr);
        return;
    }

    _variant_t shape;
    if (shapes.vt != VT_EMPTY) {
        vtIndex.Clear();
        vtIndex = 1;
        AutoWrap(DISPATCH_METHOD, result.GetAddress(), shapes, (LPOLESTR)L"Item", 1, vtIndex);
        if (result.vt != VT_EMPTY) {
            shape = result.pdispVal;
        }
    }
    else {
        wprintf(L"Item w/err 0x%08lx\n", hr);
        return;
    }

    _variant_t frame;
    if (shape.vt != VT_EMPTY) {
        result.Clear();
        HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), shape, (LPOLESTR)L"TextFrame", 0);
        if (result.vt != VT_EMPTY) {
            frame = result.pdispVal;
        }
    }
    else {
        wprintf(L"TextFrame w/err 0x%08lx\n", hr);
        return;
    }

    _variant_t range;
    if (frame.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), frame, (LPOLESTR)L"TextRange", 0);
        if (result.vt != VT_EMPTY) {
            range = result.pdispVal;
        }
    }
    else {
        wprintf(L"TextRange w/err 0x%08lx\n", hr);
        return;
    }

    if (range.vt != VT_EMPTY) {
        _variant_t welcome(L"Welcome To Contiq");
        AutoWrap(DISPATCH_PROPERTYPUT, NULL, range, (LPOLESTR)L"Text", 1, welcome);
    }
}

int startPPT(EventHandler*pEvents, IUIAutomation* pAutomation) {
    if (!pEvents || !pAutomation) {
        return -1;
    }
    CLSID clsid;

    // Get CLSID from ProgID using CLSIDFromProgID. 
    LPCOLESTR progID = L"PowerPoint.Application";
    HRESULT hr = CLSIDFromProgID(progID, &clsid);
    if (FAILED(hr)) {
        wprintf(L"CLSIDFromProgID(\"%s\") failed w/err 0x%08lx\n", progID, hr);
        return 0;
    }
    // Start the server and get the IDispatch interface 
    hr = CoCreateInstance(        // [-or-] CoCreateInstanceEx, CoGetObject 
        clsid,                    // CLSID of the server 
        NULL,CLSCTX_LOCAL_SERVER,    // PowerPoint.Application is a local server 
        IID_IDispatch,  (void **)&pEvents->app);        // Output 
    if (FAILED(hr)) {
        wprintf(L"PowerPoint is not registered properly w/err 0x%08lx\n", hr);
        return 0;
    }
    _putws(L"PowerPoint.Application is started");
    pEvents->pid = FindProcessId(L"POWERPNT.EXE");
    //SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
   // PostMessage(HWND_BROADCAST, WM_WININICHANGE, SPI_SETSCREENREADER, 0);

    _variant_t result;
    AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pEvents->app, (LPOLESTR)L"OperatingSystem", 0);
    if (result.vt != VT_EMPTY) {
        _putws(result.bstrVal);
    }
    else {
        wprintf(L"OperatingSystem w/err 0x%08lx\n", hr);
        return 0;
    }

    // Get the build todo enforce versions here or at the file version level?
    result.Clear();
    AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pEvents->app, (LPOLESTR)L"Build", 0);
    if (result.vt != VT_EMPTY) {
        _putws(result.bstrVal);
    }
    else {
        wprintf(L"Build w/err 0x%08lx\n", hr);
        return 0;
    }

    createSlide(pEvents);

    return 1;
}


std::wstring RegistryGetString(HKEY    key, PCWSTR  subKey, PCWSTR  valueName) {
    //
    // Try getting the size of the string value to read,
    // to properly allocate a buffer for the string
    //
    DWORD keyType = 0;
    DWORD dataSize = 0;
    const DWORD flags = RRF_RT_REG_SZ; // Only read strings (REG_SZ)
    LONG result = ::RegGetValueW(key, subKey, valueName, flags, &keyType, nullptr, &dataSize);
    if (result != ERROR_SUCCESS) {
        //ERROR_FILE_NOT_FOUND ERROR_UNSUPPORTED_TYPE
        return L"";
    }
    WCHAR* data = new WCHAR[dataSize / sizeof(WCHAR) + sizeof(WCHAR)];
    memset(data, 0, sizeof(data));
    result = ::RegGetValueW(key, subKey, valueName, flags, nullptr, data, &dataSize);
    if (result != ERROR_SUCCESS) {
        return L"";
    }
    std::wstring text = data;
    delete[] data;
    return text;
}

std::wstring GetInstallPath() {
    std::wstring location = RegistryGetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\Software\\Wow6432Node\\Microsoft\\Office\\16.0\\Common\\InstallRoot", L"Path");
    if (location.size() > 0) {
        return location;
    }
    return RegistryGetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\Software\\Wow6432Node\\Microsoft\\Office\\15.0\\Common\\InstallRoot", L"Path");
}

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64() {
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (NULL != fnIsWow64Process) {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64)) {
            // Handle error...
        }
    }
    return bIsWow64;
}

void fileVersion(const std::wstring& filePath, uint16_t& major, uint16_t& minor, uint16_t& revision, uint16_t& build) {
    DWORD  verHandle = 0;
    UINT   size = 0;
    LPBYTE lpBuffer = NULL;
    DWORD  verSize = GetFileVersionInfoSize(filePath.c_str(), &verHandle);

    if (verSize != NULL) {
        WCHAR* verData = new WCHAR[verSize];
        if (GetFileVersionInfoW(filePath.c_str(), verHandle, verSize, verData)) {
            if (VerQueryValueW(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size)) {
                if (size) {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd) {
                        // Doesn't matter if you are on 32 bit or 64 bit,
                        // DWORD is always 32 bits, so first two revision numbers
                        // come from dwFileVersionMS, last two come from dwFileVersionLS
                        if (IsWow64()) {
                            // 64 bit build
                            major = (verInfo->dwProductVersionMS >> 16) & 0xffff;
                            minor = (verInfo->dwProductVersionMS >> 0) & 0xffff;
                            revision = (verInfo->dwProductVersionLS >> 16) & 0xffff;
                            build = (verInfo->dwProductVersionLS >> 0) & 0xffff;
                        }
                        else {
                            // 32 bit build
                            major = HIWORD(verInfo->dwProductVersionMS);
                            minor = LOWORD(verInfo->dwProductVersionMS);
                            revision = HIWORD(verInfo->dwProductVersionLS);
                            build = LOWORD(verInfo->dwProductVersionLS);
                        }
                    }
                }
            }
        }
        delete[] verData;
    }

}
IUIAutomationElement* GetTopLevelWindowByName(IUIAutomation *automation, const std::wstring& windowName) {
    IUIAutomationElement* pRoot = nullptr;
    IUIAutomationElement* pFound = nullptr;

    // Get the desktop element
    HRESULT hr = automation->GetRootElement(&pRoot);
    // Get a top-level element by name, such as "Program Manager"
    if (pRoot) {
        IUIAutomationCondition* pCondition;
        VARIANT varProp;
        VariantInit(&varProp);
        varProp.vt = VT_BSTR;
        varProp.bstrVal = SysAllocString(windowName.c_str());
        automation->CreatePropertyCondition(UIA_NamePropertyId, varProp, &pCondition);
        pRoot->FindFirst(TreeScope_Children, pCondition, &pFound);
       // HRESULT h = pFound->get_CurrentNativeWindowHandle(&wh);
        //if (wh) {
           // SendMessage((HWND)wh, WM_KEYDOWN, VK_TAB, 0);
        //}
        pRoot->Release();
        pCondition->Release();
    }
    return pFound;
}
void parse(IUIAutomation *automation, IUIAutomationElement* pRoot, const UICommand cmd) {
    if (!pRoot) {
        return;
    }
    IUIAutomationElement* pFound;//bugbug we do not free this
    IUIAutomationElementArray* all;//bugbug we do not free this
    Sleep(cmd.sleep);
    // Get the desktop element
    //HRESULT hr = automation->GetRootElement(&pRoot);

    // Get a top-level element by name, such as "Program Manager"
    if (pRoot) {
        IUIAutomationCondition * pCondition;
        automation->CreateTrueCondition(&pCondition);
        //IUIAutomationElementArray* all;
        pRoot->FindAll(TreeScope_Subtree, pCondition, &all);
        pCondition->Release();
        if (all) {
            int len;
            all->get_Length(&len);
            for (int i = 0; i < len; ++i) {
                all->GetElement(i, &pFound);
                BOOL b;
                _bstr_t cls;
                _bstr_t name;
                _bstr_t id; // just call the seach each time, super easy parser for install at least
                echoElement(pFound, cmd.target, b, cls, name, id);
                _bstr_t test(L"Show In Menu");
                _bstr_t test2(L"NetUIHWND");
                // test code
                if (name == cmd.target && cmd.cmd == UICommand::Parse) {
                }
                // helpful https://docs.microsoft.com/en-gb/windows/desktop/WinAuto/uiauto-controlpatternsoverview
            //https://github.com/Microsoft/Windows-classic-samples/blob/master/Samples/UIAutomationDocumentClient/cpp/UiaDocumentClient.cpp
                size_t index = -1;
                if (name.length() > 0) {
                    std::wstring s = (wchar_t*)name;
                    index = s.find(L"Contiq");
                }

                if (index >= 0 && cmd.cmd == UICommand::ListItem) {
                    //UIA_TreeItemControlTypeId
                    CComPtr<IUIAutomationValuePattern> file_name_value_pattern;
                    HRESULT hr = pFound->GetCurrentPatternAs(UIA_ValuePatternId, IID_PPV_ARGS(&file_name_value_pattern));
                    if (FAILED(hr)) {
                        //LOGHR(WARN, hr) << "Failed to get value pattern for file name edit box on current dialog, trying next dialog";
                        //turn false;
                    }

                    CComPtr<IUIAutomationLegacyIAccessiblePattern> aha;
                    hr = pFound->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, IID_PPV_ARGS(&aha));
                    if (aha) {
                        aha->Select(1);
                    }

                    IUIAutomationAnnotationPattern* _annotation;
                    hr = pFound->GetCurrentPatternAs(UIA_AnnotationPatternId, IID_PPV_ARGS(&_annotation));
                    if (FAILED(hr)){
                        wprintf(L"Failed to Get Annotation Pattern, HR: 0x%08x\n", hr);
                    }

                    IUIAutomationElement *g;
                    HRESULT h = pFound->GetCurrentPatternAs(UIA_GridItemPatternId, __uuidof(IUIAutomationElement), (void **)&g);
                    if (g) {
                        int k = 0;
                    }
                    UIA_HWND wh;
                    h = pFound->get_CurrentNativeWindowHandle(&wh);
                    if (wh) {
                        SendMessage((HWND)wh, WM_KEYDOWN, VK_TAB, 0);
                    }
                    //IUIAutomationSelectionItemPattern
                   // Microsoft::WRL::ComPtr<IUIAutomationTreeWalker> tree_walker;
                    IUIAutomationElement *img;
                    h = pFound->GetCurrentPatternAs(UIA_ImageControlTypeId, IID_PPV_ARGS(&img));
                    //h = pFound->GetCurrentPatternAs(UIA_ImageControlTypeId, __uuidof(IUIAutomationElement), (void **)&img);
                    if (img) {
                        int k = 0;
                    }

                    // pFound->SetFocus();
                    // UIA_HWND wh;
                     //pFound->get_CurrentNativeWindowHandle(&wh);
                    IUIAutomationSelectionItemPattern*itm;
                    h = pFound->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void **)&itm);
                    if (itm) {
                        itm->Select();
                        return;
                    }

                    IUIAutomationElement *list;
                    h = pFound->GetCurrentPatternAs(UIA_ListItemControlTypeId, __uuidof(IUIAutomationElement), (void **)&list);

                    IUIAutomationTextEditPattern *pedit;
                    h = pFound->GetCurrentPatternAs(UIA_TextPatternId, __uuidof(IUIAutomationTextEditPattern), (void **)&pedit);
                    IUIAutomationValuePattern *pval;
                    // IValueProvider
                   // IUIAutomationTextEditPattern *pval;
                   // IValueProvider
                    h = pFound->GetCurrentPatternAs(UIA_ValuePatternId, __uuidof(IUIAutomationValuePattern), (void **)&pval);
                    // pFound->SetFocus();

                    IUIAutomationElement *pe; // UIA_PaneControlTypeId  UIA_GroupControlTypeId
                    h = pFound->GetCurrentPatternAs(UIA_TextControlTypeId, __uuidof(IUIAutomationElement), (void **)&pe);


                    IItemContainerProvider *pcont; // UIA_PaneControlTypeId  UIA_GroupControlTypeId
                    h = pFound->GetCurrentPatternAs(UIA_GroupControlTypeId, __uuidof(IUIAutomationItemContainerPattern), (void **)&pcont);

                    IUIAutomationElementArray *parray;
                    h = pFound->GetCurrentPatternAs(UIA_GroupControlTypeId, __uuidof(IUIAutomationElementArray), (void **)&parray);

                    IUIAutomationTogglePattern * ptoggle; // none of these are free
                    h = pFound->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern), (void **)&ptoggle);
                    if (ptoggle) {
                    }
                    IUIAutomationSelectionItemPattern * pSelect;
                    h = pFound->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void **)&pSelect);
                    if (pSelect) {
                        pSelect->Select();
                    }
                    IUIAutomationExpandCollapsePattern * pUniverse;
                    h = pFound->GetCurrentPatternAs(UIA_ExpandCollapsePatternId, __uuidof(IUIAutomationExpandCollapsePattern), (void **)&pUniverse);
                    if (pUniverse) {
                        pUniverse->Expand();
                    }
                    IUIAutomationInvokePattern * pInvoke;
                    h = pFound->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void **)&pInvoke);
                    if (pInvoke) {
                        int i = 0;
                    }
                }
                //https://docs.microsoft.com/en-us/windows/desktop/winauto/uiauto-implementingselectionitem
                _bstr_t outspace(L"NetUIOutSpaceButton");

                if (name == cmd.target  && cmd.cmd == UICommand::ExpandCollapse) {
                    IUIAutomationExpandCollapsePattern * pUniverse;
                    pFound->GetCurrentPatternAs(UIA_ExpandCollapsePatternId, __uuidof(IUIAutomationExpandCollapsePattern), (void **)&pUniverse);
                    if (pUniverse) {
                        pUniverse->Expand();
                        pUniverse->Release();
                        return;
                    }
                }
                if (name == cmd.target && cmd.cmd == UICommand::Invoke) { // do we need id == fileID ?
                    IUIAutomationInvokePattern * pInvoke;
                    pFound->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void **)&pInvoke);
                    if (pInvoke) {
                        pInvoke->Invoke();
                        pInvoke->Release();
                        return;
                    }
                }
                if (name == cmd.target && cmd.cmd == UICommand::Insert) {
                    BOOL enabled;
                    pFound->get_CurrentIsEnabled(&enabled);
                    if (!enabled) {
                        // set error
                        int i = 0;
                    }
                    pFound->get_CurrentIsKeyboardFocusable(&enabled);
                    if (enabled) {
                        IUIAutomationValuePattern *pval;
                        // IValueProvider
                       // IUIAutomationTextEditPattern *pval;
                       // IValueProvider
                        pFound->GetCurrentPatternAs(UIA_ValuePatternId, __uuidof(IUIAutomationValuePattern), (void **)&pval);
                        pFound->SetFocus();
                        if (pval) {
                            _bstr_t val(cmd.data.c_str());
                            pval->SetValue(val.GetBSTR());
                            pval->Release();
                            return;
                        }
                    }
                }
                //C:\Program Files (x86)\Windows Kits\10\bin\10.0.17134.0\x64>inspect
                if (name == cmd.target && cmd.cmd == UICommand::Select) {
                    IUIAutomationSelectionItemPattern * pSelect;
                    pFound->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void **)&pSelect);
                    if (pSelect) {
                        pSelect->Select();
                        pSelect->Release();
                        return;
                    }
                }

                if (name == cmd.target && cmd.cmd == UICommand::EnableToggle) {
                    // IToggleProvider UIA_TextPatternId
                    IUIAutomationTogglePattern * pToggle;
                    pFound->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern), (void **)&pToggle);
                    if (pToggle) {
                        ToggleState state;
                        pToggle->get_CurrentToggleState(&state);
                        if (state != ToggleState_On) {
                            pToggle->Toggle(); // 
                        }
                        pToggle->Release();
                        return;
                    }
                }
            }
        }
    }

}
void findCheckBox(IUIAutomation *automation, IUIAutomationElement * pElement) {
    IUIAutomationCondition * pCheckBoxProp;
    VARIANT varCheckBox;
    varCheckBox.vt = VT_I4;
    varCheckBox.lVal = UIA_CheckBoxControlTypeId;
    automation->CreatePropertyCondition(UIA_ControlTypePropertyId, varCheckBox, &pCheckBoxProp);

    IUIAutomationElement * pFound;
    pElement->FindFirst(TreeScope_Descendants, pCheckBoxProp, &pFound);

}

void createShare(LPWSTR name) {
    // need to run as admin, c++ does not.  Batch file may? https://stackoverflow.com/questions/4419868/what-is-the-current-directory-in-a-batch-file
    //system("net share h=C:\\of_v0.10.0_vs2017_release\\apps\\myApps\\myContiq\\bin\\data");
    SHARE_INFO_2 p;
    memset(&p, 0, sizeof(p));
    DWORD parm_err = 0;
    // be sure to delete when done
    p.shi2_netname = name; // make a guid name
    p.shi2_type = STYPE_DISKTREE;
    p.shi2_remark = (LPWSTR)L"Contiq Install Share";
    p.shi2_max_uses = 4;
    p.shi2_current_uses = 0;
    p.shi2_path = (LPWSTR)L"C:\\install";
    p.shi2_passwd = NULL; // no password
    p.shi2_permissions = ACCESS_ALL;;// ACCESS_READ;
    NET_API_STATUS status = NetShareAdd(NULL, 2, (LPBYTE)&p, &parm_err);
    if (!status) {
        std::wcout << "created: " << p.shi2_netname;
     }
     else {
             std::wcout << "failed to create: " << p.shi2_netname << " error " << status;
    }
}


struct Parm {
    //myEvents events;
    int type;
    IUIAutomation *automation;
};

DWORD WINAPI AutomatePowerPointByCOMAPI(LPVOID lpParam) {
    struct Parm* parm = reinterpret_cast<struct Parm*>(lpParam);

    //parm* parms = (parm*)lpParam;
    // Initializes the COM library on the current thread and identifies  
    // the concurrency model as single-thread apartment (STA).  
    // [-or-] CoInitialize(NULL  


    // [-or-] CoCreateInstance(NULL); 
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    ///////////////////////////////////////////////////////////////////////// 
    // Create the PowerPoint.Application COM object using C++ and COM APIs. 
    //  

    // Get CLSID of the server 

    CLSID clsid;
    HRESULT hr;


    // Option 1. Get CLSID from ProgID using CLSIDFromProgID. 
    LPCOLESTR progID = L"PowerPoint.Application";
    hr = CLSIDFromProgID(progID, &clsid);
    if (FAILED(hr)) {
        wprintf(L"CLSIDFromProgID(\"%s\") failed w/err 0x%08lx\n", progID, hr);
        return 1;
    }
    // Option 2. Build the CLSID directly. 
    /*const IID CLSID_Application =
    {0x91493441,0x5A91,0x11CF,{0x87,0x00,0x00,0xAA,0x00,0x60,0x26,0x3B}};
    clsid = CLSID_Application;*/

    // Start the server and get the IDispatch interface 
    IDispatch *pPpApp = NULL;
    hr = CoCreateInstance(        // [-or-] CoCreateInstanceEx, CoGetObject 
        clsid,                    // CLSID of the server 
        NULL,
        CLSCTX_LOCAL_SERVER,    // PowerPoint.Application is a local server 
        IID_IDispatch,            // Query the IDispatch interface 
        (void **)&pPpApp);        // Output 

    if (FAILED(hr)) {
        wprintf(L"PowerPoint is not registered properly w/err 0x%08lx\n", hr);
        return 1;
    }
    _variant_t app(pPpApp, FALSE);
    _putws(L"PowerPoint.Application is started");

    ///////////////////////////////////////////////////////////////////////// 
    // Make PowerPoint invisible. (i.e. Application.Visible = 0) 
    //  

    // By default PowerPoint is invisible, till you make it visible: 
    //{ 
    //    VARIANT x; 
    //    x.vt = VT_I4; 
    //    x.lVal = 0;    // Office::MsoTriState::msoFalse 
    //    hr = AutoWrap(DISPATCH_PROPERTYPUT, NULL, pPpApp, L"Visible", 1, x); 
    //_variant_t x();
   // AutoWrap(DISPATCH_PROPERTYPUT, NULL, app, L"Visible", 1, x);
    // } 

    _variant_t result;

    //if (lpParam != (LPVOID)1) {
        // Get the OS collection 
    hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), app, (LPOLESTR)L"OperatingSystem", 0);
    if (result.vt != VT_EMPTY && !IS_ERROR(hr)) {
        _putws(result.bstrVal);
    }

    // Get the build todo enforce versions here or at the file version level?
    result.Clear();
    hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), app, (LPOLESTR)L"Build", 0);
    if (result.vt != VT_EMPTY) {
        _putws(result.bstrVal);
    }
    // }

     // Create a new Presentation. (i.e. Application.Presentations.Add) 
    _variant_t pres;
    result.Clear();
    hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), app, (LPOLESTR)L"Presentations", 0);
    if (result.vt != VT_EMPTY) {
        pres = result.pdispVal;
    }


    // Call Presentations.Add to create a new presentation
    _variant_t addins;
    result.Clear();
    AutoWrap(DISPATCH_METHOD, result.GetAddress(), app, (LPOLESTR)L"AddIns", 0);//DISP_E_UNKNOWNNAME
    if (result.vt != VT_EMPTY) {
        addins = result.pdispVal;
        _putws(L"A new presentation is created");
    }

    // Call Presentations.Add to create a new presentation
    _variant_t pre;
    if (pres.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_METHOD, result.GetAddress(), pres, (LPOLESTR)L"Add", 0);
        if (result.vt != VT_EMPTY) {
            pre = result.pdispVal;
            _putws(L"A new presentation is created");
        }

    }

    /////////////////////////////////////////////////////////////////////////
    // Insert a new Slide and add some text to it. 
    //  

    // Get the Slides collection 
    _variant_t slides;
    if (pre.vt != VT_EMPTY) {
        result.Clear();
        AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pre, (LPOLESTR)L"Slides", 0);
        if (result.vt != VT_EMPTY) {
            slides = result.pdispVal;
        }
    }

    // Insert a new slide 
    _putws(L"Insert a slide");

    _variant_t slide;
    _variant_t vtIndex(1);
    _variant_t vtLayout(2);
    result.Clear();
    // If there are more than 1 parameters passed, they MUST be pass in  
    // reversed order. Otherwise, you may get the error 0x80020009. 
    AutoWrap(DISPATCH_METHOD, result.GetAddress(), slides, (LPOLESTR)L"Add", 2, vtLayout, vtIndex);
    if (result.vt != VT_EMPTY) {
        slide = result.pdispVal;
    }

    // Add some texts to the slide 
    _putws(L"Add some texts");

    _variant_t shapes;
    result.Clear();
    AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), slide, (LPOLESTR)L"Shapes", 0);
    if (result.vt != VT_EMPTY) {
        shapes = result.pdispVal;
    }

    _variant_t shape;
    vtIndex.Clear();
    vtIndex = 1;
    AutoWrap(DISPATCH_METHOD, result.GetAddress(), shapes, (LPOLESTR)L"Item", 1, vtIndex);
    if (result.vt != VT_EMPTY) {
        shape = result.pdispVal;
    }

    _variant_t frame;
    result.Clear();
    hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), shape, (LPOLESTR)L"TextFrame", 0);
    if (result.vt != VT_EMPTY) {
        frame = result.pdispVal;
    }

    _variant_t range;
    result.Clear();
    AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), frame, (LPOLESTR)L"TextRange", 0);
    if (result.vt != VT_EMPTY) {
        range = result.pdispVal;
    }

    _variant_t welcome(L"Welcome To Contiq");
    AutoWrap(DISPATCH_PROPERTYPUT, NULL, range, (LPOLESTR)L"Text", 1, welcome);
    std::wstring cap;
    _bstr_t caption;
    result.Clear();
    AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), app, (LPOLESTR)L"Caption", 0);
    if (result.vt != VT_EMPTY) {
        caption = result.bstrVal;
        cap = (wchar_t*)caption;
        cap += L" - PowerPoint";
        _putws(result.bstrVal);
    }



    SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    PostMessage(HWND_BROADCAST, WM_WININICHANGE, SPI_SETSCREENREADER, 0);
    // assumes ppt running make sure this is the case and only our instance is running

    IUIAutomationElement*parent = GetTopLevelWindowByName(parm->automation,  cap);// (L"PowerPoint"
    if (parm->type == 1) {
        createShare((LPWSTR)"Contiq");
        parse(parm->automation, parent, UICommand(_bstr_t("No"))); // first dlg box
        parse(parm->automation, parent, UICommand(_bstr_t(L"File Tab"))); // File
        parse(parm->automation, parent, UICommand(_bstr_t(L"Options"))); // Options
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trust Center"), UICommand::Select));//Trust Center
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trust Center Settings..."))); // button
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trust access to the VBA project object model"), UICommand::EnableToggle)); // Trust access to the VBA project object model
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // knock down any error screen 
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trust Center Settings..."))); // button
        parse(parm->automation, parent, UICommand(_bstr_t(L"Trusted Add-in Catalogs"))); // button
        ///parse(parm->automation, parent, UICommand(_bstr_t(L"Catalog Url"), share, UICommand::Insert)); // edit field
        parse(parm->automation, parent, UICommand(_bstr_t(L"Add catalog"))); // button
        parse(parm->automation, parent, UICommand(_bstr_t(L"Show in Menu"), UICommand::EnableToggle));
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // knock down any possible error screen 
        parse(parm->automation, parent, UICommand(_bstr_t(L"Show in Menu"), UICommand::EnableToggle)); // make sure its set if there was a knock down
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // OK - but only tthis OK --- pass in somehow
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // 2end OK 
        parse(parm->automation, parent, UICommand(_bstr_t(L"OK"))); // 2end OK 
        //DWORD e = NetShareDel(NULL, (LPSTR)"Contiq", 0L);
        //if (e == NERR_NetNameNotFound) {

//        }
    }
    else {
        parse(parm->automation, parent, UICommand(_bstr_t(L"No"))); // first dlg box
        parse(parm->automation, parent, UICommand(_bstr_t(L"Ribbon Tabs"), UICommand::Select)); //  NetUIRibbonTab
        parse(parm->automation, parent, UICommand(_bstr_t(L"Insert"), UICommand::Select));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Lower Ribbon"), UICommand::Select)); //  group
        parse(parm->automation, parent, UICommand(_bstr_t(L"Store"), UICommand::Invoke));
        //IUIAutomationElement*parent2 = GetTopLevelWindowByName(L"Office Add-ins");
        parse(parm->automation, parent, UICommand(_bstr_t(L"SHARED FOLDER"), UICommand::Invoke));

        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq"), UICommand::ListItem));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq Companion Beta"), UICommand::ListItem));
        parse(parm->automation, parent, UICommand(_bstr_t(L"Contiq Companion Beta.Use arrow keys to navigate the items, press enter to use the selected add - in."), UICommand::ListItem));
        //parse(parent, UICommand(_bstr_t(L"Add"), UICommand::Invoke));
    }
    //1) Go to Insert > Click on "My Add-ins" > Go to "SHARED FOLDER" tab > Select Contiq plugin and click Add.

    ///////////////////////////////////////////////////////////////////////// 
    // Save the presentation as a pptx file and close it. 
    //  
    /*
    _putws(L"Save and close the presentation");

    {
        // Make the file name

        // Get the directory of the current exe.
        wchar_t szFileName[MAX_PATH];
        if (!GetModuleDirectory(szFileName, ARRAYSIZE(szFileName)))
        {
            _putws(L"GetModuleDirectory failed");
            return 1;
        }

        // Concat "Sample2.pptx" to the directory
        wcsncat_s(szFileName, ARRAYSIZE(szFileName), L"Sample2.pptx", 12);

        VARIANT vtFileName;
        vtFileName.vt = VT_BSTR;
        vtFileName.bstrVal = SysAllocString(szFileName);

        VARIANT vtFormat;
        vtFormat.vt = VT_I4;
        vtFormat.lVal = 24;    // PpSaveAsFileType::ppSaveAsOpenXMLPresentation

        VARIANT vtEmbedFont;
        vtEmbedFont.vt = VT_I4;
        vtEmbedFont.lVal = -2;    // MsoTriState::msoTriStateMixed

        // If there are more than 1 parameters passed, they MUST be pass in
        // reversed order. Otherwise, you may get the error 0x80020009.
        AutoWrap(DISPATCH_METHOD, NULL, pPre, L"SaveAs", 3, vtEmbedFont,
            vtFormat, vtFileName);

        VariantClear(&vtFileName);
    }
    */

    // pPre->Close() 
    AutoWrap(DISPATCH_METHOD, NULL, pre, (LPOLESTR)L"Close", 0);


    ///////////////////////////////////////////////////////////////////////// 
    // Quit the PowerPoint application. (i.e. Application.Quit()) 
    //  
    _putws(L"Quit the PowerPoint application");
    AutoWrap(DISPATCH_METHOD, NULL, app, (LPOLESTR)L"Quit", 0);
    

    // Uninitialize COM for this thread 
    CoUninitialize();

    return 0L;
}
struct EventData {
    HANDLE  handles[2];
    int type;
};
DWORD WINAPI events(LPVOID lpParam){
    HRESULT hr;
    int ret = 0;
    CComPtr<IUIAutomationElement> pRoot;
    EventHandler* pEvents = NULL;
    EventData*pdata = (struct EventData*)lpParam;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    CComPtr<IUIAutomation> pAutomation;
    hr = CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&pAutomation);
    if (FAILED(hr) || pAutomation == NULL)    {
        wprintf(L"CoCreateInstance failed w/err 0x%08lx\n", hr);
        return 0;
    }
    // Use root element for listening to window and tooltip creation and destruction.
    hr = pAutomation->GetRootElement(&pRoot);
    if (FAILED(hr) || pRoot == NULL)    {
        wprintf(L"GetRootElement failed w/err 0x%08lx\n", hr);
        return 0;
    }
    pdata->handles[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    pEvents = new EventHandler(pAutomation, pRoot, pdata->handles[1], pdata->type);
    if (pEvents == NULL) {
        wprintf(L"EventHandler er\n");
        return 0;
    }
    wprintf(L"-Adding Event Handlers.\n");
    hr = pAutomation->AddAutomationEventHandler(UIA_MenuOpenedEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_Window_WindowOpenedEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_Window_WindowClosedEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_NotificationEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_SystemAlertEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_SystemAlertEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);
    hr = pAutomation->AddAutomationEventHandler(UIA_ToolTipOpenedEventId, pRoot, TreeScope_Subtree, NULL, (IUIAutomationEventHandler*)pEvents);

    //HandleStructureChangedEvent
    // let others know we are ready then block for ever or someone frees the sem
    startPPT(pEvents, pAutomation);
    SetEvent(pdata->handles[0]);
    DWORD dw = WaitForSingleObject(pdata->handles[1], INFINITE);
    //getchar(); // put in a  real block

    if (pEvents != NULL)
        pEvents->Release();

    CoUninitialize();
    return 0;
}

void fireItup(int type) {
    HANDLE  hThread;
    DWORD id;
    struct EventData data;
    memset(&data, 0, sizeof(struct EventData));
    data.type = type;
    data.handles[0] = CreateEvent(NULL, TRUE, FALSE, NULL); 
    hThread = CreateThread(NULL, 0, events, (LPVOID)&data, 0, &id);
    DWORD dw = WaitForSingleObject(data.handles[0], INFINITE);
    CloseHandle(data.handles[0]);
    WaitForMultipleObjects(1, &hThread, TRUE, INFINITE);
    if (data.handles[1]) {
        CloseHandle(data.handles[1]);
    }
}
int wmain() {
    std::wcout << L"Hello World!\n"; 

    //fireItup(0);
    fireItup(1);

    LPWSTR *szArglist;
    int nArgs;

    szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    if (NULL == szArglist) {
        wprintf(L"CommandLineToArgvW failed\n");
        return 0;
    }
    else {
        for (int i = 0; i < nArgs; i++)
            printf("%d: %ws\n", i, szArglist[i]);
    }
    LocalFree(szArglist);

    //createShare((LPWSTR)L"Contiq");
    do {
        // debuger holds on to this
        if (0 && FindProcessId(L"POWERPNT.EXE")) {
            postMessage(L"Please shut down all instances of PowerPoint");
        }
        else {
            break;
        }
    } while (true);

    //Windows::Foundation::Initialize();

    //struct Parm data;
    //CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //CoCreateInstance(__uuidof(CUIAutomation), NULL, CLSCTX_INPROC_SERVER, __uuidof(IUIAutomation), (void**)&data.automation);
    //data.events.setup(data.automation);
   // data.type = 0;
    //HANDLE hThread = CreateThread(NULL, 0, AutomatePowerPointByCOMAPI, &data, 0, NULL);
    //while (data.events.pEHTemp->wait)
     //   Sleep(100L);
//  WaitForSingleObject(hThread, INFINITE);
   // CloseHandle(hThread);
    //hThread = CreateThread(NULL, 0, AutomatePowerPointByCOMAPI, (LPVOID)1, 0, NULL);
    //WaitForSingleObject(hThread, INFINITE);
    //CloseHandle(hThread);
    //data.automation->RemoveAllEventHandlers();
   // data.automation->Release();
    CoUninitialize();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
