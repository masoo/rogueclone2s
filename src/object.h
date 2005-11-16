#ifndef __OBJECT_H__
#define __OBJECT_H__

extern void free_stuff(object *objlist);
extern void get_food(object *obj, boolean force_ration);
extern void put_objects(void);
extern void put_stairs(void);

#endif /* __OBJECT_H__ */
