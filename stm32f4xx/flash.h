#ifndef ETL_STM32F4XX_FLASH_H_
#define ETL_STM32F4XX_FLASH_H_

namespace etl {
namespace stm32f4xx {

struct Flash {
  #define ETL_BFF_DEFINITION_FILE etl/stm32f4xx/flash.reg
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Flash flash;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // ETL_STM32F4XX_FLASH_H_