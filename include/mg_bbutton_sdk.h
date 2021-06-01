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

#ifndef MG_BBUTTON_SDK_H_
#define MG_BBUTTON_SDK_H_

#include <stdbool.h>
#include "mg_bthing_sdk.h"
#include "mg_bsensor_sdk.h"
#include "mgos_bbutton.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MG_BBUTTON_NEW(b) MG_BTHING_SENS_NEW(b);

enum mg_bbutton_push_state {
  MG_BBUTTON_PUSH_STATE_UP,
  MG_BBUTTON_PUSH_STATE_DOWN,
  MG_BBUTTON_PUSH_STATE_FIRST_UP,
  MG_BBUTTON_PUSH_STATE_SECOND_DOWN,
  MG_BBUTTON_PUSH_STATE_PRESSED,
  MG_BBUTTON_PUSH_STATE_RESET
};

#define MG_EV_BBUTTON_NOTHING -1

/*****************************************
 * Cast Functions
 */

// Convert (mgos_bbutton_t) into (struct mg_bthing_sens *)
struct mg_bthing_sens *MG_BBUTTON_CAST1(mgos_bbutton_t thing);
/*****************************************/

#define MG_BBUTTON_CFG(s) ((struct mg_bbutton_cfg *)MG_BBUTTON_CAST1(s)->cfg)

struct mg_bbutton_overrides {
  mg_bthing_getting_state_handler_t getting_state_cb;
};

struct mg_bbutton_cfg {
  struct mg_bsensor_cfg base;
  struct mg_bbutton_overrides overrides;
  enum mg_bbutton_push_state push_state;
  mgos_bbutton_event_handler_t on_event_cb;
  void *on_event_ud;
  int64_t start_time;
  int64_t stop_time;
  int press_count;
  int click_ticks;
  int press_ticks;
  int press_repeat_ticks;
  int debounce_ticks;
};

bool mg_bbutton_init(mgos_bbutton_t btn, struct mg_bbutton_cfg *cfg);

void mg_bbutton_reset(mgos_bbutton_t btn);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MG_BBUTTON_SDK_H_ */