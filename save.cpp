 #include <windows.h>
#include <lm.h>
#include <tchar.h>
#include <processthreadsapi.h>
#include <stdio.h>
#include <string.h>         // For wcslen()
#include <memory>           // For std::unique_ptr
#include <stdexcept>        // For std::runtime_error
#include <string>           // For std::wstring
#include <utility>          // For std::pair, std::swap
#include <vector>           // For std::vector
#include <direct.h>  
#include <uiautomation.h>
#include <comutil.h>
#include "ofApp.h"
#include "ofxXmlPoco.h"
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Mincore.lib")
#pragma comment(lib, "comsuppw.lib")
DWORD WINAPI AutomatePowerPointByCOMAPI(LPVOID lpParam);
void postError(const std::wstring& error, DWORD e) {
    MessageBoxW(nullptr, error.c_str(), L"Error!\0", MB_ICONEXCLAMATION | MB_OK);
}
void postMessage(const std::wstring& message) {
    MessageBoxW(nullptr, message.c_str(), L"Message\0", MB_ICONINFORMATION | MB_OK);
}
int askQuestion(const std::wstring& question) {
    return MessageBoxW(nullptr, question.c_str(), L"Question\0", MB_ICONQUESTION | MB_YESNO);
}

std::wstring RegistryGetString(HKEY    key, PCWSTR  subKey,  PCWSTR  valueName){
    //
    // Try getting the size of the string value to read,
    // to properly allocate a buffer for the string
    //
    DWORD keyType = 0;
    DWORD dataSize = 0;
    const DWORD flags = RRF_RT_REG_SZ; // Only read strings (REG_SZ)
    LONG result = ::RegGetValueW(key, subKey, valueName, flags, &keyType, nullptr, &dataSize);
    if (result != ERROR_SUCCESS)    {
        //ERROR_FILE_NOT_FOUND ERROR_UNSUPPORTED_TYPE
        return L"";
    }
    WCHAR* data = new WCHAR[dataSize / sizeof(WCHAR) + sizeof(WCHAR)];
    memset(data, 0, sizeof(data));
    result = ::RegGetValueW(key, subKey, valueName, flags, nullptr, data, &dataSize);
    if (result != ERROR_SUCCESS)    {
        return L"";
    }
    std::wstring text = data;
    delete[] data;
    return text;
}



