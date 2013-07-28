#include "bsp.hpp"

#include <qp_port.h>

namespace controller
{
Q_DEFINE_THIS_FILE
}

namespace QP
{
void QF::onStartup()
{
    QF_setTickRate(controller::BSP_TICKS_PER_SEC);
}

void QF::onCleanup() {}

void QF_onClockTick()
{
    static uint8_t const clock_tick = 0U;

    QF::TICK(&clock_tick);
}

extern "C" void Q_onAssert(char const Q_ROM * const file, int line) {}

}
