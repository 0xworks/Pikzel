// HelloLog: the smallest possible program built on the engine.
// Initialises engine logging, writes a message to standard output, and shuts down.

#include "Pikzel/Core/Log.h"

namespace pk = Pikzel;


int main() {
   pk::Log::Init();

   PKZL_LOG_INFO("Hello from Pikzel!");

   pk::Log::Shutdown();
   return 0;
}
