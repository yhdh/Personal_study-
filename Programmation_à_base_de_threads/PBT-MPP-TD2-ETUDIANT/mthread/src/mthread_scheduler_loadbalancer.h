#ifndef MTHREAD_SCHEDULER_LOADBALANCER_H

class mthread_dummy_loadbalancer {
public:
  mthread_dummy_loadbalancer() = default;

  ~mthread_dummy_loadbalancer() = default;

  static int get_vp_id() { return 0; }
};

#define MTHREAD_SCHEDULER_LOADBALANCER_H
#endif
