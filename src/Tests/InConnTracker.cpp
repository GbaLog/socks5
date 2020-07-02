#include "Mocks.h"
#include "InConnTracker.h"
#include "InetUtils.h"
//-----------------------------------------------------------------------------
using ::testing::Return;
using ::testing::InSequence;
using ::testing::_;
using ::testing::Action;
using ::testing::NiceMock;
//-----------------------------------------------------------------------------
namespace
{
//-----------------------------------------------------------------------------
ACTION_P(GetConnectionUser, ptr)
{
  *ptr = arg0;
  return;
}
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
class InConnTrackerTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    _conn = std::make_shared<SocksConnectionMock>();
    EXPECT_CALL(*_conn, setUser(_)).WillOnce(GetConnectionUser(&_user));

    _owner = new IConnTrackerOwnerMock;
    _tracker = new InConnTracker(0, *_owner, _conn);
  }

  void TearDown() override
  {
    delete _tracker;
    delete _owner;
    _conn.reset();
    _user = nullptr;
  }

protected:
  std::shared_ptr<SocksConnectionMock> _conn;
  IConnTrackerOwnerMock * _owner;
  InConnTracker * _tracker;
  ISocksConnectionUser * _user;
};
//-----------------------------------------------------------------------------
TEST_F(InConnTrackerTest, ImmediateDisconnect)
{
  InSequence seq;
  EXPECT_CALL(*_owner, onConnectionClosed(0)).Times(1);

  ASSERT_NE(nullptr, _user);
  _user->onConnectionClosed();
}