// CoInitialize must be called before calling this function, and the  
// caller must release the returned pointer when finished with it.
// 
IUIAutomation *automation=nullptr;
HRESULT InitializeUIAutomation(IUIAutomation **ppAutomation){
    CoInitialize(nullptr);
    return CoCreateInstance(CLSID_CUIAutomation, NULL,
        CLSCTX_INPROC_SERVER, IID_IUIAutomation,
        reinterpret_cast<void**>(ppAutomation));
}
std::wstring GetInstallPath(){
    std::wstring location = RegistryGetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\Software\\Wow6432Node\\Microsoft\\Office\\16.0\\Common\\InstallRoot", L"Path");
    if (location.size() > 0) {
        return location;
    }
    return RegistryGetString(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Office\\ClickToRun\\REGISTRY\\MACHINE\\Software\\Wow6432Node\\Microsoft\\Office\\15.0\\Common\\InstallRoot", L"Path");
}

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;

BOOL IsWow64(){
    BOOL bIsWow64 = FALSE;

    //IsWow64Process is not available on all supported versions of Windows.
    //Use GetModuleHandle to get a handle to the DLL that contains the function
    //and GetProcAddress to get a pointer to the function if available.

    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
        GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

    if (NULL != fnIsWow64Process)    {
        if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))        {
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

    if (verSize != NULL)    {
        WCHAR* verData = new WCHAR[verSize];
        if (GetFileVersionInfoW(filePath.c_str(), verHandle, verSize, verData))        {
            if (VerQueryValueW(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size))            {
                if (size)                {
                    VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
                    if (verInfo->dwSignature == 0xfeef04bd)                    {
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

/*
 Run Setup (exe file) to install VBA macros and run below steps after completing the setup.

================= Copy manifest File & Enable  =================

1) Create a folder "ContiqManifest" in any drive and copy manifest file (xml file) in that folder.

2)  Share this folder with logged in user. (Right Click > Properties > Sharing > Click on Share... > You will see logged in user in the list automatically > Select that user and click Share.)

3) Go to properties of that folder, go to Sharing tab & copy the Network Path.

4) Open PowerPoint > Start with blank Presentation, go to File > Options > Trust Center > Trust Center Settings > Trusted Add-In Catalogs.

5) Paste Network path (copied in step 3) to the Catalog URL, then click Add Catalog & check "Show In Menu" option in the below available list. Click "OK" to save changes.

6) Restart your PowerPoint & open/start new presentation.

================= Include Web Add-In in PowerPoint  =================

1) Go to Insert > Click on "My Add-ins" > Go to "SHARED FOLDER" tab > Select Contiq plugin and click Add.

*/
//--------------------------------------------------------------
IUIAutomationElement* GetTopLevelWindowByName(LPWSTR windowName){
    if (!windowName)    {
        return NULL;
    }

    IUIAutomationElement* pRoot;
    IUIAutomationElement* pFound;
    VARIANT varProp;
    varProp.vt = VT_BSTR;
    varProp.bstrVal = SysAllocString(windowName);

    // Get the desktop element
    HRESULT hr = automation->GetRootElement(&pRoot);

    // Get a top-level element by name, such as "Program Manager"
    BSTR name;
    if (pRoot)    {
        IUIAutomationCondition* pCondition;
        automation->CreatePropertyCondition(UIA_NamePropertyId,
            varProp, &pCondition);
        pRoot->FindFirst(TreeScope_Children, pCondition, &pFound);
        pRoot->Release();
        pCondition->Release();
        if (pFound){
            hr = pFound->get_CurrentName(&name);
            if (SUCCEEDED(hr)) {

            }
        }
    }

    VariantClear(&varProp);
    return pFound;
}
void justEcho(const std::wstring& name, _bstr_t& bs) {
    wprintf_s(L"%s = %s\n",  name.c_str(), static_cast<wchar_t*>(bs));
}
void findCheckBox(IUIAutomationElement * pElement) {
    IUIAutomationCondition * pCheckBoxProp;
    VARIANT varCheckBox;
    varCheckBox.vt = VT_I4;
    varCheckBox.lVal = UIA_CheckBoxControlTypeId;
    automation->CreatePropertyCondition(
        UIA_ControlTypePropertyId,
        varCheckBox, &pCheckBoxProp);

    IUIAutomationElement * pFound;
    pElement->FindFirst(TreeScope_Descendants,
        pCheckBoxProp, &pFound);

}

HRESULT GetAnnotationPattern(){
    // get comments https://code.msdn.microsoft.com/windowsdesktop/UI-Automation-Document-24a37c82/sourcecode?fileId=42885&pathId=1331485365
    return (HRESULT)0;
}
void invoke(IUIAutomationElement* pNode) {
    if (pNode) {
        IUIAutomationInvokePattern * pInvoke;
        pNode->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void **)&pInvoke);
        if (pInvoke) {
            pInvoke->Invoke();
        }
    }

}
// CAUTION: Do not pass in the root (desktop) element. Traversing the entire subtree
// of the desktop could take a very long time and even lead to a stack overflow.
void ListDescendants(IUIAutomationElement* pParent, int indent)
{
    if (pParent == NULL)
        return;

    IUIAutomationTreeWalker* pControlWalker = NULL;
    IUIAutomationElement* pNode = NULL;
    automation->get_ControlViewWalker(&pControlWalker);
    if (pControlWalker == NULL)
        goto cleanup;

    pControlWalker->GetFirstChildElement(pParent, &pNode);
    if (pNode == NULL)
        goto cleanup;

    while (pNode)
    {
        BOOL b;
        for (int x = 0; x <= indent; x++)
        {
            std::wcout << L"   ";
        }
        pNode->get_CurrentIsControlElement(&b);
        _bstr_t cls;
        pNode->get_CurrentClassName(cls.GetAddress());
        justEcho(L"cls", cls);
        _bstr_t name;
        pNode->get_CurrentName(name.GetAddress());
        justEcho(L"name", name);
        _bstr_t status;
        pNode->get_CurrentItemStatus(status.GetAddress());
        justEcho(L"status", status);
        _bstr_t desc;
        pNode->get_CurrentLocalizedControlType(desc.GetAddress());
        justEcho(L"desc", desc);
        _bstr_t id;
        pNode->get_CurrentAutomationId(id.GetAddress());
        justEcho(L"id", id);
        //BSTR providerDesc;
        //pNode->get_CurrentProviderDescription(&providerDesc);
        //echoAndFree(providerDesc);
        // IUIAutomationInvokePattern::Invoke
        // https://github.com/hnakamur/w32uiautomation hints
        // select no
        int i = 0;
        _bstr_t simpleui(L"NetUISimpleButton");
        _bstr_t blank(L"Blank Presentation");
        _bstr_t fileTab(L"File Tab");
        _bstr_t myID(L"AIOStartDocument");
        _bstr_t fileID(L"FileTabButton");
        _bstr_t no(L"No");
        _bstr_t two(2);
        if (id == two && name == no) {
            invoke(pNode);
        }
        if (id == myID && name == blank && cls == simpleui) {
            invoke(pNode);
        }
        if (id == fileID && name == fileTab) {
            invoke(pNode);
        }

        HRESULT h;

        ListDescendants(pNode, indent + 1);
        IUIAutomationElement* pNext;
        pControlWalker->GetNextSiblingElement(pNode, &pNext);
        pNode->Release();
        pNode = pNext;
    }

cleanup:
    if (pControlWalker != NULL)
        pControlWalker->Release();

    if (pNode != NULL)
        pNode->Release();

    return;
}
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
HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp,
    LPOLESTR ptName, int cArgs...)
{
    // Begin variable-argument list 
    va_list marker;
    va_start(marker, cArgs);

    if (!pDisp)
    {
        _putws(L"NULL IDispatch passed to AutoWrap()");
        _exit(0);
        return E_INVALIDARG;
    }

    // Variables used 
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;

    // Get DISPID for name passed 
    hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if (FAILED(hr))
    {
        wprintf(L"IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx\n",
            ptName, hr);
        _exit(0);
        return hr;
    }

    // Allocate memory for arguments 
    VARIANT *pArgs = new VARIANT[cArgs + 1];
    // Extract arguments... 
    for (int i = 0; i < cArgs; i++)
    {
        pArgs[i] = va_arg(marker, VARIANT);
    }

    // Build DISPPARAMS 
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts 
    if (autoType & DISPATCH_PROPERTYPUT)
    {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    // Make the call 
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
        autoType, &dp, pvResult, NULL, NULL);
    if (FAILED(hr))
    {
        wprintf(L"IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx\n",
            ptName, dispID, hr);
        _exit(0);
        return hr;
    }

    // End variable-argument section 
    va_end(marker);

    delete[] pArgs;

    return hr;
}


// 
//   FUNCTION: GetModuleDirectory(LPWSTR, DWORD); 
// 
//   PURPOSE: This is a helper function in this sample. It retrieves the  
//      fully-qualified path for the directory that contains the executable  
//      file of the current process. For example, "D:\Samples\". 
// 
//   PARAMETERS: 
//      * pszDir - A pointer to a buffer that receives the fully-qualified  
//      path for the directory taht contains the executable file of the  
//      current process. If the length of the path is less than the size that  
//      the nSize parameter specifies, the function succeeds and the path is  
//      returned as a null-terminated string. 
//      * nSize - The size of the lpFilename buffer, in characters. 
// 
//   RETURN VALUE: If the function succeeds, the return value is the length  
//      of the string that is copied to the buffer, in characters, not  
//      including the terminating null character. If the buffer is too small  
//      to hold the directory name, the function returns 0 and sets the last  
//      error to ERROR_INSUFFICIENT_BUFFER. If the function fails, the return  
//      value is 0 (zero). To get extended error information, call  
//      GetLastError. 
// 
DWORD GetModuleDirectory(LPWSTR pszDir, DWORD nSize);


// 
//   FUNCTION: AutomatePowerPointByCOMAPI(LPVOID) 
// 
//   PURPOSE: Automate Microsoft PowerPoint using C++ and the COM APIs. 
// 
DWORD WINAPI AutomatePowerPointByCOMAPI(LPVOID lpParam)
{
    // Initializes the COM library on the current thread and identifies  
    // the concurrency model as single-thread apartment (STA).  
    // [-or-] CoInitialize(NULL); 
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
    if (FAILED(hr))
    {
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

    if (FAILED(hr))
    {
        wprintf(L"PowerPoint is not registered properly w/err 0x%08lx\n", hr);
        return 1;
    }

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
    //} 


    // Get the OS collection 
    IDispatch *pOS = NULL;
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pPpApp, L"OperatingSystem", 0);
        pOS = result.pdispVal;
        _putws(result.bstrVal);
    }

    // Get the build 
    IDispatch *pBuild = NULL;
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pPpApp, L"Build", 0);
        pBuild = result.pdispVal;
        _putws(result.bstrVal);
    }


    ///////////////////////////////////////////////////////////////////////// 
    // Create a new Presentation. (i.e. Application.Presentations.Add) 
    //  

    // Get the Presentations collection 
    IDispatch *pPres = NULL;
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pPpApp, L"Presentations", 0);
        pPres = result.pdispVal;
    }

    // Call Presentations.Add to create a new presentation 
    IDispatch *pPre = NULL;
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_METHOD, &result, pPres, L"Add", 0);
        pPre = result.pdispVal;
    }

    _putws(L"A new presentation is created");


    ///////////////////////////////////////////////////////////////////////// 
    // Insert a new Slide and add some text to it. 
    //  

    // Get the Slides collection 
    IDispatch *pSlides = NULL;
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pPre, L"Slides", 0);
        pSlides = result.pdispVal;
    }

    // Insert a new slide 
    _putws(L"Insert a slide");

    IDispatch *pSlide = NULL;
    {
        VARIANT vtIndex;
        vtIndex.vt = VT_I4;
        vtIndex.lVal = 1;

        VARIANT vtLayout;
        vtLayout.vt = VT_I4;
        vtLayout.lVal = 2;    // PowerPoint::PpSlideLayout::ppLayoutText 

        VARIANT result;
        VariantInit(&result);
        // If there are more than 1 parameters passed, they MUST be pass in  
        // reversed order. Otherwise, you may get the error 0x80020009. 
        AutoWrap(DISPATCH_METHOD, &result, pSlides, L"Add", 2, vtLayout, vtIndex);
        pSlide = result.pdispVal;
    }

    // Add some texts to the slide 
    _putws(L"Add some texts");

    IDispatch *pShapes = NULL;        // pSlide->Shapes 
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pSlide, L"Shapes", 0);
        pShapes = result.pdispVal;
    }

    IDispatch *pShape = NULL;        // pShapes->Item(1) 
    {
        VARIANT vtIndex;
        vtIndex.vt = VT_I4;
        vtIndex.lVal = 1;

        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_METHOD, &result, pShapes, L"Item", 1, vtIndex);
        pShape = result.pdispVal;
    }

    IDispatch *pTxtFrame = NULL;    // pShape->TextFrame 
    {
        VARIANT result;
        VariantInit(&result);
        hr = AutoWrap(DISPATCH_PROPERTYGET, &result, pShape, L"TextFrame", 0);
        pTxtFrame = result.pdispVal;
    }

    IDispatch *pTxtRange = NULL;    // pTxtFrame->TextRange 
    {
        VARIANT result;
        VariantInit(&result);
        AutoWrap(DISPATCH_PROPERTYGET, &result, pTxtFrame, L"TextRange", 0);
        pTxtRange = result.pdispVal;
    }

    {
        VARIANT x;
        x.vt = VT_BSTR;
        x.bstrVal = SysAllocString(L"All-In-One Code Framework");
        AutoWrap(DISPATCH_PROPERTYPUT, NULL, pTxtRange, L"Text", 1, x);
        VariantClear(&x);
    }


    ///////////////////////////////////////////////////////////////////////// 
    // Save the presentation as a pptx file and close it. 
    //  

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

    // pPre->Close() 
    AutoWrap(DISPATCH_METHOD, NULL, pPre, L"Close", 0);


    ///////////////////////////////////////////////////////////////////////// 
    // Quit the PowerPoint application. (i.e. Application.Quit()) 
    //  

    _putws(L"Quit the PowerPoint application");
    AutoWrap(DISPATCH_METHOD, NULL, pPpApp, L"Quit", 0);


    ///////////////////////////////////////////////////////////////////////// 
    // Release the COM objects. 
    //  

    if (pTxtRange != NULL)
    {
        pTxtRange->Release();
    }
    if (pTxtFrame != NULL)
    {
        pTxtFrame->Release();
    }
    if (pShape != NULL)
    {
        pShape->Release();
    }
    if (pShapes != NULL)
    {
        pShapes->Release();
    }
    if (pSlide != NULL)
    {
        pSlide->Release();
    }
    if (pSlides != NULL)
    {
        pSlides->Release();
    }
    if (pPre != NULL)
    {
        pPre->Release();
    }
    if (pPres != NULL)
    {
        pPres->Release();
    }
    if (pPpApp != NULL)
    {
        pPpApp->Release();
    }

    // Uninitialize COM for this thread 
    CoUninitialize();

    return 0;
}

