#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <string>

#define BUFSIZE 512

int makeSynchCall(LPTSTR lpszPipename, LPTSTR lpvMessage)
{
    HANDLE hPipe; 
    TCHAR  chBuf[BUFSIZE]; 
    BOOL   fSuccess = FALSE; 
    DWORD  cbRead, cbToWrite, cbWritten, dwMode; 

// Try to open a named pipe; wait for it, if necessary. 
    while (1) { 
        hPipe = CreateFile( 
            lpszPipename,   // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE, 
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 
 
        if (hPipe != INVALID_HANDLE_VALUE) 
            break; 

        if (GetLastError() != ERROR_PIPE_BUSY) {
            _tprintf( TEXT("Could not open pipe. Last error=%d\n"), GetLastError() ); 
            return -1;
        }

        if ( ! WaitNamedPipe(lpszPipename, 20000)) { 
            printf("Could not open pipe: 20 second wait timed out."); 
            return -1;
        } 
   } 
 
// The pipe connected; change to message-read mode. 
 
   dwMode = PIPE_READMODE_MESSAGE; 
   fSuccess = SetNamedPipeHandleState( 
                                    hPipe,    // pipe handle 
                                    &dwMode,  // new pipe mode 
                                    NULL,     // don't set maximum bytes 
                                    NULL);    // don't set maximum time 
   if ( ! fSuccess) {
      _tprintf( TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError() ); 
      return -1;
   }
 
   cbToWrite = (lstrlen(lpvMessage)+1)*sizeof(TCHAR);
   _tprintf( TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage); 

   fSuccess = WriteFile( 
      hPipe,                  // pipe handle 
      lpvMessage,             // message 
      cbToWrite,              // message length 
      &cbWritten,             // bytes written 
      NULL);                  // not overlapped 

   if ( ! fSuccess) {
      _tprintf( TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError() ); 
      return -1;
   }

   printf("\nMessage sent to server, receiving reply as follows:\n");
    
   do { 
   // Read from the pipe. 
      fSuccess = ReadFile( 
         hPipe,    // pipe handle 
         chBuf,    // buffer to receive reply 
         BUFSIZE*sizeof(TCHAR),  // size of buffer 
         &cbRead,  // number of bytes read 
         NULL);    // not overlapped 
 
      if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA )
         break; 
 
      _tprintf( TEXT("\"%s\"\n"), chBuf ); 
   } while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA 

   if ( ! fSuccess) {
      _tprintf( TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError() );
      return -1;
   }

   printf("\n<End of message, press ENTER to terminate connection and exit>\n");
   //_getch();
 
   CloseHandle(hPipe); 
    printf("closed handle man\n");
   return 0; 
}

int CompletedReadRoutine()
{
    printf("Completed Read routine\n");
}

int makeAsynchCall(LPTSTR lpszPipename, LPTSTR lpvMessage)
{
    HANDLE hPipe; 
    TCHAR  chBuf[BUFSIZE]; 
    BOOL   fSuccess = FALSE; 
    DWORD  cbRead, cbToWrite, cbWritten, dwMode; 
    OVERLAPPED overlapped;
    HANDLE hEvent;

    hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);

    if (hEvent == NULL) {
        printf("Create event failed with %d.\n", GetLastError());
        return 0;
    }

    overlapped.hEvent = hEvent;

    if (hEvent == NULL) {
        printf("CreateEvent failed with %d.\n", GetLastError());
        return 0;
    }

    // Try to open a named pipe; wait for it, if necessary. 
    
    while (1) { 
        hPipe = CreateFile( lpszPipename,   // pipe name 
                            GENERIC_READ |  // read and write access 
                            GENERIC_WRITE, 
                            0,              // no sharing 
                            NULL,           // default security attributes
                            OPEN_EXISTING,  // opens existing pipe 
                            0,              // default attributes 
                            NULL);          // no template file 
    
    
        if (hPipe != INVALID_HANDLE_VALUE) 
            break; 
    
        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
    
        if (GetLastError() != ERROR_PIPE_BUSY) {
            _tprintf( TEXT("Could not open pipe. GLE=%d\n"), GetLastError() ); 
            return -1;
        }
    
        // All pipe instances are busy, so wait for 20 seconds. 
    
        if ( ! WaitNamedPipe(lpszPipename, 20000)) { 
            printf("Could not open pipe: 20 second wait timed out."); 
            return -1;
        } 
    } 
    
    // The pipe connected; change to message-read mode. 
    
    dwMode = PIPE_READMODE_MESSAGE; 
    fSuccess = SetNamedPipeHandleState( 
        hPipe,    // pipe handle 
        &dwMode,  // new pipe mode 
        NULL,     // don't set maximum bytes 
        NULL);    // don't set maximum time 
    if ( ! fSuccess) {
        _tprintf( TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError() ); 
        return -1;
    }
    
    // Send a message to the pipe server. 
    
    cbToWrite = (lstrlen(lpvMessage)+1)*sizeof(TCHAR);
    _tprintf( TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage); 

    fSuccess = WriteFile( 
        hPipe,                  // pipe handle 
        lpvMessage,             // message 
        cbToWrite,              // message length 
        &cbWritten,             // bytes written 
        NULL);

    if ( ! fSuccess) {
        _tprintf( TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError() ); 
        return -1;
    }

    printf("\nMessage sent to server, receiving reply as follows:\n");
    //do { 
        fSuccess = ReadFile( 
            hPipe,    // pipe handle 
            chBuf,    // buffer to receive reply 
            BUFSIZE*sizeof(TCHAR),  // size of buffer 
            &cbRead,  // number of bytes read 
            NULL);
    
        if ( ! fSuccess && GetLastError() != ERROR_MORE_DATA ) {
            printf("Got nothing, returning\n"); 
            return -1;
        }
    
        _tprintf( TEXT("\"%s\"\n"), chBuf ); 
   // } while ( ! fSuccess);  // repeat loop if ERROR_MORE_DATA 

    if ( ! fSuccess) {
        _tprintf( TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError() );
        return -1;
    }

    printf("\n<End of message, press ENTER to terminate connection and exit>\n");
    //_getch();
    
    CloseHandle(hPipe); 
    printf("Closed handle\n");
    return 0;
}
 
int _tmain(int argc, char *argv[]) 
{ 
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
    LPTSTR lpvMessageA = TEXT("Asynch call");
    LPTSTR lpvMessageB = TEXT("Synch call");
    
    if (strcmp(argv[1], "asynch") == 0)
        makeAsynchCall(lpszPipename, lpvMessageA);
    
    if (strcmp(argv[1], "synch") == 0)
        makeSynchCall(lpszPipename, lpvMessageB);

    printf("End of the main\n");
    return 0;
}