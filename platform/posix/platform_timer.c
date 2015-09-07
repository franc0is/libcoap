#include "coap_timer.h"

struct coap_timer_t {
  int time;
};

void coap_timer_init(void) {

}

coap_timer_t *coap_new_timer(CoapTimerCallback cb, void *data) {
  return NULL;
}

void coap_timer_set(coap_timer_t *timer, coap_tick_t num_ticks) {
  return;
}

void coap_timer_is_set(coap_timer_t *timer) {
  return;
}

void coap_timer_unset(coap_timer_t *timer) {
  return;
}


