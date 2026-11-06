// HelloLog: the smallest possible program built on the engine.
// Initialises engine logging, writes a message to standard output, and shuts down.

#include "Pikzel/Core/Log.h"


int main() {
   Pikzel::Log::Init();

   PKZL_LOG_INFO("Hello from Pikzel!");

   Pikzel::Log::Shutdown();
   return 0;
}
