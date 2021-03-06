#include "mgos.h"
#include "mg_bbutton_sdk.h"

static mgos_bvar_t s_bool_state = NULL;

/*****************************************
 * Cast Functions
 */

// Convert (mgos_bbutton_t) into (struct mg_bthing_sens *)
struct mg_bthing_sens *MG_BBUTTON_CAST1(mgos_bbutton_t thing) {
  return (struct mg_bthing_sens *)thing;
}
/*****************************************/

bool mg_bbutton_upd_state(mgos_bbutton_t btn, mgos_bvar_t state, enum mgos_bbutton_event new_state) {
  switch(new_state) {
    case MGOS_EV_BBUTTON_ON_CLICK:
    case MGOS_EV_BBUTTON_ON_DBLCLICK:
    case MGOS_EV_BBUTTON_ON_PRESS:
    case MGOS_EV_BBUTTON_ON_RELEASE:
    case MGOS_EV_BBUTTON_ON_IDLE:
      new_state = (new_state - MGOS_EV_BBUTTON_ANY);
      break;
    default:
      return false;
  }

  // set MG_BUTTON_STATEKEY_EVENT key
  mgos_bvar_set_key_integer(state, MG_BUTTON_STATEKEY_EVENT, new_state);

  // set MG_BUTTON_STATEKEY_PRESS_COUNT key
  mgos_bvar_set_key_integer(state, MG_BUTTON_STATEKEY_PRESS_COUNT, mgos_bbutton_get_press_count(btn));

  // set MG_BUTTON_STATEKEY_PRESS_DURATION
  mgos_bvar_set_key_integer(state, MG_BUTTON_STATEKEY_PRESS_DURATION, mgos_bbutton_get_press_duration(btn));

  return true;
}

void mg_bbutton_state_machine_reset(struct mg_bbutton_cfg *cfg) {
  cfg->push_state = MG_BBUTTON_PUSH_STATE_UP;
  cfg->start_time = 0;
  cfg->stop_time = 0;
  cfg->press_count = 0;
}

