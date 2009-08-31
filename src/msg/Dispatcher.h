// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */


#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include "Message.h"
#include "config.h"

class Messenger;

class Dispatcher {
  Dispatcher *next;

  // how i receive messages
  virtual bool ms_dispatch(Message *m) = 0;
  bool _ms_deliver_dispatch(Message *m) {
    bool ret = false;
    if (next)
      ret = next->_ms_deliver_dispatch(m);
    if (!ret)
      ret = ms_dispatch(m);
    return ret;
  }
public:
  void ms_deliver_dispatch(Message *m) {
    if (!_ms_deliver_dispatch(m)) {
      generic_dout(0) << "unhandled message " << m << " " << *m
		      << " from " << m->get_orig_source_inst()
		      << dendl;
      assert(0);
    }
  }

  void ms_deliver_handle_reset(const entity_addr_t& peer) {
    ms_handle_reset(peer);
    if (next)
      next->ms_handle_reset(peer);
  }
  void ms_deliver_handle_remote_reset(const entity_addr_t& peer) {
    ms_handle_remote_reset(peer);
    if (next)
      next->ms_handle_remote_reset(peer);
  }
  void ms_deliver_handle_failure(Message *m, const entity_addr_t& peer) {
    ms_handle_failure(m, peer);
    if (next)
      next->ms_handle_failure(m, peer);
  }


public:
  virtual ~Dispatcher() { }
  Dispatcher() : next(NULL) { }

  virtual void link_dispatcher(Dispatcher *disp) {
    if (!next) {
      next = disp;
    } else {
      next->link_dispatcher(disp);
    }
  }
  virtual void unlink_dispatcher(Dispatcher *disp) {
    assert(next);
    if (next == disp)
      next = next->next;
    else
      next->unlink_dispatcher(disp);
  }

  // how i deal with transmission failures.
  virtual void ms_handle_failure(Message *m, const entity_addr_t& addr) {  }

  /*
   * on any connection reset.
   * this indicates that the ordered+reliable delivery semantics have 
   * been violated.  messages may have been lost.
   */
  virtual void ms_handle_reset(const entity_addr_t& peer) { }

  // on deliberate reset of connection by remote
  //  implies incoming messages dropped; possibly/probably some of our previous outgoing too.
  virtual void ms_handle_remote_reset(const entity_addr_t& peer) { }
};

#endif