//   FUNCTION: GetModuleDirectory(LPWSTR, DWORD); 
// 
//   PURPOSE: This is a helper function in this sample. It retrieves the  
//      fully-qualified path for the directory that contains the executable  
//      file of the current process. For example, "D:\Samples\". 
// 
//   PARAMETERS: 
//      * pszDir - A pointer to a buffer that receives the fully-qualified  
//      path for the directory taht contains the executable file of the  
//      current process. If the length of the path is less than the size that  
//      the nSize parameter specifies, the function succeeds and the path is  
//      returned as a null-terminated string. 
//      * nSize - The size of the lpFilename buffer, in characters. 
// 
//   RETURN VALUE: If the function succeeds, the return value is the length  
//      of the string that is copied to the buffer, in characters, not  
//      including the terminating null character. If the buffer is too small  
//      to hold the directory name, the function returns 0 and sets the last  
//      error to ERROR_INSUFFICIENT_BUFFER. If the function fails, the return  
//      value is 0 (zero). To get extended error information, call  
//      GetLastError. 
// 
DWORD GetModuleDirectory(LPWSTR pszDir, DWORD nSize)
{
    // Retrieve the path of the executable file of the current process. 
    nSize = GetModuleFileNameW(NULL, pszDir, nSize);
    if (!nSize || GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        *pszDir = L'\0'; // Ensure it's NULL terminated 
        return 0;
    }

    // Run through looking for the last slash in the file path. 
    // When we find it, NULL it to truncate the following filename part. 

    for (int i = nSize - 1; i >= 0; i--)
    {
        if (pszDir[i] == L'\\' || pszDir[i] == L'/')
        {
            pszDir[i + 1] = L'\0';
            nSize = i + 1;
            break;
        }
    }
    return nSize;
}
/*
void getMenus() {
    AutomationSearchCondition condition = AutomationSearchCondition.ByControlType(ControlType.MenuItem);
    var finder = new AutomationElementFinder(parent);
    finder = Retry.For(() = > PerformanceHackAsPopupMenuForWin32AppComesOnDesktop(finder, parent), CoreAppXmlConfiguration.Instance.BusyTimeout());
    List<AutomationElement> children = finder.Descendants(condition);
    foreach(AutomationElement child in children)
        Add((Menu)Factory.Create(child, actionListener));
}
*/
//https://docs.microsoft.com/en-us/officeupdates/update-history-office-2019
    void ofApp::setup(){
        HANDLE hThread;

        // Demonstrate automating PowerPoint using C++ and the COM APIs in a  
        // separate thread. 
       // hThread = CreateThread(NULL, 0, AutomatePowerPointByCOMAPI, NULL, 0, NULL);
       // WaitForSingleObject(hThread, INFINITE);
       // CloseHandle(hThread);
        //https://docs.microsoft.com/en-us/windows/desktop/winauto/uiauto-controltypesoverview
    HRESULT hr = InitializeUIAutomation(&automation);
    SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    PostMessage(HWND_BROADCAST, WM_WININICHANGE, SPI_SETSCREENREADER, 0);
    //GetTopLevelWindowByName(L"Program Manager");
    // assumes ppt running make sure this is the case and only our instance is running
    IUIAutomationElement*parent = GetTopLevelWindowByName(L"PowerPoint");
    ListDescendants(parent, 5); // turn off the popup i get
    ofSleepMillis(1000);
    ListDescendants(parent, 5); // select pt
    ofSleepMillis(1000);
    ListDescendants(parent, 5); // title changes
    // Get the element under the cursor
    // Use GetPhysicalCursorPos to interact properly with
    // High DPI
    POINT pt;
    GetPhysicalCursorPos(&pt);

    IUIAutomationElement *pAtMouse;
    hr = automation->ElementFromPoint(pt, &pAtMouse);
    if (FAILED(hr))
        return ;

    // Get the element's name and print it
    BSTR name;
    hr = pAtMouse->get_CurrentName(&name);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Element's Name: %s \n", name);
        SysFreeString(name);
    }

    // Get the element's Control Type (in the current languange)
    // and print it
    BSTR controlType;
    hr = pAtMouse->get_CurrentLocalizedControlType(&controlType);
    if (SUCCEEDED(hr))
    {
        wprintf(L"Element's Control Type: %s \n", controlType);
        SysFreeString(controlType);
    }

    // Clean up our COM pointers
    pAtMouse->Release();
    automation->Release();
    CoUninitialize();
    ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
    ofSetBackgroundColor(ofColor::black);
    ofSetColor(ofColor::white);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofLogToConsole();
    uint16_t major, minor, revison, build;

    //
    std::wstring path = GetInstallPath();
    std::wstring cmd = L"POWERPNT.exe";
    std::wstring location;
    if (path.size() > 0) {
        //major.minor.build.revision
        fileVersion(path + cmd, major, minor, revison, build);
        wcout << path + cmd << L" ver " << major << L"." << minor << L"." << revison << L"." << build;
       _wchdir(path.c_str());
    }
    else {
        postError(L"Powerpoint must be installed", 0);
        return;
    }
    HWND ppt = nullptr;
    do {
        //Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\ClickToRun
    //https://stackoverflow.com/questions/2113950/how-to-send-keystrokes-to-a-window
        ppt = FindWindowW(NULL, _T("PowerPoint"));
        wchar_t buf[128];
        buf[0] = L'\0';
        if (ppt) {
            GetClassNameW(ppt, buf, 127); //PPTFrameClass do we need this?
        }
        // echo ppt version
        //https://docs.microsoft.com/en-us/officeupdates/update-history-office-2019
        if (!ppt) {

            STARTUPINFO info = { sizeof(info) };
            PROCESS_INFORMATION processInfo;
            if (!CreateProcessW((path+cmd).c_str(), L"data\\ppt1.pptx", NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))    {
                postMessage(L"Microsoft Powerpoint is not properly installed");
                return;
            }    
            if (!SetForegroundWindow(ppt)) {
                ofLogFatalError("SetForegroundWindow") << "failed to create: " << _T("ppt");

            }
            //EM_REPLACESEL WM_SETTEXT
            //SendMessage(ppt, EM_REPLACESEL, NULL, (LPARAM)_T("hello"));
            //Fill in the array of keystrokes to send.
            // https://docs.microsoft.com/en-gb/windows/desktop/inputdev/virtual-key-codes
            INPUT input;
            WORD vkey = VK_RETURN;//
            
            //
//VK_F12; // see link below

            // clear any first time usage popups
            for (int i = 0; i < 3; ++i){
                SendMessage(ppt, VK_RETURN, NULL, NULL);
                input.type = INPUT_KEYBOARD;
                input.ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
                input.ki.time = 0;
                input.ki.dwExtraInfo = 0;
                input.ki.wVk = vkey;
                input.ki.dwFlags = 0; // there is no KEYEVENTF_KEYDOWN KEYEVENTF_UNICODE
               // SendInput(1, &input, sizeof(INPUT));
                input.ki.dwFlags = KEYEVENTF_KEYUP;
              //  SendInput(1, &input, sizeof(INPUT));
            }




            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);

            //Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Wow6432Node\Microsoft\Office\16.0\Common\InstallRoot
            /*V16_SAMEBIT_CTR - SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Microsoft\Office\16.0\Visio
              V16X86_X64_CTR - SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Wow6432Node\Microsoft\Office\16.0\Visio
            */
            //C:\Program Files (x86)\Microsoft Office\root\Office16\POWERPNT.EXE

        }
        else {
            postMessage(L"Please stop all instances of Microsoft Powerpoint");
        }
    } while (true);
    // loop look/ask user to close PowerPoint? https://docs.microsoft.com/en-gb/windows/desktop/ToolHelp/taking-a-snapshot-and-viewing-processes

    //Bring the Notepad window to the front.
    if (!SetForegroundWindow(ppt)) {
        ofLogFatalError("SetForegroundWindow") << "failed to create: " << _T("Notepad");

    }
   // SendMessage(edit, WM_SETTEXT, NULL, (LPARAM)_T("hello"));

    ofDirectory dir("ContiqManifest");
    if (dir.create()) {
        ofLogNotice("ofDirectory") << "created: " << dir.path();
        ofFilePath file;
        ofxXmlPoco xml;
        //xml.setTo(getXML());
        SHARE_INFO_2 p;
        DWORD parm_err = 0;
        // be sure to delete when done
        p.shi2_netname = TEXT("TESTSHARE"); // make a guid name
        p.shi2_type = STYPE_DISKTREE; 
        p.shi2_remark = TEXT("Contiq Install Share");
        p.shi2_permissions = 0;
        p.shi2_max_uses = 1;
        p.shi2_current_uses = 0;
        p.shi2_path = TEXT(".");
        p.shi2_passwd = NULL; // no password
        NET_API_STATUS status = NetShareAdd(NULL, 2, (LPBYTE)&p, &parm_err);
        if (!status) {
            ofLogNotice("NetShareAdd") << "created: " << p.shi2_netname;
        }
        else {
            ofLogFatalError("NetShareAdd") << "failed to create: " << p.shi2_netname << " error " << GetLastError();
        }
        // close here once installed --- how?
        NetShareDel(NULL, L"TESTSHARE", 0L);
    }
    else {
        ofSystemAlertDialog("Cannot create"); // fill these in
    }

}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
