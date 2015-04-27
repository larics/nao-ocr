#include <iostream>
#include <fstream>
#include <qi/os.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <alcommon/albroker.h>
#include <alcommon/albrokermanager.h>
#include <alcommon/altoolsmain.h>

#include "OcrModule.hpp"


#define ALCALL

extern "C"
{

  /**
   * \brief Creates a new NAO module.
   *
   * \param pBroker
   * \return int
   */
  ALCALL int _createModule(boost::shared_ptr<AL::ALBroker> pBroker)
  {
    // init broker with the main broker instance from the parent executable
    AL::ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
    AL::ALBrokerManager::getInstance()->addBroker(pBroker);

    // create module instances
    AL::ALModule::createModule<OcrModule>(pBroker, "OcrModule");
    return 0;
  }

  /**
   * \brief Closes a NAO module.
   *
   * \return int
   */
  ALCALL int _closeModule(  )
  {
    return 0;
  }
} // extern "C"

#ifdef TXTREC_IS_REMOTE
  int main(int argc, char *argv[])
  {
    // pointer to createModule
    TMainType sig;
    sig = &_createModule;
    // call main
    return ALTools::mainFunction("OcrModule", argc, argv, sig);
  }
#endif
