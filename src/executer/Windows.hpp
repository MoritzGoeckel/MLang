#include <windows.h>
#include <iostream>
 
int cmd(const char *cmd, char *output, DWORD maxbuffer)
{
    HANDLE readHandle;
    HANDLE writeHandle;
    HANDLE stdHandle;
    DWORD bytesRead;
    DWORD retCode;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
   
    ZeroMemory(&sa,sizeof(SECURITY_ATTRIBUTES));
    ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));
    ZeroMemory(&si,sizeof(STARTUPINFO));
   
    sa.bInheritHandle=true;
    sa.lpSecurityDescriptor=NULL;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    si.cb=sizeof(STARTUPINFO);
    si.dwFlags=STARTF_USESHOWWINDOW;
    si.wShowWindow=SW_HIDE;
 
    if (!CreatePipe(&readHandle,&writeHandle,&sa,NULL))
    {
        std::cout << "cmd: CreatePipe failed!" << std::endl;
        return 0;
    }
 
    stdHandle=GetStdHandle(STD_OUTPUT_HANDLE);
 
    if (!SetStdHandle(STD_OUTPUT_HANDLE,writeHandle))
    {
        std::cout << "cmd: SetStdHandle(writeHandle) failed!" << std::endl;
        return 0;
    }
 
    if (!CreateProcess(NULL, cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&pi))
    {
        std::cout << "cmd: CreateProcess failed!" << std::endl;
        return 0;
    }
 
    GetExitCodeProcess(pi.hProcess,&retCode);
    while (retCode==STILL_ACTIVE)
    {
        GetExitCodeProcess(pi.hProcess,&retCode);
    }
 
    if (!ReadFile(readHandle,output,maxbuffer,&bytesRead,NULL))
    {
        std::cout << "cmd: ReadFile failed!" << std::endl;
        return 0;
    }
    output[bytesRead]='\0';
 
    if (!SetStdHandle(STD_OUTPUT_HANDLE,stdHandle))
    {
        std::cout << "cmd: SetStdHandle(stdHandle) failed!" << std::endl;
        return 0;
    }
 
    if (!CloseHandle(readHandle))
    {
        std::cout << "cmd: CloseHandle(readHandle) failed!" << std::endl;
    }
    if (!CloseHandle(writeHandle))
    {
        std::cout << "cmd: CloseHandle(writeHandle) failed!" << std::endl;
    }
 
    return 1;
}