#ifndef BSP_HPP
#define BSP_HPP

#include <cstddef>

namespace controller
{
size_t const BSP_TICKS_PER_SEC = static_cast< size_t >(50);
inline size_t SECONDS(size_t duration)
{
    return duration * BSP_TICKS_PER_SEC;
}

}

#endif
