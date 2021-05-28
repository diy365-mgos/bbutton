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

mgos_bbutton_t mgos_bbutton_create(const char *id, enum mgos_bthing_pub_state_mode pub_state_mode) {
  mgos_bbutton_t MG_BBUTTON_NEW(btn);
  if (mg_bthing_init(MG_BTHING_SENS_CAST3(btn), id, MGOS_BBUTTON_TYPE, pub_state_mode)) {
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

static void mg_bbutton_poll_cb(void *arg) {
  mgos_bthing_t thing;
  mgos_bthing_enum_t things = mgos_bthing_get_all();
  while (mgos_bthing_get_next(&things, &thing)) {
    if (mgos_bthing_is_typeof(thing, MGOS_BBUTTON_TYPE)) {
      mg_bsensor_update_state(MGOS_BBUTTON_DOWNCAST((mgos_bsensor_t)thing));
    }
  }
  (void) arg;
}

bool mgos_bbutton_init() {
  if (!mgos_event_register_base(MGOS_BBUTTON_EVENT_BASE, "bButton events")) {
    return false;
  }

   // initialize the polling global timer
  if (mgos_set_timer(10, MGOS_TIMER_REPEAT, mg_bbutton_poll_cb, NULL) == MGOS_INVALID_TIMER_ID) {
    LOG(LL_ERROR, ("Unable to start the internal polling timer for bButtons.'"));
    return false;
  }

  return true;
}