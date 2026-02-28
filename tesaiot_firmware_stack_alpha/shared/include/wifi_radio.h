#ifndef WIFI_RADIO_H_
#define WIFI_RADIO_H_

#include "cy_wcm.h"

cy_rslt_t wifi_radio_init(void);

cy_wcm_config_t *wifi_radio_get_wcm_config(void);

#endif /* WIFI_RADIO_H_ */
