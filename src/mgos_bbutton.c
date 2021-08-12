#include "mgos.h"
#include "mgos_bthing.h"
#include "mg_bbutton_sdk.h"
#include "mgos_bbutton.h"

#ifdef MGOS_HAVE_MJS
#include "mjs.h"
#endif

mgos_bthing_t MGOS_BBUTTON_THINGCAST(mgos_bbutton_t sensor) {
  return MG_BTHING_SENS_CAST4(MG_BBUTTON_CAST1(sensor));
}

mgos_bsensor_t MGOS_BBUTTON_DOWNCAST(mgos_bbutton_t sensor) {
  return (mgos_bsensor_t)sensor;
}

mgos_bbutton_t mgos_bbutton_create(const char *id) {
  mgos_bbutton_t MG_BBUTTON_NEW(btn);
  if (mg_bthing_init(MG_BTHING_SENS_CAST3(btn), id, MGOS_BBUTTON_TYPE)) {
    struct mg_bbutton_cfg *cfg = calloc(1, sizeof(struct mg_bbutton_cfg));
    if (cfg) {
      if (mg_bbutton_init(btn, cfg) &&
          mg_bthing_register(MGOS_BBUTTON_THINGCAST(btn))) {
        LOG(LL_INFO, ("bButton '%s' successfully created.", id));
        return btn;
      }
      mg_bthing_reset(MG_BTHING_SENS_CAST3(btn));
    } else {
      LOG(LL_ERROR, ("Unable to allocate memory for 'mg_bbutton_cfg.'"));
    }
    free(cfg);
  }
  free(btn);

  LOG(LL_ERROR, ("Error creating bButton '%s'. See above error message for more details.'", (id ? id : "")));
  return NULL; 
}

bool mgos_bbutton_is_pressed(mgos_bbutton_t button) {
  return (!button ? false : (MG_BBUTTON_CFG(button)->push_state == MG_BBUTTON_PUSH_STATE_PRESSED));
}

int mgos_bbutton_get_press_duration(mgos_bbutton_t button) {
  struct mg_bbutton_cfg *cfg = MG_BBUTTON_CFG(button);
  return ((cfg->stop_time - cfg->start_time) / 1000);
}

int mgos_bbutton_get_press_count(mgos_bbutton_t button) {
  return (!button ? 0 : MG_BBUTTON_CFG(button)->press_count);
}

bool mgos_bbutton_on_event(mgos_bbutton_t button, mgos_bbutton_event_handler_t on_event_cb, void* userdata) {
  if (button) {
    struct mg_bbutton_cfg *cfg = MG_BBUTTON_CFG(button);
    if (!cfg->on_event_cb) {
      cfg->on_event_cb = on_event_cb;
      cfg->on_event_ud = (on_event_cb ? userdata : NULL);
      return true;
    }
    LOG(LL_ERROR, ("The event handler of bButton '%s' is already configured.",
      mgos_bthing_get_id(MGOS_BBUTTON_THINGCAST(button))));
  }
  return false;
}

bool mgos_bbutton_set_cfg(mgos_bbutton_t button, struct mgos_bbutton_cfg *cfg) {
  if (!button || !cfg) return false;
  struct mg_bbutton_cfg *bcfg = MG_BBUTTON_CFG(button);
  bcfg->click_ticks = (cfg->click_ticks == -1 ? MGOS_BBUTTON_DEFAULT_CLICK_TICKS : cfg->click_ticks);
  bcfg->press_ticks = (cfg->press_ticks == -1 ? MGOS_BBUTTON_DEFAULT_PRESS_TICKS : cfg->press_ticks);
  bcfg->press_repeat_ticks = (cfg->press_repeat_ticks == -1 ? MGOS_BBUTTON_DEFAULT_PRESS_TICKS : cfg->press_repeat_ticks);
  bcfg->debounce_ticks = (cfg->debounce_ticks == -1 ? MGOS_BBUTTON_DEFAULT_DEBOUNCE_TICKS : cfg->debounce_ticks);
  return true;
}

bool mgos_bbutton_get_cfg(mgos_bbutton_t button, struct mgos_bbutton_cfg *cfg) {
  if (!button || !cfg) return false;
  struct mg_bbutton_cfg *bcfg = MG_BBUTTON_CFG(button);
  memcpy(cfg, bcfg, sizeof(struct mgos_bbutton_cfg ));
  return true;
}

static void mg_bbutton_poll_cb(void *arg) {
  mg_bthing_update_states(MGOS_BBUTTON_TYPE, false);
  (void) arg;
}

bool mgos_bbutton_init() {
  if (!mgos_event_register_base(MGOS_BBUTTON_EVENT_BASE, "bButton events")) {
    return false;
  }

  // initialize the polling global timer
  if (mgos_set_timer(50, MGOS_TIMER_REPEAT, mg_bbutton_poll_cb, NULL) == MGOS_INVALID_TIMER_ID) {
    LOG(LL_ERROR, ("Unable to start the internal polling timer for bButtons.'"));
    return false;
  }

  return true;
}