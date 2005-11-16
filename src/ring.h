#ifndef __RING_H__
#define __RING_H__

extern void ring_stats(boolean pr);
extern void gr_ring(object *ring, boolean assign_wk);
extern void put_on_ring(void);
extern void remove_ring(void);
extern void inv_rings(void);
extern void un_put_on(object *ring);
extern void do_put_on(object *ring, boolean on_left);

#endif /* __RING_H__ */
