#include "pjsipcallbacks.h"

#include "sipmanager.h"
#include "sipcall.h"

#include <utils/log.h>

#include <pjsua.h>

// callbacks for SipCall
void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
	if (SipCall * call = SipCall::findCallById(call_id))
		call->onCallState(call_id, e);
}

void on_call_media_state(pjsua_call_id call_id)
{
	if (SipCall * call = SipCall::findCallById(call_id))
		call->onCallMediaState(call_id);
}

void on_call_tsx_state(pjsua_call_id call_id, pjsip_transaction *tsx, pjsip_event *e)
{
	if (SipCall * call = SipCall::findCallById(call_id))
		call->onCallTsxState(call_id, tsx, e);
}

pj_status_t my_put_frame_callback(int call_id, pjmedia_frame *frame, int w, int h, int stride)
{
	if (SipCall * call = SipCall::findCallById(call_id))
		return call->onMyPutFrameCallback(frame, w, h, stride);
	else
		return -1;
}

pj_status_t my_preview_frame_callback(int call_id, pjmedia_frame *frame, const char* colormodelName, int w, int h, int stride)
{
	Q_UNUSED(frame)
	Q_UNUSED(colormodelName)
	Q_UNUSED(w)
	Q_UNUSED(h)
	Q_UNUSED(stride)

	if (SipCall * call = SipCall::findCallById(call_id))
		return call->onMyPreviewFrameCallback(frame, colormodelName, w, h, stride);
	else
		return -1;
}

// callbacks for SipManager
void on_reg_state(pjsua_acc_id acc_id)
{
	SipManager::callbackInstance()->onSipRegistrationState(acc_id);
}

void on_incoming_call(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata)
{
	SipManager::callbackInstance()->onIncomingCall(acc_id, call_id, rdata);
}

void simple_registrar(pjsip_rx_data *rdata)
{
	pjsip_tx_data *tdata;
	const pjsip_expires_hdr *exp;
	const pjsip_hdr *h;
	unsigned cnt = 0;
	pjsip_generic_string_hdr *srv;
	pj_status_t status;

	status = pjsip_endpt_create_response(pjsua_get_pjsip_endpt(), rdata, 200, NULL, &tdata);
	if (status != PJ_SUCCESS)
		return;

	exp = (pjsip_expires_hdr*)pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, NULL);

	h = rdata->msg_info.msg->hdr.next;
	while (h != &rdata->msg_info.msg->hdr)
	{
		if (h->type == PJSIP_H_CONTACT)
		{
			const pjsip_contact_hdr *c = (const pjsip_contact_hdr*)h;
			int e = c->expires;

			if (e < 0)
			{
				if (exp)
					e = exp->ivalue;
				else
					e = 3600;
			}

			if (e > 0)
			{
				pjsip_contact_hdr *nc = (pjsip_contact_hdr*)pjsip_hdr_clone(tdata->pool, h);
				nc->expires = e;
				pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)nc);
				++cnt;
			}
		}
		h = h->next;
	}

	srv = pjsip_generic_string_hdr_create(tdata->pool, NULL, NULL);
	srv->name = pj_str((char*)"Server");
	srv->hvalue = pj_str((char*)"pjsua simple registrar");
	pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*)srv);

	pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(), rdata, tdata, NULL, NULL);
}

/* Notification on incoming request */
pj_bool_t default_mod_on_rx_request(pjsip_rx_data *rdata)
{
	/* Simple registrar */
	if (pjsip_method_cmp(&rdata->msg_info.msg->line.req.method, &pjsip_register_method) == 0)
	{
		simple_registrar(rdata);
		return PJ_TRUE;
	}
	return PJ_FALSE;
}

void PJCallbacks::registerCallbacks(pjsua_callback & cb)
{
	LogDetail(QString("[PJCallbacks::registerCallbacks]: Registering pjsip callbacks..."));
	cb.on_reg_state = &on_reg_state;
	cb.on_call_state = &on_call_state;
	cb.on_incoming_call = &on_incoming_call;
	cb.on_call_media_state = &on_call_media_state;
	cb.on_call_tsx_state = &on_call_tsx_state;
}

void PJCallbacks::registerModuleCallbacks(pjsip_module & module)
{
	LogDetail(QString("[PJCallbacks::registerModuleCallbacks]: Registering pjsip module %1 callbacks...").arg(module.name.ptr));
	module.on_rx_request = &default_mod_on_rx_request;
}

void PJCallbacks::registerFrameCallbacks(pjmedia_vid_dev_myframe & myframe)
{
	myframe.put_frame_callback = &my_put_frame_callback;
	myframe.preview_frame_callback = &my_preview_frame_callback;
}
