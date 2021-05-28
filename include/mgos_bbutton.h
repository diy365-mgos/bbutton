/*
 * Copyright (c) 2021 DIY356
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_BBUTTON_H_
#define MGOS_BBUTTON_H_

#include <stdbool.h>
#include "mgos_event.h"
#include "mgos_bvar_dic.h"
#include "mgos_bthing.h"
#include "mgos_bsensor.h"

#ifdef __cplusplus
extern "C" {
#endif

// [  ACTUATORS ]-[     SENSORS ]-[ SYS]
// [][][][][][][]-[][][][][1][][]-[0][0]
#define MGOS_BBUTTON_TYPE 16

#define MGOS_BBUTTON_DEFAULT_CLICK_TICKS 600 //milliseconds
#define MGOS_BBUTTON_DEFAULT_PRESS_TICKS 1000 //1 second
#define MGOS_BBUTTON_DEFAULT_DEBOUNCE_TICKS 50 //milliseconds

#define MGOS_BBUTTON_STR_STATE_IDLE         "IDLE"
#define MGOS_BBUTTON_STR_STATE_CLICKED      "CLICKED"
#define MGOS_BBUTTON_STR_STATE_DBLCLICKED   "DBLCLICKED"
#define MGOS_BBUTTON_STR_STATE_PRESSED      "PRESSED"
#define MGOS_BBUTTON_STR_STATE_RELEASED     "RELEASED"

#define MGOS_BBUTTON_EVENT_BASE MGOS_EVENT_BASE('B', 'U', 'T')
enum mgos_bbutton_event {
  MGOS_EV_BBUTTON_ANY = MGOS_BBUTTON_EVENT_BASE,
  MGOS_EV_BBUTTON_ON_IDLE,
  MGOS_EV_BBUTTON_ON_CLICK,
  MGOS_EV_BBUTTON_ON_DBLCLICK,
  MGOS_EV_BBUTTON_ON_PRESS,
  MGOS_EV_BBUTTON_ON_RELEASE
};

struct mg_bthing_sens;
typedef struct mg_bthing_sens *mgos_bbutton_t;

mgos_bthing_t MGOS_BBUTTON_THINGCAST(mgos_bbutton_t button);

mgos_bsensor_t MGOS_BBUTTON_DOWNCAST(mgos_bbutton_t button);

typedef void (*mgos_bbutton_event_handler_t)(mgos_bbutton_t button, enum mgos_bbutton_event ev, void *userdata);

mgos_bbutton_t mgos_bbutton_create(const char *id, enum mgos_bthing_pub_state_mode pub_state_mode);

bool mgos_bbutton_on_event(mgos_bbutton_t button, mgos_bbutton_event_handler_t on_event_cb, void* userdata);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_BBUTTON_H_ */