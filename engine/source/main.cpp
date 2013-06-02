#include "torqueConfig.h"
#include "platform/types.h"
#include "platformWin32/platformWin32.h"
#include "platform/platform.h"
#include "collection/vector.h"
#include "collection/template.h"
#include "game/gameInterface.h"

S32 PASCAL WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, S32)
{
   Vector<char *> argv;
   char moduleName[256];
   GetModuleFileNameA(NULL, moduleName, sizeof(moduleName));
   argv.push_back(moduleName);

   for (const char* word,*ptr = lpszCmdLine; *ptr; )  {
      // Eat white space
      for (; dIsspace(*ptr) && *ptr; ptr++)
         ;

      // Pick out the next word
      bool quote = false;
      for (word = ptr; !(dIsspace(*ptr) && !quote) && *ptr; ptr++)
      {
         if(*ptr == '\"') quote = ! quote;
      }

      if(*word == '\"') ++word;
      
      // Add the word to the argument list.
      if (*word) {
         int len = ptr - word;
         if(*(ptr-1) == '\"') --len;

         char *arg = (char *) dMalloc(len + 1);
         dStrncpy(arg, word, len);
         arg[len] = 0;
         argv.push_back(arg);
      }
   }

   winState.appInstance = hInstance;


    // Initialize fonts.
   HDC fontHDC = CreateCompatibleDC(NULL);
   HBITMAP fontBMP = CreateCompatibleBitmap(fontHDC, 256, 256);

    //windowSize.set(0,0);

    // Finish if the game didn't initialize.
    if(!Game->mainInitialize(argv.size(), (const char **) argv.address()) )
        return 0;

    // run the game main loop.
    while( Game->isRunning() )
    {
        Game->mainLoop();
    }

    // Shut the game down.
    Game->mainShutdown();

   DeleteObject(fontBMP);
   DeleteObject(fontHDC);


   for(U32 j = 1; j < (U32)argv.size(); j++)
      dFree(argv[j]);

   return 0;
}