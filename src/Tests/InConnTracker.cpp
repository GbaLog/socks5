#include "Mocks.h"
#include "InConnTracker.h"
#include "InetUtils.h"
//-----------------------------------------------------------------------------
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;
using ::testing::Action;
//-----------------------------------------------------------------------------
class InConnTrackerTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    _conn = std::make_shared<SocksConnectionMock>();
    _owner = new IConnTrackerOwnerMock;
    _tracker = new InConnTracker(0, *_owner, _conn);
  }

  void TearDown() override
  {
    delete _tracker;
    delete _owner;
    _conn.reset();
  }

protected:
  ISocksConnectionPtr _conn;
  IConnTrackerOwnerMock * _owner;
  InConnTracker * _tracker;
};
//-----------------------------------------------------------------------------
