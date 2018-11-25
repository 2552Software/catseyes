 // install.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Mincore.lib")
#pragma comment(lib, "comsuppw.lib")

const char*fromWide(const std::wstring&w) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(w).c_str();
}
void postError(const std::wstring& error, DWORD e) {
    MessageBoxW(nullptr, error.c_str(), L"Contiq Error!\0", MB_ICONEXCLAMATION | MB_OK);
}
void postMessage(const std::wstring& message) {
    MessageBoxW(nullptr, message.c_str(), L"Contiq Message\0", MB_ICONINFORMATION | MB_OK);
}
int askQuestion(const std::wstring& question) {
    return MessageBoxW(nullptr, question.c_str(), L"Contiq Question\0", MB_ICONQUESTION | MB_YESNO);
}

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
HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...){
    va_list marker;
    va_start(marker, cArgs);

    if (!pDisp) {
        spdlog::error("NULL IDispatch passed to AutoWrap()");
        return E_INVALIDARG;
    }

    // Variables used 
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;

    // try a few times if server busy
    int i = 60; // try for 1 minute
    do {
        hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
        if (FAILED(hr)) {
            if (hr == RPC_E_SERVERCALL_RETRYLATER) {
                spdlog::error("PowerPoint busy");
                Sleep(1000UL);
                if (--i > 0) {
                    continue;
                }
            }
            else if (hr == DISP_E_UNKNOWNNAME) {
                spdlog::error("unknown name IDispatch::GetIDsOfNames(\"{0}\") failed w/err  {1}\n", fromWide(ptName), hr);
            }
            else {
                spdlog::error("IDispatch::GetIDsOfNames(\"{0}\") failed w/err {1}\n", fromWide(ptName), hr);
            }
            return hr;
        }
        break;
    } while (1);

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
            spdlog::error("IDispatch::Invoke(\"{0}\") failed w/err {2} DISP_E_EXCEPTION\n", fromWide(ptName), hr);
        }
        else {
            spdlog::error("IDispatch::Invoke(\"{0}\") failed w/err{2}\n", fromWide(ptName),  hr);
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

std::wstring convert(const _bstr_t& bs) {
    return static_cast<wchar_t*>(bs);
}
void justEcho(const _bstr_t target, const std::wstring& name, _bstr_t& bs) {
    spdlog::debug("target {0}, {1} = {2}\n", fromWide(static_cast<wchar_t*>(target)), fromWide(name), fromWide(static_cast<wchar_t*>(bs)));
}
std::wstring name(IUIAutomationElement* element) {
    if (element) {
        _bstr_t name;
        element->get_CurrentName(name.GetAddress());
        if (name.length() > 0) {
            return convert(name);
        }
    }
    return L"null name";
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
                spdlog::debug("set {0}", fromWide(name(pNode)));
                pToggle->Toggle(); 
            }
            return true;
        }
        else {
            spdlog::info("cannot set {0}", fromWide(name(pNode)));
        }
    }
    else {
        spdlog::info("set null");
    }
    return false;
}
bool insert(IUIAutomationElement* pNode, const std::wstring& data) {
    if (pNode) {
        BOOL enabled;
        pNode->get_CurrentIsEnabled(&enabled);
        if (!enabled) {
            spdlog::info("insert element not enabled {0}", fromWide(name(pNode)));
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
                spdlog::debug("insert element {0}, val {1}", fromWide(name(pNode)), fromWide(data));
                return true;
            }
            else {
                spdlog::info("cannot insert element {0}, val {1}", fromWide(name(pNode)), fromWide(data));
            }
        }
    }
    else {
        spdlog::info("insert null");
    }
    return false;
}
bool invoke(IUIAutomationElement* pNode) {
    if (pNode) {
        CComPtr<IUIAutomationInvokePattern> pInvoke;
        pNode->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void **)&pInvoke);
        if (pInvoke) {
            pInvoke->Invoke();
            spdlog::debug("invoked {0}", fromWide(name(pNode)));
            return true;
        }
        else {
            spdlog::info("invoked failed {0}", fromWide(name(pNode)));
        }
    }
    else {
        spdlog::info("invoke null");
    }
    return false;
}
bool select(IUIAutomationElement* pNode) {
    if (pNode) {
        CComPtr<IUIAutomationSelectionItemPattern> pSelect;
        pNode->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void **)&pSelect);
        if (pSelect) {
            pSelect->Select();
            spdlog::debug("select {0}", fromWide(name(pNode)));
            return true;
        }
        else {
            spdlog::info("select failed {0}", fromWide(name(pNode)));
        }
    }
    else {
        spdlog::info("select null");
    }
    return false;
}

