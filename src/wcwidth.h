#ifndef WCWIDTH_H_INCLUDED
#define WCWIDTH_H_INCLUDED

#include <stdlib.h>

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS	extern "C" {
#define __END_DECLS	}
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

__BEGIN_DECLS

int wcwidth(wchar_t ucs);

__END_DECLS

#endif
