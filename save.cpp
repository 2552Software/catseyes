#include <windows.h>
#include <lm.h>
#include <tchar.h>
#include <processthreadsapi.h>
#include "ofApp.h"
#include "ofxXmlPoco.h"
#pragma comment(lib, "Netapi32.lib")
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
std::string getXML() {
    std::string xml = "< ? xml version = \"1.0\" encoding = \"utf - 8\" ? >";
    return xml;
    /*
        <!--Published:70EDFC97 - B41D - 43C5 - B751 - 7C00AD999804-->
        <!--Created:ce44715c - 8c4e - 446b - 879c - ea9ebe0f09c8-->
        <OfficeApp xmlns = "http://schemas.microsoft.com/office/appforoffice/1.1" xmlns:xsi = "http://www.w3.org/2001/XMLSchema-instance" xmlns : bt = "http://schemas.microsoft.com/office/officeappbasictypes/1.0" xmlns : ov = "http://schemas.microsoft.com/office/taskpaneappversionoverrides" xsi : type = "TaskPaneApp">

        <!--Begin Basic Settings : Add - in metadata, used for all versions of Office unless override provided. -->

        <!--IMPORTANT!Id must be unique for your add - in, if you reuse this manifest ensure that you change this id to a new GUID. -->
        < Id>78841981 - 1345 - 488a - b421 - 805271f60630< / Id>

        <!--Version.Updates from the store only get triggered if there is a version change. -->
        <Version>1.0.0.0< / Version>
        <ProviderName>Contiq< / ProviderName>
        <DefaultLocale>en - US< / DefaultLocale>
        <!--The display name of your add - in.Used on the store and various places of the Office UI such as the add - ins dialog. -->
        <DisplayName DefaultValue = "Contiq Companion Beta" / >
        <Description DefaultValue = "Contiq Companion Beta" / >

        <!--Icon for your add - in.Used on installation screens and the add - ins dialog. -->
        <IconUrl DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button32x32.png" / >

        <SupportUrl DefaultValue = "https://contiq.com/support/" / >

        <!--Domains that will be allowed when navigating.For example, if you use ShowTaskpane and then have an href link, navigation will only be allowed if the domain is on this list. -->
        <AppDomains>
        <AppDomain>AppDomain1< / AppDomain>
        <AppDomain>AppDomain2< / AppDomain>
        <AppDomain>AppDomain3< / AppDomain>
        < / AppDomains>
        <!--End Basic Settings. -->

        <!--Begin TaskPane Mode integration.This section is used if there are no VersionOverrides or if the Office client version does not support add - in commands. -->
        <Hosts>
        <Host Name = "Presentation" / >
        < / Hosts>
        <DefaultSettings>
        <SourceLocation DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Home.html" / >
        < / DefaultSettings>
        <!--End TaskPane Mode integration.  -->

        <Permissions>ReadWriteDocument< / Permissions>

        <!--Begin Add - in Commands Mode integration. -->
        <VersionOverrides xmlns = "http://schemas.microsoft.com/office/taskpaneappversionoverrides" xsi:type = "VersionOverridesV1_0">

        <!--The Hosts node is required. -->
        <Hosts>
        <!--Each host can have a different set of commands. -->
        <!--Excel host is Workbook, Word host is Document, and PowerPoint host is Presentation. -->
        <!--Make sure the hosts you override match the hosts declared in the top section of the manifest. -->
        <Host xsi : type = "Presentation">
        <!--Form factor.Currently only DesktopFormFactor is supported. -->
        <DesktopFormFactor>
        <!--"This code enables a customizable message to be displayed when the add-in is loaded successfully upon individual install."-->
        <GetStarted>
        <!--Title of the Getting Started callout.resid points to a ShortString resource-->
        <Title resid = "Contiq.GetStarted.Title" / >

        <!--Description of the Getting Started callout.resid points to a LongString resource-->
        <Description resid = "Contiq.GetStarted.Description" / >

        <!--Point to a url resource which details how the add - in should be used. -->
        <LearnMoreUrl resid = "Contiq.GetStarted.LearnMoreUrl" / >
        < / GetStarted>
        <!--Function file is a HTML page that includes the JavaScript where functions for ExecuteAction will be called.
        Think of the FunctionFile as the code behind ExecuteFunction. -->
        <FunctionFile resid = "Contiq.DesktopFunctionFile.Url" / >

        <!--PrimaryCommandSurface is the main Office Ribbon. -->
        <ExtensionPoint xsi : type = "PrimaryCommandSurface">
        <!--Use OfficeTab to extend an existing Tab.Use CustomTab to create a new tab. -->
        <OfficeTab id = "TabHome">
        <!--Ensure you provide a unique id for the group.Recommendation for any IDs is to namespace using your company name. -->
        <Group id = "Contiq.Group1">
        <!--Label for your group.resid must point to a ShortString resource. -->
        <Label resid = "Contiq.Group1Label" / >
        <!--Icons.Required sizes 16, 32, 80, optional 20, 24, 40, 48, 64. Strongly recommended to provide all sizes for great UX. -->
        <!--Use PNG icons.All URLs on the resources section must use HTTPS. -->
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>

        <!--Control.It can be of type "Button" or "Menu". -->
        <Control xsi : type = "Button" id = "Contiq.TaskpaneButton">
        <Label resid = "Contiq.TaskpaneButton.Label" / >
        <Supertip>
        <!--ToolTip title.resid must point to a ShortString resource.-->
        <Title resid = "Contiq.TaskpaneButton.ShortText" / >
        <!--ToolTip description.resid must point to a LongString resource.-->
        <Description resid = "Contiq.TaskpaneButton.Tooltip" / >
        < / Supertip>
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>

        <!--This is what happens when the command is triggered(E.g.click on the Ribbon).Supported actions are ExecuteFunction or ShowTaskpane.-->
        <Action xsi : type = "ShowTaskpane">
        <TaskpaneId>ButtonId1< / TaskpaneId>
        <!--Provide a url resource id for the location that will be displayed on the task pane.-->
        <SourceLocation resid = "Contiq.Taskpane.Url" / >
        < / Action>
        < / Control>
        <!--Control.It can be of type "Button" or "Menu".-->
        <!--<Control xsi : type = "Button" id = "Contiq.TaskpaneButton1">
        <Label resid = "Contiq.TaskpaneButton1.Label" / >
        <Supertip>
        --><!--ToolTip title.resid must point to a ShortString resource.--><!--
        <Title resid = "Contiq.TaskpaneButton1.Label" / >
        --><!--ToolTip description.resid must point to a LongString resource.--><!--
        <Description resid = "Contiq.TaskpaneButton1.Tooltip" / >
        < / Supertip>
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>

        --><!--This is what happens when the command is triggered(E.g.click on the Ribbon).Supported actions are ExecuteFunction or ShowTaskpane.--><!--
        <Action xsi : type = "ExecuteFunction">
        <FunctionName>publishDocument< / FunctionName>
        <TaskpaneId>ButtonId1< / TaskpaneId>
        Provide a url resource id for the location that will be displayed on the task pane.
        <SourceLocation resid = "Contiq.Taskpane.Url" / >
        < / Action>
        < / Control>-->
        < !--<Control xsi : type = "Menu" id = "ContiqDropDown">
        <Label resid = "Contiq.TaskpaneMainButton.Label" / >
        <Supertip>
        <Title resid = "Contiq.TaskpaneButton.Label" / >
        <Description resid = "Contiq.TaskpaneButton.Tooltip" / >
        < / Supertip>
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>
        <Items>
        <Item id = "Contiq.TaskpaneButton">
        <Label resid = "Contiq.TaskpaneButton.Label" / >
        <Supertip>
        <Title resid = "Contiq.TaskpaneButton.Label" / >
        <Description resid = "Contiq.TaskpaneButton.Tooltip" / >
        < / Supertip>
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>
        <Action xsi : type = "ShowTaskpane">
        <TaskpaneId>ButtonId1< / TaskpaneId>
        <SourceLocation resid = "Contiq.Taskpane.Url" / >
        < / Action>
        < / Item>
        <Item id = "Contiq.TaskpaneButton1">
        <Label resid = "Contiq.TaskpaneButton1.Label" / >
        <Supertip>
        <Title resid = "Contiq.TaskpaneButton1.Label" / >
        <Description resid = "Contiq.TaskpaneButton1.Tooltip" / >
        < / Supertip>
        <Icon>
        <bt:Image size = "16" resid = "Contiq.tpicon_16x16" / >
        <bt:Image size = "20" resid = "Contiq.tpicon_20x20" / >
        <bt:Image size = "24" resid = "Contiq.tpicon_24x24" / >
        <bt:Image size = "32" resid = "Contiq.tpicon_32x32" / >
        <bt:Image size = "40" resid = "Contiq.tpicon_40x40" / >
        <bt:Image size = "48" resid = "Contiq.tpicon_48x48" / >
        <bt:Image size = "64" resid = "Contiq.tpicon_64x64" / >
        <bt:Image size = "80" resid = "Contiq.tpicon_80x80" / >
        < / Icon>
        <Action xsi : type = "ExecuteFunction">
        <FunctionName>publishDocument< / FunctionName>
        < / Action>
        < / Item>
        < / Items>
        < / Control>-- >
        < / Group>
        < / OfficeTab>
        < / ExtensionPoint>
        < / DesktopFormFactor>
        < / Host>
        < / Hosts>

        <!--You can use resources across hosts and form factors. -->
        <Resources>
        <bt:Images>
        <bt:Image id = "Contiq.tpicon_16x16" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button16x16.png" / >
        <bt:Image id = "Contiq.tpicon_20x20" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button20x20.png" / >
        <bt:Image id = "Contiq.tpicon_24x24" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button24x24.png" / >
        <bt:Image id = "Contiq.tpicon_32x32" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button32x32.png" / >
        <bt:Image id = "Contiq.tpicon_40x40" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button40x40.png" / >
        <bt:Image id = "Contiq.tpicon_48x48" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button48x48.png" / >
        <bt:Image id = "Contiq.tpicon_64x64" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button64x64.png" / >
        <bt:Image id = "Contiq.tpicon_80x80" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Images/Button80x80.png" / >
        < / bt:Images>
        <bt:Urls>
        <bt:Url id = "Contiq.DesktopFunctionFile.Url" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Functions/FunctionFile.html" / >
        <bt:Url id = "Contiq.Taskpane.Url" DefaultValue = "https://dev.contiq.com/webaddin/PowerPoint/stage/Home.html" / >
        <bt:Url id = "Contiq.GetStarted.LearnMoreUrl" DefaultValue = "https://go.microsoft.com/fwlink/?LinkId=276812" / >
        < / bt:Urls>
        <!--ShortStrings max characters == 125. -->
        <bt:ShortStrings>
        <bt:String id = "Contiq.TaskpaneMainButton.Label" DefaultValue = "Contiq" / >
        <!--<bt:String id = "Contiq.TaskpaneButton.Label" DefaultValue = "Open Contiq" / >-->
        <bt:String id = "Contiq.TaskpaneButton.Label" DefaultValue = "Contiq" / >
        <bt:String id = "Contiq.TaskpaneButton.ShortText" DefaultValue = "Open Contiq" / >
        <bt:String id = "Contiq.TaskpaneButton1.Label" DefaultValue = "Save to Contiq" / >
        <bt:String id = "Contiq.Group1Label" DefaultValue = "Add-ins" / >
        <bt:String id = "Contiq.GetStarted.Title" DefaultValue = "Get started with Contiq!" / >
        < / bt:ShortStrings>
        <!--LongStrings max characters == 250. -->
        <bt:LongStrings>
        <bt:String id = "Contiq.TaskpaneButton.Tooltip" DefaultValue = "Search and reuse slides from existing decks" / >
        <bt:String id = "Contiq.TaskpaneButton1.Tooltip" DefaultValue = "Save this document at Contiq server." / >
        <bt:String id = "Contiq.GetStarted.Description" DefaultValue = "Contiq add-in loaded successfully. Click on the logo above to open the add-in." / >
        < / bt:LongStrings>
        < / Resources>
        < / VersionOverrides>
        <!--End Add - in Commands Mode integration. -->
        < / OfficeApp>
            */

}
//--------------------------------------------------------------
void ofApp::setup(){

    ofSetWindowShape(ofGetScreenWidth(), ofGetScreenHeight());
    ofSetBackgroundColor(ofColor::black);
    ofSetColor(ofColor::white);
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofLogToConsole();
    HWND notepad;

    do {
        //https://stackoverflow.com/questions/2113950/how-to-send-keystrokes-to-a-window
        notepad = FindWindow(_T("Notepad"), NULL);
        if (!notepad) {
            STARTUPINFO info = { sizeof(info) };
            PROCESS_INFORMATION processInfo;
            if (CreateProcessW(_T("powerpoint"), _T("cmd"), NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))        {
                WaitForSingleObject(processInfo.hProcess, INFINITE);
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
            }
            else {
                ofLogFatalError("CreateProcess") << "failed to create: " << _T("Notepad");
                break;
            }
        }
    } while (true);
    // loop look/ask user to close PowerPoint? https://docs.microsoft.com/en-gb/windows/desktop/ToolHelp/taking-a-snapshot-and-viewing-processes

    HWND edit = FindWindowEx(notepad, NULL, _T("Edit"), NULL);
    //Bring the Notepad window to the front.
    if (!SetForegroundWindow(notepad)) {
        ofLogFatalError("SetForegroundWindow") << "failed to create: " << _T("Notepad");

    }
    SendMessage(edit, WM_SETTEXT, NULL, (LPARAM)_T("hello"));

    ofDirectory dir("ContiqManifest");
    if (dir.create()) {
        ofLogNotice("ofDirectory") << "created: " << dir.path();
        ofFilePath file;
        ofxXmlPoco xml;
        xml.setTo(getXML());
        SHARE_INFO_2 p;
        DWORD parm_err = 0;
        // be sure to delete when done
        p.shi2_netname = TEXT("TESTSHARE");
        p.shi2_type = STYPE_DISKTREE; // disk drive
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