bool findDescendantByClass(IUIAutomation* pAutomation, IUIAutomationElement * pSender, _variant_t className){
    if (!pSender || !pAutomation)   {
        spdlog::info("findDescendantByClass pSender or pAutomation null");
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
            // not used beyond this yet
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
                std::wstring test = static_cast<wchar_t*>(cls);
                if (targetClass == test || targetClass.length() == 0) {
                    spdlog::debug("found {0}", fromWide(test));
                    ret = pFound; // loop to see all values when debugging
                }
            }
        }
    }
    if (!ret) {
        spdlog::info("not found {0}", fromWide(targetClass));
    }
    return ret; // not found
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
                name = static_cast<wchar_t*>(senderName);
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
                spdlog::debug("Event UIA_MenuOpenedEventId Received");
                //http://source.roslyn.io/#Microsoft.VisualStudio.IntegrationTest.Utilities/AutomationElementExtensions.cs,1a83d951b02044f1,references
                //http://source.roslyn.io/#Microsoft.VisualStudio.IntegrationTest.Utilities/ScreenshotService.cs
                if (name == L"File") {
                    invoke(find(pAutomation, pSender, _variant_t(L"Options"), L"NetUIOutSpaceButton"));
                }
            }
            break;
        case UIA_MenuModeStartEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_MenuModeStartEventId Received");
            }
            break;
        case UIA_ToolTipOpenedEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_ToolTipOpenedEventId Received");
            }
            break;
        case UIA_Window_WindowOpenedEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_Window_WindowOpenedEventId Received");
                if (name.find(L"Office Add-ins") != std::string::npos) {
                    invoke(find(pAutomation, pSender, _variant_t(L"SHARED FOLDER"), L""));
                    while (1) {
                        Sleep(100UL);
                        UIA_HWND h;
                        pSender->get_CurrentNativeWindowHandle(&h);
                        if (!h) {
                            select(find(pAutomation, pRoot, _variant_t(L"Home"), L"NetUIRibbonTab"));
                            invoke(find(pAutomation, pRoot, _variant_t(L"Contiq"), L""));
                            break;
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
            }
            break;
        case UIA_NotificationEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_NotificationEventId Received");
            }
            break;
        case UIA_SystemAlertEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_SystemAlertEventId Received");
            }
            break;
        case UIA_Window_WindowClosedEventId:
            if (isMe(pSender)) {
                spdlog::debug("Event UIA_Window_WindowClosedEventId Received");
                if (pAutomation) {
                    pAutomation->RemoveAllEventHandlers();
                }
                app->Release();
                pAutomation->Release();
                SetEvent(event); // stop thread
            }
            break;
        default:
            if (isMe(pSender)) {
                spdlog::debug("Event default Received {0}", eventID);
            }
            break;
        }
        return S_OK;
    }
}; 
void createSlide(EventHandler*pEvents) {
    if (!pEvents) {
        spdlog::error("createSlide missing pEvents");
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
        spdlog::error("Presentations {0}", hr);
        return;
    }
    // Call Presentations.Add to create a new presentation
    if (pres.vt != VT_EMPTY) {
        result.Clear();
        hr = AutoWrap(DISPATCH_METHOD, result.GetAddress(), pres, (LPOLESTR)L"Add", 0);
        if (result.vt != VT_EMPTY) {
            pEvents->presentation = result.pdispVal;
        }
        else {
            spdlog::error("Add Presentations {0}", hr);
            return;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    // Insert a new Slide and add some text to it. 
    //  

    _variant_t slides;
    if (pEvents->presentation.vt != VT_EMPTY) {
        result.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), pEvents->presentation, (LPOLESTR)L"Slides", 0);
        if (result.vt != VT_EMPTY) {
            slides = result.pdispVal;
        }
        else {
            spdlog::error("Add Slides {0}", hr);
            return;
        }
    }
    _variant_t slide;
    _variant_t vtIndex(1);
    _variant_t vtLayout(2);
    if (slides.vt != VT_EMPTY) {
        result.Clear();
        // If there are more than 1 parameters passed, they MUST be pass in  
        // reversed order. Otherwise, you may get the error 0x80020009. 
        hr = AutoWrap(DISPATCH_METHOD, result.GetAddress(), slides, (LPOLESTR)L"Add", 2, vtLayout, vtIndex);
        if (result.vt != VT_EMPTY) {
            slide = result.pdispVal;
        }
        else {
            spdlog::error("Add {0}", hr);
            return;
        }
    }
    _variant_t shapes;
    if (slide.vt != VT_EMPTY) {
        result.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), slide, (LPOLESTR)L"Shapes", 0);
        if (result.vt != VT_EMPTY) {
            shapes = result.pdispVal;
        }
        else {
            spdlog::error("Add Shapes {0}", hr);
            return;
        }
    }

    _variant_t shape;
    if (shapes.vt != VT_EMPTY) {
        vtIndex.Clear();
        vtIndex = 1;
        hr = AutoWrap(DISPATCH_METHOD, result.GetAddress(), shapes, (LPOLESTR)L"Item", 1, vtIndex);
        if (result.vt != VT_EMPTY) {
            shape = result.pdispVal;
        }
        else {
            spdlog::error("Add Item {0}", hr);
            return;
        }
    }
    _variant_t frame;
    if (shape.vt != VT_EMPTY) {
        result.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), shape, (LPOLESTR)L"TextFrame", 0);
        if (result.vt != VT_EMPTY) {
            frame = result.pdispVal;
        }
        else {
            spdlog::error("TextFrame {0}", hr);
            return;
        }
    }

    _variant_t range;
    if (frame.vt != VT_EMPTY) {
        result.Clear();
        hr = AutoWrap(DISPATCH_PROPERTYGET, result.GetAddress(), frame, (LPOLESTR)L"TextRange", 0);
        if (result.vt != VT_EMPTY) {
            range = result.pdispVal;
        }
        else {
            spdlog::error("TextRange {0}", hr);
            return;
        }
    }

    if (range.vt != VT_EMPTY) {
        _variant_t welcome(L"Welcome To Contiq");
        AutoWrap(DISPATCH_PROPERTYPUT, NULL, range, (LPOLESTR)L"Text", 1, welcome);
    }
}

