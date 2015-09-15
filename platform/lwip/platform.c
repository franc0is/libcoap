/*
 * coap_lwip.c
 *
 *  Created on: Jun 11, 2015
 *      Author: wojtek
 */

void coap_platform_init(void)
{
	prng_init(LWIP_RAND());
}

void coap_platform_deinit(void)
{

}

/* FIXME: retransmits that are not required any more due to incoming packages
 * do *not* get cleared at the moment, the wakeup when the transmission is due
 * is silently accepted. this is mainly due to the fact that the required
 * checks are similar in two places in the code (when receiving ACK and RST)
 * and that they cause more than one patch chunk, as it must be first checked
 * whether the sendqueue item to be dropped is the next one pending, and later
 * the restart function has to be called. nothing insurmountable, but it can
 * also be implemented when things have stabilized, and the performance
 * penality is minimal
 *
 * also, this completely ignores COAP_RESOURCE_CHECK_TIME.
 * */

static void coap_retransmittimer_execute(void *arg)
{
	coap_context_t *ctx = (coap_context_t*)arg;
	coap_tick_t now;
	coap_tick_t elapsed;
	coap_queue_t *nextinqueue;

	ctx->timer_configured = 0;

	coap_ticks(&now);

	elapsed = now - ctx->sendqueue_basetime; /* that's positive for sure, and unless we haven't been called for a complete wrapping cycle, did not wrap */

	nextinqueue = coap_peek_next(ctx);
	while (nextinqueue != NULL)
	{
		if (nextinqueue->t > elapsed) {
			nextinqueue->t -= elapsed;
			break;
		} else {
			elapsed -= nextinqueue->t;
			coap_retransmit(ctx, coap_pop_next(ctx));
			nextinqueue = coap_peek_next(ctx);
		}
	}

	ctx->sendqueue_basetime = now;

	coap_retransmittimer_restart(ctx);
}

static void coap_retransmittimer_restart(coap_context_t *ctx)
{
	coap_tick_t now, elapsed, delay;

	if (ctx->timer_configured)
	{
		printf("clearing\n");
		sys_untimeout(coap_retransmittimer_execute, (void*)ctx);
		ctx->timer_configured = 0;
	}
	if (ctx->sendqueue != NULL)
	{
		coap_ticks(&now);
		elapsed = now - ctx->sendqueue_basetime;
		if (ctx->sendqueue->t >= elapsed) {
			delay = ctx->sendqueue->t - elapsed;
		} else {
			/* a strange situation, but not completely impossible.
			 *
			 * this happens, for example, right after
			 * coap_retransmittimer_execute, when a retransmission
			 * was *just not yet* due, and the clock ticked before
			 * our coap_ticks was called.
			 *
			 * not trying to retransmit anything now, as it might
			 * cause uncontrollable recursion; let's just try again
			 * with the next main loop run.
			 * */
			delay = 0;
		}

		printf("scheduling for %d ticks\n", delay);
		sys_timeout(delay, coap_retransmittimer_execute, (void*)ctx);
		ctx->timer_configured = 1;
	}
}

coap_pdu_t *
coap_pdu_from_pbuf(struct pbuf *pbuf)
{
  if (pbuf == NULL) return NULL;

  LWIP_ASSERT("Can only deal with contiguous PBUFs", pbuf->tot_len == pbuf->len);
  LWIP_ASSERT("coap_read needs to receive an exclusive copy of the incoming pbuf", pbuf->ref == 1);

  coap_pdu_t *result = coap_malloc_type(COAP_PDU, sizeof(coap_pdu_t));
  if (!result) {
      pbuf_free(pbuf);
      return NULL;
  }

  memset(result, 0, sizeof(coap_pdu_t));

  result->max_size = pbuf->tot_len;
  result->length = pbuf->tot_len;
  result->hdr = pbuf->payload;
  result->pbuf = pbuf;

  return result;
}

void
coap_pdu_clear(coap_pdu_t *pdu, size_t size) {
  assert(pdu);

  /* the pdu itself is not wiped as opposed to the other implementations,
   * because we have to rely on the pbuf to be set there. */
  pdu->hdr = pdu->pbuf->payload;
  pdu->max_size = size;
  pdu->hdr->version = COAP_DEFAULT_VERSION;
  pdu->hdr->token_length = 0;

  /* data is NULL unless explicitly set by coap_add_data() */
  pdu->length = sizeof(coap_hdr_t);
}

coap_pdu_t *
coap_pdu_init(unsigned char type, unsigned char code,
          unsigned short id, size_t size) {
  coap_pdu_t *pdu;
    struct pbuf *p;
  assert(size <= COAP_MAX_PDU_SIZE);
  /* Size must be large enough to fit the header. */
  if (size < sizeof(coap_hdr_t) || size > COAP_MAX_PDU_SIZE)
    return NULL;

  p = pbuf_alloc(PBUF_TRANSPORT, size, PBUF_RAM);
  if (p != NULL) {
    u8_t header_error = pbuf_header(p, sizeof(coap_pdu_t));
    /* we could catch that case and allocate larger memory in advance, but then
     * again, we'd run into greater trouble with incoming packages anyway */
    LWIP_ASSERT("CoAP PDU header does not fit in transport header", header_error == 0);
    pdu = p->payload;
  } else {
    pdu = NULL;
  }
  if (pdu) {
    pdu->pbuf = p;
    coap_pdu_clear(pdu, size);
    pdu->hdr->id = id;
    pdu->hdr->type = type;
    pdu->hdr->code = code;
  }
  return pdu;
}

coap_pdu_t *
coap_new_pdu(void) {
  coap_pdu_t *pdu;

  pdu = coap_pdu_init(0, 0, ntohs(COAP_INVALID_TID), COAP_MAX_PDU_SIZE);

#ifndef NDEBUG
  if (!pdu)
    coap_log(LOG_CRIT, "coap_new_pdu: cannot allocate memory for new PDU\n");
#endif
  return pdu;
}

void
coap_delete_pdu(coap_pdu_t *pdu) {
  if (pdu != NULL) /* accepting double free as the other implementation accept that too */
    pbuf_free(pdu->pbuf);
}
