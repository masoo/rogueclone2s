#ifndef RING_H
#define RING_H

extern void put_on_ring(void);
extern void do_put_on(object *ring, bool on_left);
extern void remove_ring(void);
extern void un_put_on(object *ring);
extern void gr_ring(object *ring, bool assign_wk);
extern void inv_rings(void);
extern void ring_stats(bool pr);

#endif /* not RING_H */