int startPPT(EventHandler*pEvents, IUIAutomation* pAutomation) {
    if (!pEvents || !pAutomation) {
        spdlog::error("startPPT !pEvents || !pAutomation");
        return -1;
    }
    CLSID clsid;

    // Get CLSID from ProgID using CLSIDFromProgID. 
    LPCOLESTR progID = L"PowerPoint.Application";
    HRESULT hr = CLSIDFromProgID(progID, &clsid);
    if (FAILED(hr)) {
        spdlog::error("CLSIDFromProgID(\"PowerPoint.Application\") failed w/err {1}", hr);
        return 0;
    }
    // Start the server and get the IDispatch interface 
    hr = CoCreateInstance(        // [-or-] CoCreateInstanceEx, CoGetObject 
        clsid,                    // CLSID of the server 
        NULL,CLSCTX_LOCAL_SERVER,    // PowerPoint.Application is a local server 
        IID_IDispatch,  (void **)&pEvents->app);        // Output 
    if (FAILED(hr)) {
        spdlog::error("PowerPoint is not registered properly w/err {0}", hr);
        return 0;
    }
    pEvents->pid = FindProcessId(L"POWERPNT.EXE");
    spdlog::debug("PowerPoint PID {0}", pEvents->pid);

    // keeps things current
    SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    PostMessage(HWND_BROADCAST, WM_WININICHANGE, SPI_SETSCREENREADER, 0);

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
        spdlog::debug("created {0}", fromWide(p.shi2_netname));
    }
    if (status == 5) {
        spdlog::error("need to be admin");

    }
    else {
        spdlog::error("failed to create: {0}, error {1}", fromWide(p.shi2_netname),  status);
    }
}


struct Parm {
    //myEvents events;
    int type;
    IUIAutomation *automation;
};

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
        spdlog::error("CoCreateInstance failed w/err{0}", hr);
        return 0;
    }
    // Use root element for listening to window and tooltip creation and destruction.
    hr = pAutomation->GetRootElement(&pRoot);
    if (FAILED(hr) || pRoot == NULL)    {
        spdlog::error("GetRootElement failed w/err{0}", hr);
        return 0;
    }
    pdata->handles[1] = CreateEvent(NULL, TRUE, FALSE, NULL);
    pEvents = new EventHandler(pAutomation, pRoot, pdata->handles[1], pdata->type);
    if (pEvents == NULL) {
        spdlog::error("out of memory");
        return 0;
    }
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

    spdlog::info("thread ending");

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
int main(int argc, char**argv) {

    spdlog::set_level(spdlog::level::err);

    try {
        cxxopts::Options options("MyProgram", "One line description of MyProgram");
        options.add_options()
            ("d,debug", "Enable debugging")
            ("f,file", "File name", cxxopts::value<std::string>());
        auto result = options.parse(argc, argv);
        if (result.count("d")){
            spdlog::set_level(spdlog::level::debug);
        }
        // create color multi threaded logger
        spdlog::info("Welcome to Contiq installer!");

        //fireItup(0);
        fireItup(1);

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


    }
    catch (cxxopts::option_not_exists_exception e)    {
        spdlog::error(e.what());
    }
 
}
