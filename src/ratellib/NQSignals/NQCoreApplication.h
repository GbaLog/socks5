#ifndef NQCoreApplicationH
#define NQCoreApplicationH

class NQCoreApplication
{
public:
  NQCoreApplication();

  static class SignalHandler & getSignalHandler();

  int runOnce();
  int run(bool inNewThread = false);
  void stop();
};

#endif // NQCoreApplicationH