static enum mgos_bbutton_event mg_bbutton_state_machine_tick(mgos_bbutton_t btn,
                                                             struct mg_bbutton_cfg *cfg,
                                                             enum mg_bbutton_push_state new_push_state) {
  int64_t now = mgos_uptime_micros();

  // Implementation of the state machine
  
  if (cfg->push_state == MG_BBUTTON_PUSH_STATE_RESET) {
    mg_bbutton_state_machine_reset(cfg); // restart
    return MGOS_EV_BBUTTON_ON_IDLE;

  } else if (cfg->push_state == MG_BBUTTON_PUSH_STATE_UP) { // waiting for button being pressed.
    if (new_push_state == MG_BBUTTON_PUSH_STATE_DOWN) {
      cfg->push_state = MG_BBUTTON_PUSH_STATE_DOWN;
      cfg->start_time = now;
    }

  } else if (cfg->push_state == MG_BBUTTON_PUSH_STATE_DOWN) { // waiting for button being released.
    int start_ticks = ((now - cfg->start_time) / 1000);
    if (new_push_state == MG_BBUTTON_PUSH_STATE_UP) {
      if (cfg->debounce_ticks > 0 && start_ticks < cfg->debounce_ticks) {
        // The button was released to quickly so I assume some debouncing.
        // So, go back to STATE_UP without calling a function.
        mg_bbutton_state_machine_reset(cfg); // restart
      } else {
        // The button was released for the firt time.
        cfg->push_state = MG_BBUTTON_PUSH_STATE_FIRST_UP;
        cfg->stop_time = now;
      }
    } else if (new_push_state == MG_BBUTTON_PUSH_STATE_DOWN) {
      if (start_ticks > cfg->press_ticks) {
        // The button starts being pressed (long press). 
        cfg->push_state = MG_BBUTTON_PUSH_STATE_PRESSED;
        cfg->stop_time = now;
        cfg->press_count = 1;
        return MGOS_EV_BBUTTON_ON_PRESS;
      }
    }

  } else if (cfg->push_state == MG_BBUTTON_PUSH_STATE_FIRST_UP) {
    // waiting for button being pressed the second time or timeout.
    if (((now - cfg->start_time) / 1000) > cfg->click_ticks) {
      cfg->start_time = cfg->stop_time = 0;
      cfg->push_state = MG_BBUTTON_PUSH_STATE_RESET; // going to be restarted
      return MGOS_EV_BBUTTON_ON_CLICK;

    } else if (new_push_state == MG_BBUTTON_PUSH_STATE_DOWN) {
      if (cfg->debounce_ticks == 0 || ((now - cfg->stop_time) / 1000) > cfg->debounce_ticks) {
        cfg->push_state = MG_BBUTTON_PUSH_STATE_SECOND_DOWN;
        cfg->start_time = now;
      }
    }

  } else if (cfg->push_state == MG_BBUTTON_PUSH_STATE_SECOND_DOWN) { // waiting for button being released finally.
    // Stay here for at least cfg->debounce_ticks because else we might end up in
    // state 1 if the button bounces for too long.
    if (new_push_state == MG_BBUTTON_PUSH_STATE_UP) {
      if (((now - cfg->start_time) / 1000) > cfg->debounce_ticks) {
        // this was a 2 click sequence.
        cfg->start_time = cfg->stop_time = 0;
        cfg->push_state = MG_BBUTTON_PUSH_STATE_RESET; // going to be restarted
        return MGOS_EV_BBUTTON_ON_DBLCLICK;

      }
    }

  } else if (cfg->push_state == MG_BBUTTON_PUSH_STATE_PRESSED) {
    // The button is pressed (long press).
    // So, waiting for the button being released.
    if (new_push_state == MG_BBUTTON_PUSH_STATE_UP) {
      // The button is released after a long press
      cfg->stop_time = now;
      cfg->push_state = MG_BBUTTON_PUSH_STATE_RESET; // going to be restarted
      return MGOS_EV_BBUTTON_ON_RELEASE; 
      
    } else {
      // The button continue being pressed (long press).
      if (cfg->press_repeat_ticks > 0 &&
          ((now - cfg->stop_time) / 1000) >= cfg->press_repeat_ticks) {
        cfg->stop_time = now;
        cfg->press_count += 1;
        return MGOS_EV_BBUTTON_ON_PRESS;

      }
    }
  }

  return MG_EV_BBUTTON_NOTHING; // -1
}

enum mg_bthing_state_result mg_bbutton_getting_state_cb(struct mg_bthing_sens *btn,
                                                        mgos_bvar_t state,
                                                        void *userdata) {
  if (btn && state) {
    struct mg_bbutton_cfg *cfg = MG_BBUTTON_CFG(btn);
    enum mg_bthing_state_result ret = cfg->overrides.getting_state_cb(btn, s_bool_state, userdata);
    if (ret != MG_BTHING_STATE_RESULT_SUCCESS) return ret;
    if (mgos_bvar_get_type(s_bool_state) == MGOS_BVAR_TYPE_BOOL) {
      enum mgos_bbutton_event ev = mg_bbutton_state_machine_tick(btn, cfg,
        (mgos_bvar_get_bool(s_bool_state) ? MG_BBUTTON_PUSH_STATE_DOWN : MG_BBUTTON_PUSH_STATE_UP));

      if (ev == MG_EV_BBUTTON_NOTHING) {
        if (!mgos_bvar_is_null(state)) {
          return MG_BTHING_STATE_RESULT_UNHANDLED;
        }
        // this is the first time I'm getting the state, so
        // I must move ahead updating the state as IDLE.
        ev = MGOS_EV_BBUTTON_ON_IDLE; 
      }

      if (mg_bbutton_upd_state((mgos_bbutton_t)btn, state, ev)) {
        return MG_BTHING_STATE_RESULT_SUCCESS;
      }

    } else {
      LOG(LL_ERROR, ("The '%s' get-state handler returned a state of type %d (%d was expected).",
        mgos_bthing_get_uid(MGOS_BBUTTON_THINGCAST(btn)), mgos_bvar_get_type(s_bool_state), MGOS_BVAR_TYPE_BOOL));
    }
  }
  return MG_BTHING_STATE_RESULT_ERROR;
}

