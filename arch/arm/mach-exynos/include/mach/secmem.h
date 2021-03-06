/* linux/arch/arm/mach-exynos/include/mach/secmem.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS - Secure memory support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_SECMEM_H
#define __ASM_ARCH_SECMEM_H __FILE__

#if defined(CONFIG_ION)
#include <linux/ion.h>
#endif

struct secchunk_info {
	int		index;
	phys_addr_t	base;
	size_t		size;
};

#if defined(CONFIG_ION)
struct secfd_info {
	int	fd;
	ion_phys_addr_t phys;
};
#endif

struct secmem_crypto_driver_ftn {
	int (*lock) (void);
	int (*release) (void);
};

#if defined(CONFIG_EXYNOS_CONTENT_PATH_PROTECTION)
void secmem_crypto_register(struct secmem_crypto_driver_ftn *ftn);
void secmem_crypto_deregister(void);
#else
#define secmem_crypto_register(ftn)
#define secmem_crypto_deregister()
#endif

#define SECMEM_IOC_CHUNKINFO		_IOWR('S', 1, struct secchunk_info)
#define SECMEM_IOC_SET_DRM_ONOFF	_IOWR('S', 2, int)
#define SECMEM_IOC_GET_DRM_ONOFF	_IOWR('S', 3, int)
#define SECMEM_IOC_GET_CRYPTO_LOCK	_IOR('S', 4, int)
#define SECMEM_IOC_RELEASE_CRYPTO_LOCK	_IOR('S', 5, int)
#if defined(CONFIG_SOC_EXYNOS5250)
#define SECMEM_IOC_GET_FD_PHYS_ADDR    _IOWR('S', 8, struct secfd_info)
#endif

#endif /* __ASM_ARCH_SECMEM_H */
