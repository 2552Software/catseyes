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
#include "ofApp.h"
#include "ofxXmlPoco.h"
#pragma comment(lib, "Netapi32.lib")
#pragma comment(lib, "Mincore.lib")

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
void ofApp::setup(){

    ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
    ofSetBackgroundColor(ofColor::black);
    ofSetColor(ofColor::white);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofLogToConsole();
    uint16_t major, minor, revison, build;

    //
    std::wstring location = GetInstallPath();
    if (location.size() > 0) {
        location += L"POWERPNT.exe";
        //major.minor.build.revision
        fileVersion(location, major, minor, revison, build);
        wcout << location << L" ver " << major << L"." << minor << L"." << revison << L"." << build;
       _wchdir(location.c_str());
    }
    else {
        postError(L"Powerpoint must be installed", 0);
        return;
    }
    HWND ppt = nullptr;
    do {
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
            //Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\ClickToRun

            //Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Wow6432Node\Microsoft\Office\16.0\Common\InstallRoot
            /*V16_SAMEBIT_CTR - SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Microsoft\Office\16.0\Visio
              V16X86_X64_CTR - SOFTWARE\Microsoft\Office\ClickToRun\REGISTRY\MACHINE\Software\Wow6432Node\Microsoft\Office\16.0\Visio
            */
            //C:\Program Files (x86)\Microsoft Office\root\Office16\POWERPNT.EXE
            // ShellExecute needs COM to be initialized
            CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

            
            HINSTANCE h = ShellExecute(GetDesktopWindow(), L"open", L"POWERPNT.exe", L"data\\ppt1.pptx", 0, SW_SHOWNORMAL);
            if ((int)h < 32) {
                postMessage(L"Microsoft Powerpoint is not properly installed");
            }

            CoUninitialize();

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