static void mg_bbutton_state_changed_cb(int ev, void *ev_data, void *userdata) {
  mgos_bvarc_t ev_state;
  struct mgos_bthing_state *args = (struct mgos_bthing_state *)ev_data;
  
  if (mgos_bvarc_try_get_key(args->state, MG_BUTTON_STATEKEY_EVENT, &ev_state)) {
    enum mgos_bbutton_event ev = (mgos_bvar_get_integer(ev_state) + MGOS_EV_BBUTTON_ANY);
        
    // invoke the event handler
    struct mg_bbutton_cfg *cfg = MG_BBUTTON_CFG((mgos_bbutton_t)args->thing);
    if (cfg->on_event_cb) {
      cfg->on_event_cb((mgos_bbutton_t)args->thing, ev, cfg->on_event_ud);
    }
    // trigger the event
    mgos_event_trigger(ev, args->thing); 
  }
  (void) ev;
  (void) userdata;
}

bool mg_bbutton_init(mgos_bbutton_t btn, struct mg_bbutton_cfg *cfg) {
  if (cfg) {
    if (mg_bsensor_init(btn, &cfg->base)) {
      /* initalize ticks */
      cfg->click_ticks = MGOS_BBUTTON_DEFAULT_CLICK_TICKS;
      cfg->press_ticks = MGOS_BBUTTON_DEFAULT_PRESS_TICKS;
      cfg->press_repeat_ticks = MGOS_BBUTTON_DEFAULT_PRESS_TICKS;
      cfg->debounce_ticks = MGOS_BBUTTON_DEFAULT_DEBOUNCE_TICKS;
      /* initalize on-event handler */
      cfg->on_event_cb = NULL;
      cfg->on_event_ud = NULL;
      /* initalize state-machine */
      mg_bbutton_state_machine_reset(cfg);
      /* initalize overrides cfg */
      cfg->overrides.getting_state_cb = mg_bthing_on_getting_state(btn, mg_bbutton_getting_state_cb);
      /* initalize the state-changed handler */
      mgos_bthing_on_event(MGOS_BBUTTON_THINGCAST(btn), MGOS_EV_BTHING_STATE_CHANGED, mg_bbutton_state_changed_cb, NULL);

      // initialize the static temporary state variable
      if (!s_bool_state) s_bool_state = mgos_bvar_new_bool(false);

      return true; // initialization successfully completed
    }
    mg_bbutton_reset(btn);
  } else {
    LOG(LL_ERROR, ("Invalid NULL 'cfg' parameter."));
  }

  LOG(LL_ERROR, ("Error initializing bButton '%s'. See above error message for more details.",
    mgos_bthing_get_uid(MGOS_BBUTTON_THINGCAST(btn))));
  return false; 
}

void mg_bbutton_reset(mgos_bbutton_t btn) {
  struct mg_bbutton_cfg *cfg = MG_BBUTTON_CFG(btn);

  /* clear overrides cfg */
  if (cfg->overrides.getting_state_cb) {
    mg_bthing_on_getting_state(btn, cfg->overrides.getting_state_cb);
    cfg->overrides.getting_state_cb = NULL;
  }
  /* clear on-event handler */
  cfg->on_event_cb = NULL;
  cfg->on_event_ud = NULL;
  /* clear state-machine */
  mg_bbutton_state_machine_reset(cfg);

  // reset sensor-base obj
  mg_bsensor_reset(btn);
}