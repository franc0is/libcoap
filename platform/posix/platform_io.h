#ifndef _PLATFORM_IO_H_
#define _PLATFORM_IO_H_

#ifdef HAVE_STDIO_H
#  include <stdio.h>
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include <errno.h>

#include "address.h"

#define PLATFORM_ENDPOINT_PROPERTIES \
  int fd;

#endif /* _PLATFORM_IO_H_ */
