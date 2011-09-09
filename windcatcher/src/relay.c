// ���� ������� ��� ��������� ���������
#include "relay.h"
#include "usermemory.h"


RELAY Relay[RELAYS_AMOUNT];
//======================================================================================================
// ��������� �������� ��������
//======================================================================================================
void ReleysReset(void)
  {
    int i = RELAYS_AMOUNT;
    while (i--)
      {
        Relay[i] = RELAY_STATE_OFF;
      }
  }
RELAY RelayGet(uint16 num)
  {
    return Relay[num];
  }
void RelaySet(uint16 num, RELAY vol)
  {
    if ((num < RELAYS_AMOUNT) && ((vol == RELAY_STATE_ON) || (vol
        == RELAY_STATE_OFF)))
      {
        Relay[num] = vol;
      }
  }

void RelaysProcess()
  {
    int i = RELAYS_AMOUNT;
    while (i--)
      {
        if ((RAM.relays[(i>>4)] & (1<<(i&0x0f))) != 0 )
          {
            RelaySet(i, RELAY_STATE_ON);
          }
        else
          {
            RelaySet(i, RELAY_STATE_OFF);
          }
      }
  }
